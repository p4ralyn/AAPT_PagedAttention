#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PROMPT_PAGES 10
#define PGSIZE 4096
#define NUM_AGENTS 3

int main(void) {
    printf("--- AAPT Mock SLM Benchmark ---\n");
    int initial_free = count_free_pages(); 
    printf("Initial free pages: %d\n", initial_free);

    // 1. Simulate the KV Cache Pre-fill (Parent allocates memory)
    char *shared_prompt = malloc(PROMPT_PAGES * PGSIZE);
    for(int i = 0; i < PROMPT_PAGES * PGSIZE; i += PGSIZE) {
        shared_prompt[i] = 'A'; // Force allocation
    }
    
    int after_prompt = count_free_pages();
    printf("Free pages after prompt load: %d (Used: %d)\n", after_prompt, initial_free - after_prompt);

    // 2. Spawn Agents using your custom AAPT CoW Fork
    int pid;
    for(int i = 0; i < NUM_AGENTS; i++) {
        pid = aapt_cow_fork(); // Call your custom fork here
        if(pid == 0) {
            // --- CHILD PROCESS (Agent) ---
            pause(10); // Wait so parent can log post-fork stats
            
            // 3. Simulate Token Generation (Trigger the trap!)
            printf("Agent %d generating tokens (triggering CoW)...\n", getpid());
            for(int p = 0; p < PROMPT_PAGES; p++) {
                shared_prompt[p * PGSIZE] = 'B' + getpid(); 
            }
            exit(0);
        }
    }

    // --- PARENT PROCESS ---
    int after_fork = count_free_pages();
    printf("Free pages after %d forks: %d (Should remain unchanged!)\n", NUM_AGENTS, after_fork);

    // Wait for children to finish
    for(int i = 0; i < NUM_AGENTS; i++) {
        wait(0);
    }

    int after_generation = count_free_pages();
    printf("Free pages after generation: %d\n", after_generation);

    exit(0);
}