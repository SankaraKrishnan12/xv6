#include "kernel/types.h"
#include "user/user.h"

#define NUM_PHIL 5
#define EAT_TIME 20
#define THINK_TIME 20
#define MEALS 3

struct fork {
    int locked;
};

struct fork forks[NUM_PHIL];

// Atomic operations
int test_and_set(int *lock) {
    int old = *lock;
    *lock = 1;
    return old;
}

void reset_lock(int *lock) {
    *lock = 0;
}

// Global output lock
int output_lock = 0;

void print_philosopher(int id, char *action) {
    // Wait for output lock
    while(test_and_set(&output_lock)) {
        sleep(1);
    }
    
    // Build message safely
    char buf[32];
    char *p = buf;
    if(id >= 0) {
        *p++ = '0' + id;
        *p++ = ' ';
    }
    char *a = action;
    while(*a) {
        *p++ = *a++;
    }
    *p++ = '\n';
    *p = '\0';
    
    // Write entire message atomically
    write(1, buf, p - buf);
    
    reset_lock(&output_lock);
}

void think(int id) {
    print_philosopher(id, "thinking");
    sleep(THINK_TIME);
}

void eat(int id) {
    print_philosopher(id, "eating");
    sleep(EAT_TIME);
}

void take_forks(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHIL;
    
    // Different order to prevent deadlock
    int first = (id % 2 == 0) ? left : right;
    int second = (id % 2 == 0) ? right : left;

    // Wait for first fork
    while(test_and_set(&forks[first].locked)) {
        sleep(1);
    }

    // Wait for second fork
    while(test_and_set(&forks[second].locked)) {
        sleep(1);
    }
}

void release_forks(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHIL;
    
    reset_lock(&forks[left].locked);
    reset_lock(&forks[right].locked);
}

void philosopher(int id) {
    for(int meals = 0; meals < MEALS; meals++) {
        think(id);
        take_forks(id);
        eat(id);
        release_forks(id);
    }
    exit(0);
}

int main() {
    // Initialize forks
    for(int i = 0; i < NUM_PHIL; i++) {
        forks[i].locked = 0;
    }
    
    print_philosopher(-1, "Dining philosophers start");
    
    for(int i = 0; i < NUM_PHIL; i++) {
        int pid = fork();
        if(pid == 0) {
            philosopher(i);
        }
    }
    
    for(int i = 0; i < NUM_PHIL; i++) {
        wait(0);
    }
    
    print_philosopher(-1, "All done");
    exit(0);
}