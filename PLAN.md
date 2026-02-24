# CS314 Project #1: Synchronized Swimming Pool Control - Implementation Plan

**Student ID:** 510  
**Source File:** `P1_510.c`  
**Semaphore Keys:** 8370-8379  
**Shared Memory Keys:** 7205-7209

---

## Overview

This plan implements a solution for the "synchronized swimming pool control problem" using UNIX standard semaphores and shared memory. The system consists of:
- **1 Safeguard (S)** - parent process that creates all others
- **2 Boiler-men (B1, B2)** - heat water, repeat NUM_REPEAT times
- **3 Bathers (A1, A2, A3)** - enter/leave pool, terminate after boiler-men finish

---

## PHASE 1: Environment Setup & IPC Test (1 hour) ✓ COMPLETE

### Task 1.1: Create Basic File Structure
- [x] Create `P1_510.c` in project root
- [x] Add file header comment block with:
  - "CS314 Project #1 solution"
  - Last-three: 510
  - Course section #: 003
  - Spring 2026

### Task 1.2: Add Required Includes
```c
#include <stdio.h>          // for printf
#include <stdlib.h>         // for rand, exit
#include <sys/types.h>      // for pid_t
#include <unistd.h>         // for usleep, fork
#include <time.h>           // for time
#include <sys/ipc.h>        // for IPC
#include <sys/shm.h>        // for shared memory
#include <sys/sem.h>        // for semaphores
#include <sys/wait.h>       // for wait
```

### Task 1.3: Define Constants with Personal Keys
- [x] Define `NUM_REPEAT` = 50
- [x] Define timing constants (BATHER_TIME_xx_A/B, BOILERMAN_TIME_xx_A/B, SAFEGUARD_TIME_A/B)
- [x] Define semaphore keys using personal range: SEM_KEY_01 = 8370, SEM_KEY_02 = 8371
- [x] Define shared memory key: SHM_KEY = 7205

### Task 1.4: Implement rand_sleep() Function
- [x] Create function to generate random sleep time (0 to max_time)
- [x] Test: compile and run a simple loop to verify random timing works

### Task 1.5: Test Semaphore Creation/Deletion
- [x] Create a semaphore using `semget()` with key 8370
- [x] Initialize semaphore to 1 using `semctl()` with SETVAL
- [x] Delete semaphore using `semctl()` with IPC_RMID
- [x] Verify with `ipcs -s` command before/after

### Task 1.6: Test Shared Memory Creation/Deletion
- [x] Create shared memory using `shmget()` with key 7205
- [x] Attach using `shmat()`
- [x] Initialize a test value
- [x] Detach using `shmdt()`
- [x] Delete using `shmctl()` with IPC_RMID
- [x] Verify with `ipcs -m` command before/after

---

## PHASE 2: Fork System Call Test (1 hour)

### Task 2.1: Test Single fork()
- [ ] Write a simple test that creates one child process
- [ ] Child prints "I am the child"
- [ ] Parent prints "I am the parent"
- [ ] Parent waits for child with `wait()`

### Task 2.2: Understand fork() Behavior
- [ ] Document which variables are inherited (all variables, file descriptors)
- [ ] Document which are NOT inherited (PID, PPID, pending signals)
- [ ] Note: Child gets a COPY of parent's memory, not shared

### Task 2.3: Test Sequential fork() Calls
- [ ] Create 2 child processes sequentially (not in a loop)
- [ ] Each child identifies itself with a unique ID
- [ ] Parent waits for both children
- [ ] Verify order of creation and termination

---

## PHASE 3: Create Five Child Processes (1-4 hours)

### Task 3.1: Define Shared Memory Structure
```c
struct shared_memory {
    int ready_count;      // # of processes ready
    int start_flag;       // 1 = all processes can start
    int bw1, bw2;         // boiler-man status (1=active, 0=done)
    int ba1, ba2, ba3;    // bather status (1=active, 0=done)
    int bather_count;     // # of bathers in pool
};
```

### Task 3.2: Create Shared Memory and Semaphores
- [ ] Create shared memory segment (size = sizeof(struct shared_memory))
- [ ] Attach shared memory to process
- [ ] Initialize all shared memory fields to 0
- [ ] Create semaphore S1 (mutex for pool access) - initialize to 1
- [ ] Create semaphore S2 (bather count mutex) - initialize to 1

### Task 3.3: Implement Process Creation (Sequential fork)
**CRITICAL: Create processes in this exact order:**
1. Parent process becomes Safeguard (S)
2. fork() → B1 (Boiler-man 1)
3. fork() → B2 (Boiler-man 2)
4. fork() → A1 (Bather 1)
5. fork() → A2 (Bather 2)
6. fork() → A3 (Bather 3)

### Task 3.4: Assign Process IDs
- [ ] After each fork(), assign a process identifier variable in the child
- [ ] B1 gets ID=1, B2 gets ID=2, A1 gets ID=1, A2 gets ID=2, A3 gets ID=3
- [ ] Parent (S) knows it's the safeguard

