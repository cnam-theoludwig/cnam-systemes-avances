#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <libgen.h>
#include <sys/types.h>
#include <limits.h>

#include "../include/typedef.h"
#include "../include/tar_utils.h"

#define BLOCK_SIZE 512
#define READ_BUF_SIZE 4096

/*
* Helper: check if a 512-byte block is all zeros
*
* @param buf Pointer to the block
* @return true if all bytes are zero, false otherwise
*/
static bool is_zero_block(const unsigned char *buf) {
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        if (buf[i] != 0) return false;
    }
    return true;
}

/*
* Convert octal string (not necessarily NUL-terminated) to size_t
*
* @param s Pointer to the octal string
* @param len Length of the string
* @return Converted size_t value
*/
static size_t octal_to_size(const char *s, size_t len) {
    size_t result = 0;
    size_t i = 0;
    /* skip leading spaces and NULs */
    while (i < len && (s[i] == ' ' || s[i] == '\0')) ++i;
    for (; i < len && s[i] >= '0' && s[i] <= '7'; ++i) {
        result = result * 8 + (s[i] - '0');
    }
    return result;
}

/*
* Compute checksum of a tar header block
* @param block Pointer to the 512-byte block
* @return Computed checksum value
*/
static unsigned long compute_checksum(const unsigned char *block) {
    unsigned long sum = 0;
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        if (i >= 148 && i < 156) {
            sum += (unsigned char)' ';
        } else {
            sum += block[i];
        }
    }
    return sum;
}

/*
* Build full filename from posix_header (handling prefix)
*
* @param hdr Pointer to the posix_header
* @param out Output buffer for the filename
* @param out_sz Size of the output buffer
*/
static void build_filename(const struct posix_header *hdr, char *out, size_t out_sz) {
    char namebuf[101] = {0};
    char prefixbuf[156] = {0};

    memcpy(namebuf, hdr->name, 100);
    namebuf[100] = '\0';
    memcpy(prefixbuf, hdr->prefix, 155);
    prefixbuf[155] = '\0';

    if (prefixbuf[0] != '\0') {
        snprintf(out, out_sz, "%s/%s", prefixbuf, namebuf);
    } else {
        snprintf(out, out_sz, "%s", namebuf);
    }
}

/*
* Ensure parent directories exist for a given path
*
* @param path Path for which to ensure parent directories
* @param mode Mode to use for created
* @return 0 on success, -1 on error (errno set)
*/
static int ensure_parent_dirs(const char *path, mode_t mode) {
    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    char *dir = dirname(tmp); /* dirname may modify tmp */

    /* If dirname returns "." that means no parent to create */
    if (dir == NULL || strcmp(dir, ".") == 0) {
        return 0;
    }

    /* iterate components */
    char acc[PATH_MAX] = {0};
    size_t len = 0;
    const char *p = dir;
    if (*p == '/') {
        acc[len++] = '/';
        p++; /* skip leading slash */
    }
    char comp[PATH_MAX];
    while (*p) {
        const char *slash = strchr(p, '/');
        size_t comp_len = slash ? (size_t)(slash - p) : strlen(p);
        if (comp_len == 0) {
            p = slash ? slash + 1 : p + comp_len;
            continue;
        }
        if (len + comp_len + 2 > sizeof(acc)) {
            errno = ENAMETOOLONG;
            return -1;
        }
        if (!(len == 1 && acc[0] == '/')) { /* not to add extra slash */
            acc[len++] = '\0'; /* put NUL to allow strcat to work, we'll overwrite */
            acc[len-1] = '\0';
        }
        /* append component */
        if (acc[0] == '\0') {
            memcpy(acc, p, comp_len);
            len = comp_len;
            acc[len] = '\0';
        } else if (acc[0] == '/' && len == 1) {
            /* root + component */
            memcpy(acc + len, p, comp_len);
            len += comp_len;
            acc[len] = '\0';
        } else {
            acc[len++] = '/';
            memcpy(acc + len, p, comp_len);
            len += comp_len;
            acc[len] = '\0';
        }

        /* create if not exist */
        if (mkdir(acc, mode) != 0) {
            if (errno != EEXIST) {
                return -1;
            }
            /* else exists: ok */
        }
        if (!slash) break;
        p = slash + 1;
    }
    return 0;
}

