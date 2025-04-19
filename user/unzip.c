#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(2, "Usage: %s <source> <destination>\n", argv[0]);
        exit(1);
    }
    
    if(unzip(argv[1], argv[2]) < 0) {
        fprintf(2, "unzip failed\n");
        exit(1);
    }
    
    exit(0);
}