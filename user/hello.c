// user/hello.c

#include "kernel/types.h"
#include "user/user.h"

int main() {
    hello();  // Call the syscall
    exit(0);
}