### Task 3.5: Implement Startup Synchronization
**Each child process must:**
- [ ] Increment `ready_count` in shared memory
- [ ] Wait (sleep or semaphore) until `start_flag == 1`

**Parent (S) must:**
- [ ] Wait until `ready_count == 5`
- [ ] Set `start_flag = 1`
- [ ] All 6 processes can now begin work

### Task 3.6: Verify Process Count
- [ ] Run `ps -a` while program is running
- [ ] Confirm exactly 6 processes exist (1 parent + 5 children)
- [ ] Reference: `docs/assets/creating_five_child_processes.jpg`

### Task 3.7: Test Clean Termination
- [ ] Each child exits after a short delay
- [ ] Parent waits for all 5 children using `wait()` in a loop
- [ ] Parent deletes shared memory and semaphores
- [ ] Parent exits last
- [ ] Verify no orphan processes remain (`ps -a`)

---

## PHASE 4: Synchronization with Semaphores (1-2 days)

### Task 4.1: Implement Semaphore Wait Operation
```c
void semaphore_wait(int sem_id) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;    // decrement (wait/P operation)
    op.sem_flg = 0;    // blocking
    semop(sem_id, &op, 1);
}
```

### Task 4.2: Implement Semaphore Signal Operation
```c
void semaphore_signal(int sem_id) {
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;     // increment (signal/V operation)
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}
```

### Task 4.3: Test Mutual Exclusion
- [ ] Create a simple test where parent and child both increment a counter
- [ ] Protect counter access with semaphore wait/signal
- [ ] Verify counter reaches expected final value (no race condition)

### Task 4.4: Implement Process Start Synchronization (No Spin-Wait!)
**Option A: Use a semaphore for process start**
- [ ] Create a start semaphore initialized to 0
- [ ] Children wait on this semaphore
- [ ] Parent signals 5 times after all children are created

**Option B: Use shared memory with usleep() polling**
- [ ] Children poll `start_flag` with small `usleep()` delays
- [ ] NOT a busy loop because it includes sleep

---

## PHASE 5: Implement Process Logic (1-3 days)

### Task 5.1: Implement Safeguard (S) Logic

**Required Structure (Figure 4):**
```c
while (one of the three bathers is still active)
{
    sleep_time = rand_sleep(SAFEGUARD_TIME_A);
    usleep(sleep_time);

    semaphore_wait(S1);  // Enter critical section

    // Critical section
    printf("S starts inspection ...\n");
    sleep_time = rand_sleep(SAFEGUARD_TIME_B);
    usleep(sleep_time);
    printf("S is leaving the bathing area ..\n");

    semaphore_signal(S1);  // Exit critical section
}
```

**Subtasks:**
- [ ] Implement while loop condition: `while (ba1 == 1 || ba2 == 1 || ba3 == 1)`
- [ ] Add wait/signal around critical section
- [ ] Verify correct printf messages
- [ ] After loop ends, delete semaphores and shared memory

### Task 5.2: Implement Boiler-man (B1, B2) Logic

**Required Structure (Figure 3):**
```c
for (i = 0; i < NUM_REPEAT; i++)
{
    sleep_time = rand_sleep(BOILERMAN_TIME_xx_A);
    usleep(sleep_time);

    semaphore_wait(S1);  // Enter critical section

    // Critical section
    printf("B%d starts his boiler ...\n", my_BID);
    sleep_time = rand_sleep(BOILERMAN_TIME_xx_B);
    usleep(sleep_time);
    printf("B%d finishes water heating\n", my_BID);

    semaphore_signal(S1);  // Exit critical section
}
```

**Subtasks:**
- [ ] B1 uses BOILERMAN_TIME_01_A/B
- [ ] B2 uses BOILERMAN_TIME_02_A/B
- [ ] Each uses correct ID (1 or 2) in printf
- [ ] After loop: set bw1=0 (or bw2=0) in shared memory
- [ ] Exit process

### Task 5.3: Implement Starvation-Free Boiler-men (15% of Grade!)

**Problem:** If B1 and B2 both use simple mutex, one could repeatedly acquire while other starves.

**Solution Options:**
- [ ] Implement turn-based signaling (B1 goes, then B2, then B1, etc.)
- [ ] Or implement fair queueing semaphore
- [ ] Test: run many times, verify both B1 and B2 complete all NUM_REPEAT iterations

### Task 5.4: Implement Bather (A1, A2, A3) Logic

**Required Structure (Figure 2):**
```c
while (bw1 == 1 || bw2 == 1)  // While boilermen are active
{
    sleep_time = rand_sleep(BATHER_TIME_xx_A);
    usleep(sleep_time);

    // Readers-writer pattern for bathers (multiple can enter)
    semaphore_wait(S2);
    bather_count++;
    if (bather_count == 1) {
        semaphore_wait(S1);  // First bather locks pool
    }
    semaphore_signal(S2);

    // Critical section
    printf("A%d is entering the swimming pool\n", my_AID);
    sleep_time = rand_sleep(BATHER_TIME_xx_B);
    usleep(sleep_time);
    printf("A%d is leaving the swimming pool\n", my_AID);

    semaphore_wait(S2);
    bather_count--;
    if (bather_count == 0) {
        semaphore_signal(S1);  // Last bather unlocks pool
    }
    semaphore_signal(S2);
}
```

