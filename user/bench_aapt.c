#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PROMPT_PAGES 10
#define PGSIZE 4096
#define NUM_AGENTS 3

int main(void) {
    printf("\n--- AAPT Mock SLM Benchmark ---\n");
    int initial_free = count_free_pages(); 

    char *shared_prompt = malloc(PROMPT_PAGES * PGSIZE);
    for(int i = 0; i < PROMPT_PAGES * PGSIZE; i += PGSIZE) {
        shared_prompt[i] = 'A'; 
    }
    
    int after_prompt = count_free_pages();
    printf("Base system footprint: %d pages\n", initial_free - after_prompt);

    int pid;
    for(int i = 0; i < NUM_AGENTS; i++) {
        pid = aapt_cow_fork(); 
        if(pid == 0) {
            // --- CHILD PROCESS ---
            pause(10 + (i * 10)); // Stagger the output to prevent console scrambling
            
            // 1. Log memory BEFORE writing (Should show shared state)
            int before_write = count_free_pages();
            
            // 2. Trigger the Trap (Token Generation)
            for(int p = 0; p < PROMPT_PAGES; p++) {
                shared_prompt[p * PGSIZE] = 'B' + getpid(); 
            }
            
            // 3. Log Peak Memory AFTER trap duplication
            int peak_memory = count_free_pages();
            
            printf("[Agent %d] Pages duplicated by trap: %d\n", getpid(), before_write - peak_memory);
            
            // Sleep so all agents exist concurrently holding peak memory
            pause(50); 
            exit(0);
        }
    }

    // --- PARENT PROCESS ---
    int after_fork = count_free_pages();
    printf("[Parent] Overhead for %d forks: %d pages\n", NUM_AGENTS, after_prompt - after_fork);

    // Wait for the chaos to finish
    for(int i = 0; i < NUM_AGENTS; i++) {
        wait(0);
    }

    printf("\n[Parent] All agents terminated. Cleanup successful.\n");
    exit(0);
}