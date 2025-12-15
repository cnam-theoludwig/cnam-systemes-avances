#include "../include/ctar_helper.h"

#include <limits.h>

extern bool is_verbose;

static ssize_t ctar_io_read(struct ctar_handle* handle, void* buf, size_t len) {
  if (handle->use_zlib) {
    size_t total_read = 0;
    unsigned char* dest = (unsigned char*)buf;

    while (total_read < len) {
      size_t remaining = len - total_read;
      unsigned int chunk_size = (remaining > UINT_MAX) ? UINT_MAX : (unsigned int)remaining;

      int bytes = gzread(handle->gz_file, dest + total_read, chunk_size);
      if (bytes < 0) {
        return -1;
      }
      if (bytes == 0) {
        break;  // EOF reached
      }
      total_read += (size_t)bytes;
    }
    return (ssize_t)total_read;
  }
  return read(handle->fd, buf, len);
}

static ssize_t ctar_io_write(struct ctar_handle* handle, const void* buf, size_t len) {
  if (handle->use_zlib) {
    size_t total_written = 0;
    const unsigned char* src = (const unsigned char*)buf;

    while (total_written < len) {
      size_t remaining = len - total_written;
      unsigned int chunk_size = (remaining > UINT_MAX) ? UINT_MAX : (unsigned int)remaining;

      int bytes = gzwrite(handle->gz_file, src + total_written, chunk_size);
      if (bytes <= 0 || bytes != (int)chunk_size) {
        return -1;  // Error or partial write
      }
      total_written += (size_t)bytes;
    }
    return (ssize_t)total_written;
  }
  return write(handle->fd, buf, len);
}

static off_t ctar_io_seek(struct ctar_handle* handle, off_t offset, int whence) {
  if (handle->use_zlib) {
    return (off_t)gzseek(handle->gz_file, (z_off_t)offset, whence);
  }
  return lseek(handle->fd, offset, whence);
}

int ctar_helper_open_archive(const char* path, int flags, mode_t mode, bool compress, struct ctar_handle* out_handle) {
  if (path == NULL || out_handle == NULL) {
    errno = EINVAL;
    return -1;
  }

  int fd = open(path, flags, mode);
  if (fd < 0) {
    return -1;
  }

  bool use_zlib = false;
  const char* z_mode = NULL;

  if ((flags & O_ACCMODE) == O_RDONLY) {
    use_zlib = true;
    z_mode = "rb";
  } else if (compress) {
    use_zlib = true;
    z_mode = "wb";
  }

  if (use_zlib) {
    gzFile gz = gzdopen(fd, z_mode);
    if (gz == NULL) {
      close(fd);
      errno = ENOMEM;
      return -1;
    }
    out_handle->fd = -1;
    out_handle->gz_file = gz;
    out_handle->use_zlib = true;
  } else {
    out_handle->fd = fd;
    out_handle->gz_file = NULL;
    out_handle->use_zlib = false;
  }
  return 0;
}

int ctar_helper_close(struct ctar_handle* handle) {
  if (handle == NULL) {
    return 0;
  }
  int ret = 0;
  if (handle->use_zlib && handle->gz_file != NULL) {
    int gz_ret = gzclose(handle->gz_file);
    if (gz_ret != Z_OK) {
      errno = EIO;
      ret = -1;
    }
  } else if (!handle->use_zlib && handle->fd >= 0) {
    ret = close(handle->fd);
  }
  handle->fd = -1;
  handle->gz_file = NULL;
  return ret;
}

ssize_t ctar_helper_write_data(struct ctar_handle* handle, const void* buffer, size_t size) {
  return ctar_io_write(handle, buffer, size);
}