**Subtasks:**
- [ ] A1 uses BATHER_TIME_01_A/B
- [ ] A2 uses BATHER_TIME_02_A/B
- [ ] A3 uses BATHER_TIME_03_A/B
- [ ] Each uses correct ID (1, 2, or 3) in printf
- [ ] After loop: set ba1=0 (or ba2/ba3) in shared memory
- [ ] Exit process

### Task 5.5: Verify Multiple Bathers Can Enter Pool
- [ ] Test that multiple bathers can be in pool simultaneously
- [ ] Check output for overlapping "entering" without "leaving" between
- [ ] This is a grading requirement (see requirement 3d)

---

## PHASE 6: Cleanup and Final Verification

### Task 6.1: Implement Proper Cleanup Sequence
- [ ] B1, B2, A1, A2, A3: exit after their loops
- [ ] S (safeguard): waits for all 5 children using `wait()` loop
- [ ] S: deletes semaphores using `semctl(sem_id, 0, IPC_RMID)`
- [ ] S: detaches shared memory using `shmdt()`
- [ ] S: deletes shared memory using `shmctl(shm_id, IPC_RMID, NULL)`
- [ ] S: exits last

### Task 6.2: Verify No Resource Leaks
- [ ] Run `ipcs -s` after program ends - should show no semaphores for my keys
- [ ] Run `ipcs -m` after program ends - should show no shared memory for my keys
- [ ] Run `ps -a` after program ends - should show no zombie processes

### Task 6.3: Code Style Check
- [ ] Add in-line comments explaining logic
- [ ] Ensure consistent indentation
- [ ] Remove any debug printf statements (only required printf allowed)

### Task 6.4: Remove macOS Compatibility Code
- [ ] Remove the `#ifndef __APPLE__` block around `union semun` definition
- [ ] Ensure `union semun` is defined unconditionally for SIUE Unix server

---

## PHASE 7: Stress Testing

### Task 7.1: Swap in Test Configuration 1
Replace constants with values from `docs/05_test_configs.md`:
```c
#define NUM_REPEAT 15
#define BATHER_TIME_01_A 500000
#define BATHER_TIME_01_B 300000
// ... etc
```

### Task 7.2: Run Extended Test
- [ ] Compile and run with test config
- [ ] Monitor for deadlocks (program hangs)
- [ ] Monitor for starvation (one process never completes)
- [ ] Verify all processes terminate correctly

### Task 7.3: Restore Original Constants
- [ ] Swap back to original timing values before submission

---

## Grading Checklist

| Requirement | Status |
|-------------|--------|
| (a) Single C file | ☐ |
| (b) All 6 processes concurrent | ☐ |
| (c) Safeguard creates B1, B2, A1, A2, A3 in order | ☐ |
| (d) Children wait for all to be created | ☐ |
| (e) Correct "entering" messages | ☐ |
| (f) Correct "leaving" message for bathers | ☐ |
| (g) Correct "finishes water heating" for boiler-men | ☐ |
| (h) Correct "finishes inspection" for safeguard | ☐ |
| (i) Random wait times used | ☐ |
| (j) Bathing terminates after boiler-men finish | ☐ |
| (k) Safeguard deletes IPC resources | ☐ |
| (l) All timing constants defined | ☐ |
| (m) Figures 2, 3, 4 structures followed | ☐ |
| No starvation | ☐ |
| No mutual exclusion violation (boiler-men, safeguard) | ☐ |
| No deadlock | ☐ |
| Multiple bathers can enter pool | ☐ |
| No busy loop (spin wait) | ☐ |

---

## Key Files Reference

| File | Purpose |
|------|---------|
| `P1_510.c` | Main source code (submission file) |
| `docs/01_specifications.md` | Requirements and grading criteria |
| `docs/02_starter_code.md` | Template code and constants |
| `docs/03_logic_references.md` | Semaphore logic, example code |
| `docs/04_personal_keys.md` | Semaphore keys: 8370-8379, SHM: 7205-7209 |
| `docs/05_test_configs.md` | Alternative test parameters |
| `docs/assets/appendix2_the_required_process_structure.png` | Process hierarchy diagram |
| `docs/assets/appendix3_the_required_source_code_structure_in_your_p1.png` | Code structure diagram |

---

## Compilation Command

```bash
gcc -o P1_510 P1_510.c
./P1_510
```

## Verification Commands

```bash
# Check IPC resources before/after run
ipcs -s    # List semaphores
ipcs -m    # List shared memory

# Check processes during run
ps -a

# Remove stuck IPC resources (if needed)
ipcrm -s <sem_id>
ipcrm -m <shm_id>
```
