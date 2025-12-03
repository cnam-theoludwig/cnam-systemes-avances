#include "../include/ctar.h"

bool is_verbose = false;

int ctar_list(const char* archive_path) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_list(archive_path=\"%s\")\n",
            archive_path != NULL ? archive_path : "(null)");
  }
  if (archive_path == NULL) {
    errno = EINVAL;
    return -1;
  }

  int file_descriptor = open(archive_path, O_RDONLY);
  if (file_descriptor < 0) {
    return -1;
  }

  while (true) {
    struct ctar_helper_parsed_header parsed;
    int parse_result = ctar_helper_read_and_parse_header(file_descriptor, &parsed);
    if (parse_result < 0) {
      close(file_descriptor);
      return -1;
    }
    if (parse_result == 0) {
      break;
    }

    if (parsed.full_name[0] != '\0') {
      dprintf(STDOUT_FILENO, "%s\n", parsed.full_name);
    } else if (is_verbose) {
      dprintf(STDOUT_FILENO, "warning: empty name in header, skipping\n");
    }

    size_t blocks_to_skip = ctar_helper_blocks_for_size(parsed.size_bytes);
    off_t skip_offset = (off_t)(blocks_to_skip * TAR_BLOCK_SIZE);
    off_t new_position = lseek(file_descriptor, skip_offset, SEEK_CUR);
    if (new_position == (off_t)-1) {
      unsigned char buffer[TAR_BLOCK_SIZE];
      size_t remaining_bytes = blocks_to_skip * TAR_BLOCK_SIZE;
      while (remaining_bytes > 0) {
        size_t chunk_size = remaining_bytes < sizeof(buffer) ? remaining_bytes : sizeof(buffer);
        ssize_t skip_result = ctar_helper_safe_read(file_descriptor, buffer, chunk_size);
        if (skip_result <= 0) {
          break;
        }
        remaining_bytes -= (size_t)skip_result;
      }
    }
  }

  close(file_descriptor);
  return 0;
}

int ctar_extract(const char* archive_path, const char* target_directory) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_extract(archive_path=\"%s\", target_directory=\"%s\")\n",
            archive_path != NULL ? archive_path : "(null)",
            target_directory != NULL ? target_directory : "(null)");
  }
  if (archive_path == NULL) {
    errno = EINVAL;
    return -1;
  }

  int file_descriptor = open(archive_path, O_RDONLY);
  if (file_descriptor < 0) {
    return -1;
  }

  if (target_directory != NULL) {
    if (ctar_helper_mkdir_p(target_directory, 0755) != 0) {
      close(file_descriptor);
      return -1;
    }
    if (chdir(target_directory) != 0) {
      close(file_descriptor);
      return -1;
    }
  }

  while (true) {
    struct ctar_helper_parsed_header parsed;
    int parse_result = ctar_helper_read_and_parse_header(file_descriptor, &parsed);
    if (parse_result < 0) {
      close(file_descriptor);
      return -1;
    }
    if (parse_result == 0) {
      break;
    }

    if (parsed.full_name[0] == '\0') {
      if (is_verbose) {
        dprintf(STDOUT_FILENO, "warning: empty name in header, skipping\n");
      }
      size_t blocks_to_skip = ctar_helper_blocks_for_size(parsed.size_bytes);
      off_t skip_offset = (off_t)(blocks_to_skip * TAR_BLOCK_SIZE);
      lseek(file_descriptor, skip_offset, SEEK_CUR);
      continue;
    }

    if (is_verbose) {
      dprintf(STDOUT_FILENO, "extracting: %s\n", parsed.full_name);
    }

    char directory_path[4096];
    ctar_helper_dirname(parsed.full_name, directory_path, sizeof(directory_path));
    if (strcmp(directory_path, ".") != 0) {
      ctar_helper_mkdir_p(directory_path, 0755);
    }

    if (parsed.typeflag == USTAR_TYPE_DIR || parsed.typeflag == '5') {
      if (ctar_helper_mkdir_p(parsed.full_name, 0755) != 0 && errno != EEXIST) {
        if (is_verbose) {
          dprintf(STDOUT_FILENO, "warning: failed to create directory %s\n", parsed.full_name);
        }
      }
    } else if (parsed.typeflag == USTAR_TYPE_REG || parsed.typeflag == USTAR_TYPE_REG_ALT || parsed.typeflag == '\0') {
      int output_fd = open(parsed.full_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (output_fd < 0) {
        if (is_verbose) {
          dprintf(STDOUT_FILENO, "warning: failed to create file %s\n", parsed.full_name);
        }
        size_t blocks_to_skip = ctar_helper_blocks_for_size(parsed.size_bytes);
        off_t skip_offset = (off_t)(blocks_to_skip * TAR_BLOCK_SIZE);
        lseek(file_descriptor, skip_offset, SEEK_CUR);
        continue;
      }

      if (ctar_helper_copy_exact(file_descriptor, output_fd, parsed.size_bytes) != 0) {
        close(output_fd);
        unlink(parsed.full_name);
        close(file_descriptor);
        return -1;
      }

      close(output_fd);

      if (ctar_helper_skip_padding(file_descriptor, parsed.size_bytes) != 0) {
        close(file_descriptor);
        return -1;
      }
    } else {
      if (is_verbose) {
        dprintf(STDOUT_FILENO, "warning: unsupported file type %c for %s\n", parsed.typeflag, parsed.full_name);
      }
      size_t blocks_to_skip = ctar_helper_blocks_for_size(parsed.size_bytes);
      off_t skip_offset = (off_t)(blocks_to_skip * TAR_BLOCK_SIZE);
      lseek(file_descriptor, skip_offset, SEEK_CUR);
    }
  }

  close(file_descriptor);
  return 0;
}

int ctar_create(const char* archive_path, const char* directory_path) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_create(archive_path=\"%s\", directory_path=\"%s\")\n",
            archive_path != NULL ? archive_path : "(null)",
            directory_path != NULL ? directory_path : "(null)");
  }

  if (archive_path == NULL || directory_path == NULL) {
    errno = EINVAL;
    return -1;
  }

  struct stat dir_stat;
  if (stat(directory_path, &dir_stat) != 0 || !S_ISDIR(dir_stat.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }

  int archive_fd = open(archive_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (archive_fd < 0) {
    return -1;
  }

  if (ctar_helper_add_directory_recursive(archive_fd, directory_path, directory_path) != 0) {
    close(archive_fd);
    return -1;
  }

  unsigned char end_blocks[TAR_BLOCK_SIZE * 2] = {0};
  ssize_t write_result = write(archive_fd, end_blocks, sizeof(end_blocks));
  if (write_result != sizeof(end_blocks)) {
    close(archive_fd);
    return -1;
  }

  close(archive_fd);
  return 0;
}

int ctar_directory(const char* directory_path) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_directory(directory_path=\"%s\")\n",
            directory_path != NULL ? directory_path : "(null)");
  }
  if (directory_path == NULL) {
    errno = EINVAL;
    return -1;
  }
  if (ctar_helper_mkdir_p(directory_path, 0777) != 0) {
    return -1;
  }
  return 0;
}