ssize_t ctar_helper_safe_read(struct ctar_handle* handle, void* buffer, size_t size) {
  if (buffer == NULL || size == 0) {
    errno = EINVAL;
    return -1;
  }

  unsigned char* destination = (unsigned char*)buffer;
  size_t total_read = 0;

  while (total_read < size) {
    ssize_t bytes_read = ctar_io_read(handle, destination + total_read, size - total_read);
    if (bytes_read < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    if (bytes_read == 0) {
      break;
    }
    total_read += (size_t)bytes_read;
  }
  return (ssize_t)total_read;
}

bool ctar_helper_is_zero_block(const unsigned char* block) {
  if (block == NULL) {
    return false;
  }
  for (size_t index = 0; index < TAR_BLOCK_SIZE; index++) {
    if (block[index] != 0) {
      return false;
    }
  }
  return true;
}

unsigned long ctar_helper_compute_checksum(const unsigned char* block) {
  if (block == NULL) {
    return 0;
  }
  unsigned long sum = 0;
  for (size_t index = 0; index < TAR_BLOCK_SIZE; index++) {
    if (index >= 148 && index < 156) {
      sum += (unsigned long)' ';
    } else {
      sum += (unsigned long)block[index];
    }
  }
  return sum;
}

int ctar_helper_parse_octal(const char* field, size_t length, size_t* out_value) {
  if (field == NULL || out_value == NULL || length == 0) {
    errno = EINVAL;
    return -1;
  }
  *out_value = 0;
  size_t index = 0;
  while (index < length && (field[index] == ' ' || field[index] == '\0')) {
    index++;
  }
  if (index >= length) {
    return 0;
  }
  while (index < length && field[index] >= '0' && field[index] <= '7') {
    *out_value = (*out_value * 8) + (size_t)(field[index] - '0');
    index++;
  }
  while (index < length && (field[index] == ' ' || field[index] == '\0')) {
    index++;
  }
  if (index < length && field[index] != '\0' && field[index] != ' ') {
    errno = EINVAL;
    return -1;
  }
  return 0;
}

int ctar_helper_parse_octal_mode(const char* field, size_t length, mode_t* out_mode) {
  size_t value;
  if (ctar_helper_parse_octal(field, length, &value) != 0) {
    return -1;
  }
  *out_mode = (mode_t)value;
  return 0;
}

void ctar_helper_trim_and_copy(char* destination, size_t destination_length, const char* source, size_t source_length) {
  if (destination == NULL || source == NULL || destination_length == 0) {
    return;
  }
  size_t copy_length = 0;
  for (size_t index = 0; index < source_length && index < destination_length - 1; index++) {
    if (source[index] == '\0') {
      break;
    }
    destination[copy_length] = source[index];
    copy_length++;
  }
  destination[copy_length] = '\0';
}

void ctar_helper_join_path(char* destination, size_t destination_length, const char* prefix, const char* name) {
  if (destination == NULL || name == NULL || destination_length == 0) {
    return;
  }
  destination[0] = '\0';
  if (prefix != NULL && prefix[0] != '\0') {
    size_t prefix_len = strlen(prefix);
    if (prefix_len < destination_length - 1) {
      strncpy(destination, prefix, destination_length - 1);
      destination[destination_length - 1] = '\0';
      size_t current_length = strlen(destination);
      if (current_length > 0 && destination[current_length - 1] != '/' && current_length < destination_length - 2) {
        destination[current_length] = '/';
        destination[current_length + 1] = '\0';
      }
    }
  }
  size_t current_length = strlen(destination);
  if (current_length < destination_length - 1) {
    strncat(destination, name, destination_length - current_length - 1);
  }
}

int ctar_helper_read_and_parse_header(struct ctar_handle* handle, struct ctar_helper_parsed_header* parsed) {
  if (parsed == NULL) {
    errno = EINVAL;
    return -1;
  }
  unsigned char block[TAR_BLOCK_SIZE];
  ssize_t bytes_read = ctar_helper_safe_read(handle, block, TAR_BLOCK_SIZE);
  if (bytes_read < 0) {
    return -1;
  }
  if (bytes_read != TAR_BLOCK_SIZE) {
    errno = ENODATA;
    return -1;
  }
  if (ctar_helper_is_zero_block(block)) {
    unsigned char second_block[TAR_BLOCK_SIZE];
    ssize_t second_read = ctar_helper_safe_read(handle, second_block, TAR_BLOCK_SIZE);
    if (second_read == TAR_BLOCK_SIZE && ctar_helper_is_zero_block(second_block)) {
      parsed->is_end_of_archive = true;
      return 0;
    }
    errno = ENODATA;
    return -1;
  }
  struct header_posix_ustar* header = (struct header_posix_ustar*)block;
  if (strncmp(header->magic, USTAR_MAGIC, USTAR_MAGIC_LEN) != 0) {
    errno = EINVAL;
    return -1;
  }
  parsed->is_end_of_archive = false;
  parsed->typeflag = header->typeflag[0];
  ctar_helper_trim_and_copy(parsed->name, sizeof(parsed->name), header->name, sizeof(header->name));
  ctar_helper_trim_and_copy(parsed->prefix, sizeof(parsed->prefix), header->prefix, sizeof(header->prefix));
  ctar_helper_join_path(parsed->full_name, sizeof(parsed->full_name),
                        parsed->prefix[0] != '\0' ? parsed->prefix : NULL, parsed->name);
  if (ctar_helper_parse_octal(header->size, sizeof(header->size), &parsed->size_bytes) != 0) {
    return -1;
  }
  parsed->checksum_computed = ctar_helper_compute_checksum(block);
  if (ctar_helper_parse_octal(header->checksum, sizeof(header->checksum), &parsed->checksum_declared) != 0) {
    return -1;
  }
  parsed->checksum_matches = (parsed->checksum_computed == parsed->checksum_declared);
  return 1;
}

size_t ctar_helper_blocks_for_size(size_t size_bytes) {
  return (size_bytes + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
}

int ctar_helper_skip(struct ctar_handle* handle, size_t amount) {
  if (amount == 0) {
    return 0;
  }
  if (ctar_io_seek(handle, (off_t)amount, SEEK_CUR) != (off_t)-1) {
    return 0;
  }
  unsigned char buffer[TAR_BLOCK_SIZE];
  size_t remaining = amount;
  while (remaining > 0) {
    size_t to_read = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
    if (ctar_helper_safe_read(handle, buffer, to_read) <= 0) {
      return -1;
    }
    remaining -= to_read;
  }
  return 0;
}

int ctar_helper_skip_entry(struct ctar_handle* handle, size_t size_bytes) {
  size_t blocks = ctar_helper_blocks_for_size(size_bytes);
  size_t total = blocks * TAR_BLOCK_SIZE;
  return ctar_helper_skip(handle, total);
}

int ctar_helper_skip_padding(struct ctar_handle* handle, size_t size_bytes) {
  size_t blocks = ctar_helper_blocks_for_size(size_bytes);
  size_t total = blocks * TAR_BLOCK_SIZE;
  size_t padding = total - size_bytes;
  return ctar_helper_skip(handle, padding);
}

int ctar_helper_copy_exact(struct ctar_handle* input, struct ctar_handle* output, size_t size_bytes) {
  unsigned char buffer[COPY_BUFFER_SIZE];
  size_t remaining = size_bytes;
  while (remaining > 0) {
    size_t to_copy = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
    ssize_t bytes_read = ctar_helper_safe_read(input, buffer, to_copy);
    if (bytes_read <= 0) {
      return -1;
    }
    size_t bytes_to_write = (size_t)bytes_read;
    size_t written = 0;
    while (written < bytes_to_write) {
      ssize_t write_result = ctar_io_write(output, buffer + written, bytes_to_write - written);
      if (write_result < 0) {
        if (errno == EINTR) {
          continue;
        }
        return -1;
      }
      written += (size_t)write_result;
    }
    remaining -= (size_t)bytes_read;
  }
  return 0;
}

void ctar_helper_dirname(const char* file_path, char* out, size_t out_length) {
  if (file_path == NULL || out == NULL || out_length == 0) {
    return;
  }
  const char* last_slash = strrchr(file_path, '/');
  if (last_slash == NULL) {
    strncpy(out, ".", out_length - 1);
    out[out_length - 1] = '\0';
    return;
  }
  size_t dir_length = (size_t)(last_slash - file_path);
  if (dir_length == 0) {
    strncpy(out, "/", out_length - 1);
    out[out_length - 1] = '\0';
    return;
  }
  if (dir_length >= out_length) {
    dir_length = out_length - 1;
  }
  strncpy(out, file_path, dir_length);
  out[dir_length] = '\0';
}

int ctar_helper_mkdir_p(const char* path, mode_t mode) {
  if (path == NULL) {
    errno = EINVAL;
    return -1;
  }
  char path_copy[MAX_PATH_BUFFER_SIZE];
  strncpy(path_copy, path, sizeof(path_copy) - 1);
  path_copy[sizeof(path_copy) - 1] = '\0';
  size_t length = strlen(path_copy);
  if (length > 0 && path_copy[length - 1] == '/') {
    path_copy[length - 1] = '\0';
  }

  for (char* current = path_copy + 1; *current != '\0'; current++) {
    if (*current == '/') {
      *current = '\0';
      struct stat st;
      if (stat(path_copy, &st) != 0) {
        if (mkdir(path_copy, mode) != 0 && errno != EEXIST) {
          return -1;
        }
      } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
      }
      *current = '/';
    }
  }
  struct stat st;
  if (stat(path_copy, &st) != 0) {
    if (mkdir(path_copy, mode) != 0 && errno != EEXIST) {
      return -1;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }
  return 0;
}

int ctar_helper_write_header(struct ctar_handle* archive_handle, const char* path, const struct stat* file_stat) {
  struct header_posix_ustar header;
  memset(&header, 0, sizeof(header));
  strncpy(header.name, path, sizeof(header.name) - 1);
  snprintf(header.mode, sizeof(header.mode), "%07o", (unsigned int)(file_stat->st_mode & 07777));
  snprintf(header.uid, sizeof(header.uid), "%07o", (unsigned int)file_stat->st_uid);
  snprintf(header.gid, sizeof(header.gid), "%07o", (unsigned int)file_stat->st_gid);
  snprintf(header.mtime, sizeof(header.mtime), "%011o", (unsigned int)file_stat->st_mtime);

  if (S_ISREG(file_stat->st_mode)) {
    snprintf(header.size, sizeof(header.size), "%011o", (unsigned int)file_stat->st_size);
    header.typeflag[0] = USTAR_TYPE_REG;
  } else if (S_ISDIR(file_stat->st_mode)) {
    header.typeflag[0] = USTAR_TYPE_DIR;
  } else {
    header.typeflag[0] = USTAR_TYPE_REG;
  }

  memcpy(header.magic, USTAR_MAGIC, USTAR_MAGIC_LEN);
  header.magic[USTAR_MAGIC_LEN] = '\0';
  memcpy(header.version, USTAR_VERSION, sizeof(header.version));
  memset(header.checksum, ' ', sizeof(header.checksum));
  unsigned long checksum = ctar_helper_compute_checksum((const unsigned char*)&header);
  snprintf(header.checksum, sizeof(header.checksum) - 1, "%06o", (unsigned int)checksum);
  header.checksum[sizeof(header.checksum) - 1] = '\0';

  if (ctar_io_write(archive_handle, &header, sizeof(header)) != sizeof(header)) {
    return -1;
  }
  return 0;
}

int ctar_helper_write_file_data(struct ctar_handle* archive_handle, const char* path, size_t file_size) {
  int input_fd = open(path, O_RDONLY);
  if (input_fd < 0) {
    return -1;
  }

  struct ctar_handle input_handle;
  input_handle.fd = input_fd;
  input_handle.gz_file = NULL;
  input_handle.use_zlib = false;

  if (ctar_helper_copy_exact(&input_handle, archive_handle, file_size) != 0) {
    ctar_helper_close(&input_handle);
    return -1;
  }
  ctar_helper_close(&input_handle);

  size_t blocks_needed = ctar_helper_blocks_for_size(file_size);
  size_t total_bytes = blocks_needed * TAR_BLOCK_SIZE;
  size_t padding_needed = total_bytes - file_size;
  if (padding_needed > 0) {
    unsigned char padding[TAR_BLOCK_SIZE] = {0};
    if (ctar_io_write(archive_handle, padding, padding_needed) != (ssize_t)padding_needed) {
      return -1;
    }
  }
  return 0;
}

int ctar_helper_add_file(struct ctar_handle* archive_handle, const char* path, const char* archive_path) {
  struct stat file_stat;
  if (stat(path, &file_stat) != 0) {
    return -1;
  }
  if (ctar_helper_write_header(archive_handle, archive_path, &file_stat) != 0) {
    return -1;
  }
  if (S_ISREG(file_stat.st_mode) && file_stat.st_size > 0) {
    if (ctar_helper_write_file_data(archive_handle, path, (size_t)file_stat.st_size) != 0) {
      return -1;
    }
  }
  return 0;
}

int ctar_helper_add_directory_recursive(struct ctar_handle* archive_handle, const char* directory_path, const char* base_path) {
  DIR* directory = opendir(directory_path);
  if (directory == NULL) {
    return -1;
  }

  size_t base_length = strlen(base_path);
  if (base_length > 0 && base_path[base_length - 1] == '/') {
    base_length--;
  }

  char relative_path[MAX_PATH_BUFFER_SIZE];
  if (strlen(directory_path) > base_length && directory_path[base_length] == '/') {
    strncpy(relative_path, directory_path + base_length + 1, sizeof(relative_path) - 1);
  } else {
    strncpy(relative_path, directory_path + base_length, sizeof(relative_path) - 1);
  }
  relative_path[sizeof(relative_path) - 1] = '\0';

  if (relative_path[0] != '\0') {
    size_t rel_len = strlen(relative_path);
    if (rel_len > 0 && relative_path[rel_len - 1] != '/') {
      strncat(relative_path, "/", sizeof(relative_path) - rel_len - 1);
    }
    if (ctar_helper_add_file(archive_handle, directory_path, relative_path) != 0) {
      closedir(directory);
      return -1;
    }
    if (is_verbose) {
      dprintf(STDOUT_FILENO, "adding: %s\n", relative_path);
    }
  }

  struct dirent* entry;
  while ((entry = readdir(directory)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    char full_path[MAX_PATH_BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);
    struct stat file_stat;
    if (stat(full_path, &file_stat) != 0) {
      continue;
    }

    if (S_ISDIR(file_stat.st_mode)) {
      if (ctar_helper_add_directory_recursive(archive_handle, full_path, base_path) != 0) {
        closedir(directory);
        return -1;
      }
    } else {
      char archive_entry_path[MAX_PATH_BUFFER_SIZE];
      if (strlen(full_path) > base_length && full_path[base_length] == '/') {
        strncpy(archive_entry_path, full_path + base_length + 1, sizeof(archive_entry_path) - 1);
      } else {
        strncpy(archive_entry_path, full_path + base_length, sizeof(archive_entry_path) - 1);
      }
      archive_entry_path[sizeof(archive_entry_path) - 1] = '\0';
      if (ctar_helper_add_file(archive_handle, full_path, archive_entry_path) != 0) {
        if (is_verbose) {
          dprintf(STDOUT_FILENO, "warning: failed to add %s\n", full_path);
        }
      } else if (is_verbose) {
        dprintf(STDOUT_FILENO, "adding: %s\n", archive_entry_path);
      }
    }
  }
  closedir(directory);
  return 0;
}

bool ctar_helper_is_path_safe(const char* path) {
  if (path == NULL || path[0] == '\0') {
    return false;
  }
  if (path[0] == '/') {
    return false;
  }
  const char* current = path;
  while (*current != '\0') {
    if (current[0] == '.' && current[1] == '.') {
      if (current[2] == '/' || current[2] == '\0') {
        return false;
      }
    }
    while (*current != '\0' && *current != '/') {
      current++;
    }
    while (*current == '/') {
      current++;
    }
  }
  return true;
}
