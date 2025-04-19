#include "kernel/types.h"
// #include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Usage: filedetailstest <file>\n");
        exit(1);
    }
    
    struct file_details fd;
    
    if(filedetails(argv[1], &fd) < 0){
        printf("filedetails: failed to get details for %s\n", argv[1]);
        exit(1);
    }
    
    printf("File: %s\n", fd.name);
    printf("Inode: %d\n", fd.inum);
    printf("Type: %d\n", fd.type);
    printf("Size: %d bytes\n", fd.size);
    printf("Links: %d\n", fd.nlink);
    printf("Device: %d,%d\n", fd.major, fd.minor);
    
    exit(0);
}