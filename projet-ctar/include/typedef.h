#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

/*
 * POSIX ustar header (tar) — champs et tailles standard.
 * Total = 512 octets.
 */
struct posix_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

#endif /* __TYPEDEF_H__ */
