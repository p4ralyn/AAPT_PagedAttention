#include "kernel/types.h"
#include "user.h"

int main() {
    printf("--- AAPT CoW Test Starting ---\n");
    
    char *mem = sbrk(4096); // Allocate 1 page
    mem[0] = 'A';           // Parent writes 'A'
    
    printf("Parent set data: %c\n", mem[0]);

    int pid = aapt_cow_fork();
    
    if(pid < 0) {
        printf("Fork failed!\n");
    } else if(pid == 0) {
        // CHILD
        printf("Child: Reading Parent's data: %c\n", mem[0]);
        printf("Child: Attempting to write (triggers CoW Page Fault)...\n");
        mem[0] = 'B'; 
        printf("Child: Write successful. Data: %c\n", mem[0]);
        exit(0);
    } else {
        // PARENT
        wait(0);
        printf("Parent: Checking if data is still 'A': %c\n", mem[0]);
        if(mem[0] == 'A') printf("SUCCESS: Memory was decoupled correctly!\n");
    }
    
    exit(0);
}