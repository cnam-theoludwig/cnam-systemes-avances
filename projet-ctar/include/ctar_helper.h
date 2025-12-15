#ifndef __CTAR_HELPER__
#define __CTAR_HELPER__

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#include "../include/typedef.h"

/**
 * @brief Abstraction handle for file I/O (standard FD or zlib gzFile).
 */
struct ctar_handle {
  /** File descriptor (used for raw IO) */
  int fd;

  /** Zlib file handle (used for compressed IO) */
  gzFile gz_file;

  /** If true, use gz_file, otherwise use fd */
  bool use_zlib;
};

/**
 * @brief Parsed TAR header view for convenient use by ctar functions.
 */
struct ctar_helper_parsed_header {
  char name[100];
  char prefix[155];
  char full_name[512];
  char typeflag;
  size_t size_bytes;
  unsigned long checksum_computed;
  size_t checksum_declared;
  bool checksum_matches;
  bool is_end_of_archive;
};

/**
 * @brief Opens an archive file, handling compression transparently.
 *
 * For reading, it uses gzdopen to automatically detect compression.
 * For writing, it uses gzdopen only if compress is true.
 *
 * @param path Path to the file.
 * @param flags Open flags (e.g. O_RDONLY, O_WRONLY | O_CREAT).
 * @param mode File mode for creation.
 * @param compress Request compression (only relevant for writing).
 * @param out_handle Pointer to handle structure to initialize.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_open_archive(const char* path, int flags, mode_t mode, bool compress, struct ctar_handle* out_handle);

/**
 * @brief Close the IO handle.
 *
 * @param handle Handle to close.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_close(struct ctar_handle* handle);

/**
 * @brief Write data to the handle.
 * @param handle Handle to write to.
 * @param buffer Data buffer.
 * @param size Size of data.
 * @return Number of bytes written or -1 on error.
 */
ssize_t ctar_helper_write_data(struct ctar_handle* handle, const void* buffer, size_t size);

/**
 * @brief Read exactly size bytes unless EOF or error occurs.
 *
 * @param handle IO handle to read from.
 * @param buffer Destination buffer.
 * @param size Number of bytes to read.
 * @return Number of bytes read (can be < size on EOF), or -1 on error (errno set).
 */
ssize_t ctar_helper_safe_read(struct ctar_handle* handle, void* buffer, size_t size);

/**
 * @brief Return true if the 512-byte block is all zeros.
 *
 * @param block Pointer to 512-byte block.
 * @return true if all bytes are zero, false otherwise.
 */
bool ctar_helper_is_zero_block(const unsigned char* block);

/**
 * @brief Compute ustar checksum of a 512-byte header with checksum field treated as spaces.
 *
 * @param block Pointer to 512-byte header.
 * @return Unsigned checksum value as sum of bytes.
 */
unsigned long ctar_helper_compute_checksum(const unsigned char* block);

/**
 * @brief Parse an ASCII octal field with optional NUL/space termination.
 *
 * @param field Pointer to the field.
 * @param length Field length.
 * @param out_value Parsed value on success.
 * @return 0 on success, -1 on error (errno set to EINVAL on invalid input).
 */
int ctar_helper_parse_octal(const char* field, size_t length, size_t* out_value);

/**
 * @brief Parse mode field to mode_t (ASCII octal).
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_parse_octal_mode(const char* field, size_t length, mode_t* out_mode);

/**
 * @brief Copy a header string field and ensure NUL termination within fixed-size destination.
 *
 * Copies up to destination_length characters or until a NUL byte in source,
 * whichever comes first. Always NUL-terminates the destination.
 *
 * @param destination Destination buffer (fixed-size).
 * @param destination_length Size of destination.
 * @param source Pointer to header field.
 * @param source_length Length of header field.
 */
void ctar_helper_trim_and_copy(char* destination, size_t destination_length, const char* source, size_t source_length);

