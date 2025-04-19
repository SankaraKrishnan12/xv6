#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(2, "Usage: diff <file1> <file2>\n");
        exit(1);
    }
    
    int result = diff(argv[1], argv[2]);
    
    if(result < 0) {
        fprintf(2, "diff: comparison failed\n");
        exit(1);
    }
    
    exit(result ? 1 : 0);
}