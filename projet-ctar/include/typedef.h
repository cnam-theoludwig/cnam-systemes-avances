#ifndef __TYPEDEF__
#define __TYPEDEF__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Size of a TAR block in bytes.
 *
 * A tar archive is a sequence of 512-byte records. Headers and file data
 * are aligned to 512-byte boundaries.
 */
#define TAR_BLOCK_SIZE 512

/**
 * @brief Maximum path buffer size for path operations.
 *
 * Used for buffers that hold file paths during archive extraction,
 * creation, and directory operations.
 */
#define MAX_PATH_BUFFER_SIZE 4096

/**
 * @brief Buffer size for file copy operations.
 *
 * Used for the internal buffer when copying file data between
 * file descriptors during archive extraction and creation.
 */
#define COPY_BUFFER_SIZE 8192

/**
 * magic contains the magic value "ustar" followed by a NUL byte to indicate
 * that this is a POSIX standard archive. Full compliance requires the uname
 * and gname fields be properly set.
 */
#define USTAR_MAGIC "ustar"

/**
 * Length of the "ustar" literal without the trailing NUL.
 */
#define USTAR_MAGIC_LEN 5

/**
 * Version. This should be "00" (two copies of the ASCII digit zero) for
 * POSIX standard archives.
 */
#define USTAR_VERSION "00"

/**
 * Regular file. NUL should be treated as a synonym, for compatibility
 * purposes.
 */
#define USTAR_TYPE_REG '0'

/**
 * Regular file synonym (NUL), for compatibility purposes.
 */
#define USTAR_TYPE_REG_ALT '\0'

/**
 * Hard link.
 */
#define USTAR_TYPE_LINK '1'

/**
 * Symbolic link.
 */
#define USTAR_TYPE_SYMLINK '2'

/**
 * Character device node.
 */
#define USTAR_TYPE_CHAR '3'

/**
 * Block device node.
 */
#define USTAR_TYPE_BLOCK '4'

/**
 * Directory.
 */
#define USTAR_TYPE_DIR '5'

/**
 * FIFO node.
 */
#define USTAR_TYPE_FIFO '6'

/**
 * Reserved.
 */
#define USTAR_TYPE_RESERVED '7'

/**
 * @brief POSIX ustar (Unix Standard TAR) header - IEEE Std 1003.1-1988.
 *
 * IEEE Std 1003.1-1988 ("POSIX.1") defined a standard tar file format to be
 * read and written by compliant implementations of tar(1). This format is
 * often called the "ustar" format, after the magic value used in the header.
 * (The name is an acronym for "Unix Standard TAR".) It extends the historic
 * format with new fields.
 *
 * Notes from the manual (man 5 tar):
 * - typeflag: Type  of	entry. It is worth noting that the size field, in particular, has different
 *   meanings depending on the type. For regular files, of course, it indicates
 *   the amount of data following the header. For directories, it may be used
 *   to indicate the total size of all files in the directory, for use by
 *   operating systems that pre-allocate directory space. For all other types,
 *   it should be set to zero by writers and ignored by readers.
 *
 * - magic: Contains the magic value "ustar" followed by a NUL byte to indicate
 *   that this is a POSIX standard archive. Full compliance requires the uname
 *   and gname fields be properly set.
 *
 * - version: This should be "00" (two copies of the ASCII digit zero) for
 *   POSIX standard archives.
 *
 * - uname, gname: User and group names, as null-terminated ASCII strings.
 *   These should be used in preference to the uid/gid values when they are
 *   set and the corresponding names exist on the system.
 *
 * - devmajor, devminor: Major and minor numbers for character device or block
 *   device entry.
 *
 * - name, prefix: If the pathname is too long to fit in the 100 bytes
 *   provided by the standard format, it can be split at any '/' character
 *   with the first portion going into the prefix field. If the prefix field
 *   is not empty, the reader will prepend the prefix value and a '/' character
 *   to the regular name field to obtain the full pathname. The standard does
 *   not require a trailing '/' character on directory names, though most
 *   implementations still include this for compatibility reasons.
 */
struct header_posix_ustar {
  /* File name. If the pathname is too long, see the 'prefix' field. */
  char name[100];

  /* File mode, stored as ASCII octal. */
  char mode[8];

  /* User ID of owner, stored as ASCII octal. */
  char uid[8];

  /* Group ID of owner, stored as ASCII octal. */
  char gid[8];

  /* Size of file in bytes (regular files), stored as ASCII octal. */
  char size[12];

  /* Modification time of file, stored as ASCII octal (epoch seconds). */
  char mtime[12];

  /* Checksum for header record, stored as ASCII octal. */
  char checksum[8];

  /* Type of entry. See USTAR_TYPE_* constants above. */
  char typeflag[1];

  /* Name of linked file. */
  char linkname[100];

  /* Magic field: "ustar" followed by a NUL byte. */
  char magic[6];

  /* Version: "00". */
  char version[2];

  /* User name, null-terminated ASCII string. */
  char uname[32];

  /* Group name, null-terminated ASCII string. */
  char gname[32];

  /* Major device number (for character/block device), ASCII octal. */
  char devmajor[8];

  /* Minor device number (for character/block device), ASCII octal. */
  char devminor[8];

  /* Pathname prefix for long names; prepended with '/' to 'name'. */
  char prefix[155];

  /* Padding to make the header record 512 bytes. */
  char pad[12];
};

#endif
