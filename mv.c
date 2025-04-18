// user/mv.c

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUF_SIZE 512

int
main(int argc, char *argv[]) {
    if (argc != 3) {
        write(2, "Usage: mv <source> <destination>\n", 34);
        exit(1);
    }

    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        write(2, "mv: cannot open source file\n", 29);
        exit(1);
    }

    int dst_fd = open(argv[2], O_CREATE | O_WRONLY);
    if (dst_fd < 0) {
        write(2, "mv: cannot create destination file\n", 35);
        close(src_fd);
        exit(1);
    }

    char buffer[BUF_SIZE];
    int n;

    while ((n = read(src_fd, buffer, BUF_SIZE)) > 0) {
        if (write(dst_fd, buffer, n) != n) {
            write(2, "mv: write error\n", 17);
            close(src_fd);
            close(dst_fd);
            exit(1);
        }
    }

    close(src_fd);
    close(dst_fd);

    // Delete original file after copying
    if (unlink(argv[1]) < 0) {
        write(2, "mv: failed to delete source file\n", 34);
        exit(1);
    }

    exit(0);
}
