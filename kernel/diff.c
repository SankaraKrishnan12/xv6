#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "printf.h"
#include "proc.h"
#include "syscall.h"
#include "stat.h"


uint64 sys_diff(void) {
    char path1[MAXPATH], path2[MAXPATH];
    struct inode *ip1 = 0, *ip2 = 0;
    char buf1[BSIZE], buf2[BSIZE];
    int r = 0;

    // Get paths
    if(argstr(0, path1, MAXPATH) < 0 || argstr(1, path2, MAXPATH) < 0) {
        return -1;
    }

    begin_op();
    
    // Open files
    if((ip1 = namei(path1)) == 0) {
        printf("diff: %s not found\n", path1);
        r = -1;
        goto cleanup;
    }

    if((ip2 = namei(path2)) == 0) {
        printf("diff: %s not found\n", path2);
        r = -1;
        goto cleanup;
    }

    // Lock inodes in address order to prevent deadlock
    struct inode *first = (ip1 < ip2) ? ip1 : ip2;
    struct inode *second = (ip1 < ip2) ? ip2 : ip1;
    ilock(first);
    ilock(second);

    // Validate file types
    if(ip1->type != T_FILE || ip2->type != T_FILE) {
        printf("diff: not regular files\n");
        r = -1;
        goto cleanup;
    }

    // Read sizes
    uint size1 = ip1->size;
    uint size2 = ip2->size;
    uint off = 0;
    int differences = 0;

    // Compare chunks
    while(off < size1 || off < size2) {
        uint n1 = (off < size1) ? ((size1 - off > BSIZE) ? BSIZE : size1 - off) : 0;
        uint n2 = (off < size2) ? ((size2 - off > BSIZE) ? BSIZE : size2 - off) : 0;
        uint n = (n1 < n2) ? n1 : n2;

        // Read from each file
        if(n1 > 0 && readi(ip1, 0, (uint64)buf1, off, n) != n) {
            printf("diff: error reading %s\n", path1);
            r = -1;
            goto cleanup;
        }
        if(n2 > 0 && readi(ip2, 0, (uint64)buf2, off, n) != n) {
            printf("diff: error reading %s\n", path2);
            r = -1;
            goto cleanup;
        }

        // Compare bytes in this chunk
        for(uint i = 0; i < n; i++) {
            if(buf1[i] != buf2[i]) {
                printf("byte %d: 0x%x vs 0x%x\n", off + i, buf1[i], buf2[i]);
                differences++;
            }
        }

        // Handle remaining bytes in the larger chunk
        if(n1 != n2) {
            uint extra = (n1 > n2) ? (n1 - n2) : (n2 - n1);
            differences += extra;
            const char *path = (n1 > n2) ? path1 : path2;
            printf("%d extra bytes at offset %d in %s\n", extra, off + n, path);
        }

        off += BSIZE;
    }

    // Check total size mismatch
    if(size1 != size2) {
        uint size_diff = (size1 > size2) ? (size1 - size2) : (size2 - size1);
        differences += size_diff;
        printf("size mismatch: %d vs %d bytes\n", size1, size2);
    }

    printf("Total differences: %d\n", differences);
    r = differences;

cleanup:
    // Unlock inodes (order doesn't matter for iunlockput)
    if(ip1) iunlockput(ip1);
    if(ip2 && ip2 != ip1) iunlockput(ip2); // Avoid double put if ip1 == ip2
    end_op();
    
    return r;
}