/**
 * @brief Join prefix and name into destination with a single slash if prefix is not empty.
 *
 * The result is always NUL-terminated. If the result does not fit, it is truncated.
 *
 * @param destination Destination buffer.
 * @param destination_length Size of destination buffer.
 * @param prefix Prefix string (can be empty or NULL).
 * @param name Name string (must not be NULL).
 */
void ctar_helper_join_path(char* destination, size_t destination_length, const char* prefix, const char* name);

/**
 * @brief Read and parse next header. Handles end-of-archive markers.
 *
 * Reads one 512-byte block and, if it is a header, parses fields into parsed.
 * If the block is zero, reads the next block to detect end of archive.
 *
 * @param handle Open archive handle.
 * @param parsed Output parsed header.
 * @return 1 if a valid header was parsed,
 *         0 if end-of-archive was reached,
 *        -1 on error (errno set).
 */
int ctar_helper_read_and_parse_header(struct ctar_handle* handle, struct ctar_helper_parsed_header* parsed);

/**
 * @brief Compute number of 512-byte blocks needed for size.
 */
size_t ctar_helper_blocks_for_size(size_t size_bytes);

/**
 * @brief Skip a specific amount of bytes in the archive.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_skip(struct ctar_handle* handle, size_t amount);

/**
 * @brief Skip the entire entry data (content + padding) based on file size.
 *
 * Used when listing files or skipping entries without extracting them.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_skip_entry(struct ctar_handle* handle, size_t size_bytes);

/**
 * @brief Skip file padding only (alignment to next 512-byte block).
 *
 * Used after the file content has been read/extracted.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_skip_padding(struct ctar_handle* handle, size_t size_bytes);

/**
 * @brief Copy exactly size_bytes from input to output.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_copy_exact(struct ctar_handle* input, struct ctar_handle* output, size_t size_bytes);

/**
 * @brief Write directory path of file_path into out (like dirname), without allocations.
 *
 * If file_path has no '/', out becomes ".".
 *
 * @param file_path Input path.
 * @param out Output buffer for directory path.
 * @param out_length Size of output buffer.
 */
void ctar_helper_dirname(const char* file_path, char* out, size_t out_length);

/**
 * @brief Create directories recursively (mkdir -p behavior).
 *
 * @param path Directory path to create.
 * @param mode Directory permissions.
 * @return 0 on success, -1 on error (errno set).
 */
int ctar_helper_mkdir_p(const char* path, mode_t mode);

/**
 * @brief Write TAR header for a file to archive.
 *
 * @param archive_handle Archive handle.
 * @param path Path of the file (as it will appear in archive).
 * @param file_stat Stat information of the file.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_write_header(struct ctar_handle* archive_handle, const char* path, const struct stat* file_stat);

/**
 * @brief Write file data to archive with proper padding.
 *
 * @param archive_handle Archive handle.
 * @param path Path of the file to write.
 * @param file_size Size of the file.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_write_file_data(struct ctar_handle* archive_handle, const char* path, size_t file_size);

/**
 * @brief Add a single file to archive.
 *
 * @param archive_handle Archive handle.
 * @param path Path of the file to add.
 * @param archive_path Path as it should appear in the archive.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_add_file(struct ctar_handle* archive_handle, const char* path, const char* archive_path);

/**
 * @brief Add a directory recursively to archive.
 *
 * @param archive_handle Archive handle.
 * @param directory_path Directory to add.
 * @param base_path Base path to remove from archive paths.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_add_directory_recursive(struct ctar_handle* archive_handle, const char* directory_path, const char* base_path);

/**
 * @brief Check if a path is safe for extraction (no path traversal).
 *
 * Validates that the path doesn't contain components that could escape
 * the extraction directory, such as absolute paths or ".." sequences.
 *
 * @param path Path to validate.
 * @return true if the path is safe, false if it contains path traversal.
 */
bool ctar_helper_is_path_safe(const char* path);

#endif
