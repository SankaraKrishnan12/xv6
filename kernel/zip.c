#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"
#include "memlayout.h"

#define MAX_COMPRESS_SIZE 1024

void print_str(char *s) {
    for(; *s; s++) consputc(*s);
}

static int compress(uint8 *input, uint8 *output, int length) {
    int out_pos = 0;
    int in_pos = 0;
    const int max_out = length * 2;

    print_str("compressing: ");
    for(int i = 0; i < (length > 16 ? 16 : length); i++) {
        print_hex(input[i]);
        consputc(' ');
    }
    print_str("\n");

    while(in_pos < length && out_pos < max_out - 2) {
        int best_len = 0;
        int best_pos = 0;
        int search_start = (in_pos > 255) ? in_pos - 255 : 0;

        // Find longest match
        for(int i = search_start; i < in_pos; i++) {
            int len = 0;
            while(in_pos + len < length && 
                  input[i + len] == input[in_pos + len] &&
                  len < 255) {
                len++;
            }
            if(len > best_len) {
                best_len = len;
                best_pos = in_pos - i;
            }
        }

        if(best_len > 1) {  // Encode match
            output[out_pos++] = best_pos;
            output[out_pos++] = best_len;
            in_pos += best_len;
            
            print_str("match: pos=");
            print_hex(best_pos);
            print_str(" len=");
            print_hex(best_len);
            print_str("\n");
        } else {  // Encode literal
            output[out_pos++] = 0;
            output[out_pos++] = input[in_pos++];
            
            print_str("literal: ");
            print_hex(output[out_pos-1]);
            print_str("\n");
        }
    }
    return out_pos;
}

static int decompress(uint8 *input, uint8 *output, int length) {
    int out_pos = 0;
    int in_pos = 0;

    while(in_pos < length) {
        if(in_pos + 1 >= length) break;

        uint8 offset = input[in_pos++];
        uint8 len = input[in_pos++];

        if(offset == 0) {  // Literal
            output[out_pos++] = len;
        } else {  // Backreference
            for(int i = 0; i < len; i++) {
                if(out_pos - offset < 0) return -1;
                output[out_pos] = output[out_pos - offset];
                out_pos++;
            }
        }
    }
    return out_pos;
}

uint64 sys_zip(void) {
    char src[MAXPATH], dst[MAXPATH];
    
    if(argstr(0, src, MAXPATH) < 0 || argstr(1, dst, MAXPATH) < 0)
        return -1;

    begin_op();
    
    struct inode *ip = namei(src);
    if(!ip) {
        end_op();
        return -1;
    }

    ilock(ip);
    if(ip->type != T_FILE || ip->size == 0 || ip->size > MAX_COMPRESS_SIZE) {
        iunlockput(ip);
        end_op();
        return -1;
    }

    uint size = ip->size;
    char *input = kalloc();
    char *output = kalloc();
    
    if(readi(ip, 0, (uint64)input, 0, size) != size) {
        kfree(input);
        kfree(output);
        iunlockput(ip);
        end_op();
        return -1;
    }
    iunlockput(ip);

    int csize = compress((uint8*)input, (uint8*)output, size);
    if(csize <= 0) {
        kfree(input);
        kfree(output);
        end_op();
        return -1;
    }

    struct inode *op = create(dst, T_FILE, 0, 0);
    if(!op || writei(op, 0, (uint64)output, 0, csize) != csize) {
        kfree(input);
        kfree(output);
        if(op) iunlockput(op);
        end_op();
        return -1;
    }

    iunlockput(op);
    kfree(input);
    kfree(output);
    end_op();
    return 0;
}

uint64 sys_unzip(void) {
    char src[MAXPATH], dst[MAXPATH];
    
    if(argstr(0, src, MAXPATH) < 0 || argstr(1, dst, MAXPATH) < 0)
        return -1;

    begin_op();
    
    struct inode *ip = namei(src);
    if(!ip) {
        end_op();
        return -1;
    }

    ilock(ip);
    if(ip->type != T_FILE || ip->size == 0 || ip->size > MAX_COMPRESS_SIZE*2) {
        iunlockput(ip);
        end_op();
        return -1;
    }

    uint size = ip->size;
    char *input = kalloc();
    char *output = kalloc();
    
    if(readi(ip, 0, (uint64)input, 0, size) != size) {
        kfree(input);
        kfree(output);
        iunlockput(ip);
        end_op();
        return -1;
    }
    iunlockput(ip);

    int usize = decompress((uint8*)input, (uint8*)output, size);
    if(usize <= 0) {
        kfree(input);
        kfree(output);
        end_op();
        return -1;
    }

    struct inode *op = create(dst, T_FILE, 0, 0);
    if(!op || writei(op, 0, (uint64)output, 0, usize) != usize) {
        kfree(input);
        kfree(output);
        if(op) iunlockput(op);
        end_op();
        return -1;
    }

    iunlockput(op);
    kfree(input);
    kfree(output);
    end_op();
    return 0;
}