/*
* List archive implementation
*
* @param path Path to the archive
* @param verbose If true, print more information
* @return 0 on success, -1 on error (errno set)
*/
int tar_list(const char *path, bool verbose) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    unsigned char block[BLOCK_SIZE];
    int consecutive_zero_blocks = 0;

    while (1) {
        ssize_t r = read(fd, block, BLOCK_SIZE);
        if (r == 0) { /* EOF */
            break;
        }
        if (r < 0) {
            perror("read");
            close(fd);
            return -1;
        }
        if (r < BLOCK_SIZE) {
            fprintf(stderr, "short read on header (expected %d got %zd)\n", BLOCK_SIZE, r);
            close(fd);
            return -1;
        }

        if (is_zero_block(block)) {
            consecutive_zero_blocks++;
            if (consecutive_zero_blocks >= 2) {
                /* end of archive */
                break;
            }
            continue;
        } else {
            consecutive_zero_blocks = 0;
        }

        struct posix_header hdr;
        memcpy(&hdr, block, sizeof(hdr));

        unsigned long computed = compute_checksum(block);
        unsigned long stored = octal_to_size(hdr.chksum, sizeof(hdr.chksum));
        if (computed != stored && verbose) {
            dprintf(STDERR_FILENO, "warning: checksum mismatch (computed=%lu stored=%lu)\n", computed, stored);
        }

        char filename[PATH_MAX];
        memset(filename, 0, sizeof(filename));
        build_filename(&hdr, filename, sizeof(filename));

        size_t fsize = octal_to_size(hdr.size, sizeof(hdr.size));

        char t = hdr.typeflag;
        const char *type_str = "unknown";
        if (t == '0' || t == '\0') type_str = "file";
        else if (t == '5') type_str = "dir";
        else if (t == '2') type_str = "symlink";
        else type_str = "other";

        printf("%-7s %8zu  %s\n", type_str, fsize, filename);

        /* skip file data */
        off_t skip = ((off_t)fsize + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE;
        if (skip > 0) {
            off_t new = lseek(fd, skip, SEEK_CUR);
            if (new == (off_t)-1) {
                perror("lseek");
                close(fd);
                return -1;
            }
        }
    }

    close(fd);
    return 0;
}

