#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

uint64
sys_filedetails(void)
{
    struct file_details fd;
    char path[MAXPATH];
    struct inode *ip;
    uint64 user_fd_addr;
    
    // Get path from user space
    if (argstr(0, path, MAXPATH) < 0)
        return -1;
    
    // Get user-space address (no error check needed)
    argaddr(1, &user_fd_addr);
    
    begin_op();
    if ((ip = namei(path)) == 0) {
        end_op();
        return -1;
    }
    
    ilock(ip);
    fd.inum = ip->inum;
    fd.type = ip->type;
    fd.major = ip->major;
    fd.minor = ip->minor;
    fd.nlink = ip->nlink;
    fd.size = ip->size;
    
    // Copy filename logic
    if (ip->type == T_DIR) {
        safestrcpy(fd.name, path, DIRSIZ);
    } else {
        char *p;
        for (p = path + strlen(path); p >= path && *p != '/'; p--)
            ;
        p++;
        safestrcpy(fd.name, p, DIRSIZ);
    }
    
    iunlock(ip);
    iput(ip);
    end_op();
    
    // Copy to user space (this does the actual validity check)
    if (copyout(myproc()->pagetable, user_fd_addr, (char*)&fd, sizeof(fd)) < 0)
        return -1;
    
    return 0;
}