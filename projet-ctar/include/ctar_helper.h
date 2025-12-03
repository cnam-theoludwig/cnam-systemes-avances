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

#include "../include/typedef.h"

/**
 * @brief Read exactly size bytes unless EOF or error occurs.
 *
 * @param file_descriptor File descriptor to read from.
 * @param buffer Destination buffer.
 * @param size Number of bytes to read.
 * @return Number of bytes read (can be < size on EOF), or -1 on error (errno set).
 */
ssize_t ctar_helper_safe_read(int file_descriptor, void* buffer, size_t size);

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
 * @brief Read and parse next header. Handles end-of-archive markers.
 *
 * Reads one 512-byte block and, if it is a header, parses fields into parsed.
 * If the block is zero, reads the next block to detect end of archive.
 *
 * @param file_descriptor Open archive file descriptor.
 * @param parsed Output parsed header.
 * @return 1 if a valid header was parsed,
 *         0 if end-of-archive was reached,
 *        -1 on error (errno set).
 */
int ctar_helper_read_and_parse_header(int file_descriptor, struct ctar_helper_parsed_header* parsed);

/**
 * @brief Compute number of 512-byte blocks needed for size.
 */
size_t ctar_helper_blocks_for_size(size_t size_bytes);

/**
 * @brief Skip file padding to align to next 512-byte block based on size.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_skip_padding(int file_descriptor, size_t size_bytes);

/**
 * @brief Copy exactly size_bytes from input to output.
 *
 * @return 0 on success, -1 on error.
 */
int ctar_helper_copy_exact(int input_fd, int output_fd, size_t size_bytes);

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
 * @param archive_fd Archive file descriptor.
 * @param path Path of the file (as it will appear in archive).
 * @param file_stat Stat information of the file.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_write_header(int archive_fd, const char* path, const struct stat* file_stat);

/**
 * @brief Write file data to archive with proper padding.
 *
 * @param archive_fd Archive file descriptor.
 * @param path Path of the file to write.
 * @param file_size Size of the file.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_write_file_data(int archive_fd, const char* path, size_t file_size);

/**
 * @brief Add a single file to archive.
 *
 * @param archive_fd Archive file descriptor.
 * @param path Path of the file to add.
 * @param archive_path Path as it should appear in the archive.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_add_file(int archive_fd, const char* path, const char* archive_path);

/**
 * @brief Add a directory recursively to archive.
 *
 * @param archive_fd Archive file descriptor.
 * @param directory_path Directory to add.
 * @param base_path Base path to remove from archive paths.
 * @return 0 on success, -1 on error.
 */
int ctar_helper_add_directory_recursive(int archive_fd, const char* directory_path, const char* base_path);

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