/*
* Extract archive implementation
*
* @param path Path to the archive
* @param verbose If true, print more information
* @return 0 on success, -1 on error (errno set)
*/
int tar_extract(const char *path, bool verbose) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    unsigned char block[BLOCK_SIZE];
    int consecutive_zero_blocks = 0;

    while (1) {
        ssize_t r = read(fd, block, BLOCK_SIZE);
        if (r == 0) { /* EOF */
            break;
        }
        if (r < 0) {
            perror("read");
            close(fd);
            return -1;
        }
        if (r < BLOCK_SIZE) {
            fprintf(stderr, "short read on header (expected %d got %zd)\n", BLOCK_SIZE, r);
            close(fd);
            return -1;
        }

        if (is_zero_block(block)) {
            consecutive_zero_blocks++;
            if (consecutive_zero_blocks >= 2) {
                /* end of archive */
                break;
            }
            continue;
        } else {
            consecutive_zero_blocks = 0;
        }

        struct posix_header hdr;
        memcpy(&hdr, block, sizeof(hdr));

        unsigned long computed = compute_checksum(block);
        unsigned long stored = octal_to_size(hdr.chksum, sizeof(hdr.chksum));
        if (computed != stored && verbose) {
            dprintf(STDERR_FILENO, "warning: checksum mismatch (computed=%lu stored=%lu)\n", computed, stored);
        }

        char filename[PATH_MAX];
        memset(filename, 0, sizeof(filename));
        build_filename(&hdr, filename, sizeof(filename));

        size_t fsize = octal_to_size(hdr.size, sizeof(hdr.size));
        mode_t mode = (mode_t)octal_to_size(hdr.mode, sizeof(hdr.mode)) & 07777;
        time_t mtime = (time_t)octal_to_size(hdr.mtime, sizeof(hdr.mtime));

        char t = hdr.typeflag;
        if (t == '\0') t = '0'; /* some tars use NUL for regular files */

        if (t == '5') {
            /* directory */
            if (mkdir(filename, mode) != 0) {
                if (errno != EEXIST) {
                    perror("mkdir");
                    close(fd);
                    return -1;
                }
            }
            /* set mtime */
            struct utimbuf times;
            times.actime = mtime;
            times.modtime = mtime;
            utime(filename, &times);
            if (verbose) printf("mkdir: %s\n", filename);
        } else if (t == '2') {
            /* symlink: linkname field contains target */
            char linktarget[101] = {0};
            memcpy(linktarget, hdr.linkname, 100);
            linktarget[100] = '\0';
            /* ensure parent dirs */
            if (ensure_parent_dirs(filename, 0755) != 0) {
                perror("ensure_parent_dirs");
                close(fd);
                return -1;
            }
            /* remove existing file if any */
            unlink(filename);
            if (symlink(linktarget, filename) != 0) {
                perror("symlink");
                close(fd);
                return -1;
            }
            if (verbose) printf("symlink: %s -> %s\n", filename, linktarget);
        } else if (t == '0') {
            /* regular file */
            /* ensure parent dirs exist */
            if (ensure_parent_dirs(filename, 0755) != 0) {
                perror("ensure_parent_dirs");
                close(fd);
                return -1;
            }

            int outfd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode ? mode : 0644);
            if (outfd == -1) {
                perror("open output file");
                close(fd);
                return -1;
            }

            size_t remaining = fsize;
            char buf[READ_BUF_SIZE];
            while (remaining > 0) {
                ssize_t toread = (remaining > (size_t)READ_BUF_SIZE) ? READ_BUF_SIZE : (ssize_t)remaining;
                ssize_t rr = read(fd, buf, toread);
                if (rr <= 0) {
                    perror("read (file data)");
                    close(outfd);
                    close(fd);
                    return -1;
                }
                ssize_t ww = write(outfd, buf, rr);
                if (ww != rr) {
                    perror("write (output file)");
                    close(outfd);
                    close(fd);
                    return -1;
                }
                remaining -= rr;
            }
            close(outfd);

            /* skip padding to 512 boundary */
            off_t total = ((off_t)fsize + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE;
            off_t pad = total - (off_t)fsize;
            if (pad > 0) {
                off_t new = lseek(fd, pad, SEEK_CUR);
                if (new == (off_t)-1) {
                    perror("lseek");
                    close(fd);
                    return -1;
                }
            }

            /* set permissions explicitly (in case umask interfered) */
            if (chmod(filename, mode) != 0) {
                /* not fatal in all cases, but report if verbose */
                if (verbose) perror("chmod");
            }

            /* set mtime */
            struct utimbuf times;
            times.actime = mtime;
            times.modtime = mtime;
            if (utime(filename, &times) != 0) {
                if (verbose) perror("utime");
            }

            if (verbose) printf("extract: %s (%zu bytes)\n", filename, fsize);
            continue; /* already advanced file pointer */
        } else {
            /* unknown or unsupported type: skip its data */
            if (verbose) dprintf(STDERR_FILENO, "unsupported typeflag '%c' for '%s' — skipping\n", t, filename);
            off_t skip = ((off_t)fsize + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE;
            if (skip > 0) {
                off_t new = lseek(fd, skip, SEEK_CUR);
                if (new == (off_t)-1) {
                    perror("lseek");
                    close(fd);
                    return -1;
                }
            }
        }
    }

    close(fd);
    return 0;
}
