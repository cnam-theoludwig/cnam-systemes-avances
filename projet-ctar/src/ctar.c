#include "../include/ctar.h"

bool is_verbose = false;

int ctar_list(const char* archive_path) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_list(archive_path=\"%s\")\n", archive_path ? archive_path : "(null)");
  }
  if (archive_path == NULL) {
    errno = EINVAL;
    return -1;
  }

  struct ctar_handle archive;
  if (ctar_helper_open_archive(archive_path, O_RDONLY, 0, false, &archive) != 0) {
    return -1;
  }

  while (true) {
    struct ctar_helper_parsed_header parsed;
    int parse_result = ctar_helper_read_and_parse_header(&archive, &parsed);
    if (parse_result < 0) {
      ctar_helper_close(&archive);
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

    // Skip the entire file entry (data + padding) to reach next header
    if (ctar_helper_skip_entry(&archive, parsed.size_bytes) != 0) {
      ctar_helper_close(&archive);
      return -1;
    }
  }

  ctar_helper_close(&archive);
  return 0;
}

int ctar_extract(const char* archive_path, const char* target_directory) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_extract(archive_path=\"%s\", target_directory=\"%s\")\n",
            archive_path ? archive_path : "(null)", target_directory ? target_directory : "(null)");
  }
  if (archive_path == NULL) {
    errno = EINVAL;
    return -1;
  }

  struct ctar_handle archive;
  if (ctar_helper_open_archive(archive_path, O_RDONLY, 0, false, &archive) != 0) {
    return -1;
  }

  if (target_directory != NULL) {
    if (ctar_helper_mkdir_p(target_directory, 0755) != 0) {
      ctar_helper_close(&archive);
      return -1;
    }
    if (chdir(target_directory) != 0) {
      ctar_helper_close(&archive);
      return -1;
    }
  }

  while (true) {
    struct ctar_helper_parsed_header parsed;
    int parse_result = ctar_helper_read_and_parse_header(&archive, &parsed);
    if (parse_result < 0) {
      ctar_helper_close(&archive);
      return -1;
    }
    if (parse_result == 0) {
      break;
    }

    if (parsed.full_name[0] == '\0') {
      if (is_verbose) {
        dprintf(STDOUT_FILENO, "warning: empty name in header, skipping\n");
      }
      if (ctar_helper_skip_entry(&archive, parsed.size_bytes) != 0) {
        ctar_helper_close(&archive);
        return -1;
      }
      continue;
    }

    if (!ctar_helper_is_path_safe(parsed.full_name)) {
      if (is_verbose) {
        dprintf(STDOUT_FILENO, "warning: skipping unsafe path: %s\n", parsed.full_name);
      }
      if (ctar_helper_skip_entry(&archive, parsed.size_bytes) != 0) {
        ctar_helper_close(&archive);
        return -1;
      }
      continue;
    }

    if (is_verbose) {
      dprintf(STDOUT_FILENO, "extracting: %s\n", parsed.full_name);
    }

    char directory_path[MAX_PATH_BUFFER_SIZE];
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
        if (ctar_helper_skip_entry(&archive, parsed.size_bytes) != 0) {
          ctar_helper_close(&archive);
          return -1;
        }
        continue;
      }

      struct ctar_handle output_handle;
      output_handle.fd = output_fd;
      output_handle.gz_file = NULL;
      output_handle.use_zlib = false;

      if (ctar_helper_copy_exact(&archive, &output_handle, parsed.size_bytes) != 0) {
        ctar_helper_close(&output_handle);
        unlink(parsed.full_name);
        ctar_helper_close(&archive);
        return -1;
      }
      ctar_helper_close(&output_handle);
      if (ctar_helper_skip_padding(&archive, parsed.size_bytes) != 0) {
        ctar_helper_close(&archive);
        return -1;
      }
    } else {
      if (is_verbose) {
        dprintf(STDOUT_FILENO, "warning: unsupported type %c for %s\n", parsed.typeflag, parsed.full_name);
      }
      if (ctar_helper_skip_entry(&archive, parsed.size_bytes) != 0) {
        ctar_helper_close(&archive);
        return -1;
      }
    }
  }

  ctar_helper_close(&archive);
  return 0;
}

int ctar_create(const char* archive_path, const char* directory_path, bool compress) {
  if (is_verbose) {
    dprintf(STDOUT_FILENO, "ctar_create(archive=\"%s\", dir=\"%s\", compress=%s)\n",
            archive_path ? archive_path : "(null)", directory_path ? directory_path : "(null)", compress ? "true" : "false");
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

  struct ctar_handle archive;
  if (ctar_helper_open_archive(archive_path, O_WRONLY | O_CREAT | O_TRUNC, 0644, compress, &archive) != 0) {
    return -1;
  }

  if (ctar_helper_add_directory_recursive(&archive, directory_path, directory_path) != 0) {
    ctar_helper_close(&archive);
    return -1;
  }

  unsigned char end_blocks[TAR_BLOCK_SIZE * 2] = {0};
  if (ctar_helper_write_data(&archive, end_blocks, sizeof(end_blocks)) != sizeof(end_blocks)) {
    ctar_helper_close(&archive);
    return -1;
  }

  ctar_helper_close(&archive);
  return 0;
}
