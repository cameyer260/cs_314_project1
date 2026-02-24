Necessary Code Snippets:

int bather_count;   // # of the bather processes trying to enter and in the pool
int bw1, bw2;       // 1 = still on duty, 0 = done
int ba1, ba2, ba3;  // 1 = still there, 0 = done
semaphore S1;       // initialized by 1
semaphore S2;       // initialized by 1

// shared memory

while ((bw1 == 1) or (bw2 == 1))
{
    wait (S2);
    bather_count ++;

    if (bather_count == 1)
    { wait (S1); }
    signal (S2);

    // Missing code goes here

    wait (S2);
    bather_count --;
    if (bather_count == 0)
    { signal (S1); }
    signal (S2);
}

for (i = 0; i < NUM_REPEAT; i++)
{
    wait (S1);
    
    // Missing code goes here

    signal(S1);
}

while ((ba1 == 1) or (ba2 == 1) or (ba3 ==1))
{
    wait (S1);

    // Missing code goes here

    signal (S1);
}

/* *************************************************** *
 * This is a test program                              *
 *    With semaphore & shared memory                   *
 * *************************************************** */
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>  

// the followings are for semaphores ----- 
#include <sys/sem.h>
#include <sys/ipc.h>

// the followings are for shared memory ----
#include <sys/shm.h>   

#define NUM_REPEAT    10000     // number of loops for testing 
#define SEM_KEY       8712       // the semaphore key
#define SHM_KEY       5512       // the shared memory key 

int main (void)
{
   pid_t  process_id;  
   int    i;                     // external loop counter  
   int    j;                     // internal loop counter  
   int    k = 0;                 // dumy integer  

   int    sem_id;                // the semaphore ID  
   struct sembuf operations[1];  // Define semaphore operations 
   int    ret_val;               // system-call return value    

   int    shm_id;                // the shared memory ID 
   int    shm_size;              // the size of the shared memoy  
   struct my_mem * p_shm;        // pointer to the attached shared memory 

   // Semaphore control data structure ----
   union semun {
        int    val;  
        struct semid_ds  *buf;  
        ushort * arry;
   } argument; 
   argument.val = 1;   // the initial value of the semaphore   

   // shared memory definition ----   
   struct my_mem {
          long int counter;
          int      parent;
          int      child;  
   };    

   // find the shared memory size in bytes ----
   shm_size = sizeof(my_mem);   
   if (shm_size <= 0)
   {  
      fprintf(stderr, "sizeof error in acquiring the shared memory size. Terminating ..\n");
      exit(0); 
   }    
   
   // create a new semaphore -----
   sem_id = semget(SEM_KEY, 1, 0666 | IPC_CREAT); 
   if (sem_id < 0)
   {
      fprintf(stderr, "Failed to create a new semaphire. Terminating ..\n"); 
      exit(0);
   }

   // initialzie the new semapahore by 1 (zero) ----
   if (semctl(sem_id, 0, SETVAL, argument) < 0)
   {
      fprintf(stderr, "Failed to initialize the semapgore by 1. Terminating ..\n"); 
      exit(0);  
   }

   // create a shared memory ----
   shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);         
   if (shm_id < 0) 
   {
      fprintf(stderr, "Failed to create the shared memory. Terminating ..\n");  
      exit(0);  
   } 

   // attach the new shared memory ----
   p_shm = (struct my_mem *)shmat(shm_id, NULL, 0);     
   if (p_shm == (struct my_mem*) -1)
   {
      fprintf(stderr, "Failed to attach the shared memory.  Terminating ..\n"); 
      exit(0);   
   }   

   // initialize the shared memory ----
   p_shm->counter  = 0;  
   p_shm->parent   = 0;   
   p_shm->child    = 0;
 
   // spawn a child process ----
   process_id = fork();

   // The child process ----------------------------- //
   if (process_id == 0)
   {
        printf("I am the child process ...\n");

        // initialize child process ----    
        p_shm->child = 1; // child started .. 
        
        // wait for the parent to get ready ----
        while (p_shm->parent != 1)     
        { k = k + 1; }
        k = 0;  
 
        for (i = 0; i < NUM_REPEAT; i++)  
        { 
            // WAIT on the semaphore ----
            operations[0].sem_num = 0;
            operations[0].sem_op = -1; // "wait" on the semaphore   
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id, operations, 1);         
            if (ret_val != 0)  
            {   
                fprintf(stderr, "P-OP(wait) failed (child) ....\a\n");   
            }

            // subtract 1 from the shared memory ---
            p_shm->counter = p_shm->counter - 1;  
            printf("subtracter: %d\n", p_shm->counter);

            for (j = 0; j < 10000; j++)
            {  k = k + 1; }
        
            // SIGNAL on the semaphore ----
            operations[0].sem_num = 0;
            operations[0].sem_op  = 1; 
            operations[0].sem_flg = 0;  
            ret_val = semop(sem_id, operations, 1);   
            if (ret_val != 0)
            {
                fprintf(stderr, "V-OP (wait) failed (child) ....\a\n");    
            }  
        }

        // declare completion ----
        p_shm->child = 0;  

        // wait for the parent to complete ---
        k = 0;
        while (p_shm->parent != 0)
        {  k = k + 1; }
 
        exit(0); 
   }

   // The parent process ----------------------------- //   
   else 
   {     
        printf("I am the parent process ...\n");  

        // initialize the parent process ----
        p_shm->parent = 1;  // the parent process started ..

        // wait for the child pocess to get ready ----
        while (p_shm->child != 1)
        {  k = k + 1; }
        k = 0;   
        
        for (i = 0; i < NUM_REPEAT; i++)   
        {
            // WAIT on the semaphore -----
            operations[0].sem_num = 0;  // the first semapahore
            operations[0].sem_op  = -1; // "wait" on the semaphore
            operations[0].sem_flg = 0;  // make sure to block    
            ret_val = semop(sem_id, operations, 1); 
            if (ret_val != 0)
            {
                 fprintf(stderr, "P-OP (wait) failed ....\a\n"); 
            } 
 
            // THE CRITICAL SECTION ============= // 
            p_shm->counter = p_shm->counter + 1;  
            printf("adder: %d\n", p_shm->counter);  

            for (j = 0; j < 10000; j++)
            {  k = k + 1; }

            // SIGNAL on the semapahore ----
            operations[0].sem_num = 0;  
            operations[0].sem_op  = 1; // SIGNAL   
            operations[0].sem_flg = 0;
            ret_val = semop(sem_id, operations, 1);   
            if (ret_val != 0)  
            {
                fprintf(stderr, "V-OP (signal) failed ....\a\n");  
            }
        }

        // declare completion ----
        p_shm->parent = 0;  

        // wait for the child process to complete ---- 
        k = 0;
        while (p_shm->child != 0)  
        {  k = k + 1;  }   

        printf("Shared Memory Counter: %d\n", p_shm->counter);  

        // detach the shared memory ---
        ret_val = shmdt(p_shm);  
        if (ret_val != 0) 
        {  printf ("shared memory detach failed ....\n"); }

        ret_val = shmctl(shm_id, IPC_RMID, 0); 
        if (ret_val != 0)
        {  printf("shared memory ID remove ID failed ... \n"); } 

        ret_val = semctl(sem_id, IPC_RMID, 0);  
        if (ret_val != 0)
        {  printf("semaphore remove ID failed ... \n"); }
 
        exit(0);  
   }
}

Additional Code Snippets to help out:

/* ********************************************** *
 * fork_test.c                                    *
 *                                                *
 *  For an example of "fork" system call.         *
 *                                                *
 *  The first (parent) process creates a child    *
 *  (duplicated) process.  Each of them repeats   *
 *  as many as NUM_REPEATS times.                 *
 *                                                *
 *  gcc fork_test.c                               *
 *                                                *
 *  os.cs.siue.edu  1/28/2026                     *
 * ********************************************** */
#include <stdio.h>          // for printf, rand
#include <unistd.h>         // for sleep
#include <sys/types.h>      // for type "pid_t"
#include <stdlib.h> 

#define NUM_REPEATS   100   // number of repeats
#define  MAX_RAND       5   // the max. random number  


int main (void)
{
   int i;                   // loop counter
   int my_rand;             // a random number

   pid_t  process_id;       // process ID holer


   process_id = fork();

   // The child process ------------------------- //
   if (process_id == 0)
   {
        printf("I am the child process ...\n");

        for (i = 0; i < NUM_REPEATS; i++)
        {
            my_rand = (rand() % MAX_RAND);

            printf("the child process is about to sleep for %d seconds ...\n", my_rand);

            sleep(my_rand);

            printf("the child process wakes up ...\n");

            my_rand = (rand() % MAX_RAND);

            sleep(my_rand);
        }
 
     return(0);  
  }

   // The parent process ------------------------- //
   else
   {
        printf("I am the parent process ...\n");

        for (i = 0; i < NUM_REPEATS; i++)
        {
            my_rand = (rand() % MAX_RAND);

            printf("the parent process is about to sleep for %d seconds ...\n", my_rand);

            sleep(my_rand);

            printf("the parent process wakes up ...\n");

            my_rand = (rand() % MAX_RAND);

            sleep(my_rand);
        }
    }

    return(1);  
}

/* END OF THE LINES ----------------------------- */

APPENDIX-4: The essential system calls for Project #1 (CS314, Spring 2026)

(a) fork system call
    - Structure
    - Variables (which variables are inherited and which variables will not be inherited between the parent and the child process)
    - Risk in calling "fork" in a loop.
(b) Shared memory
    1. Define the contents of a shared memory (as C "struct")
    2. Obtain the size of the shared memory
    3. Create the shared memory ("shmget")
    4. Attach the shared memory ("shmat") - "shmat" returns a pointer
    5. Initialize the contents in the shared memory (using the pointer)
    (using the shared memory)
    6. Detach the shared memory ("shmdt")
    7. Delete the shared memory ("shmctl")

(c) Semaphores
    1. Prepare "semaphore operation array" ("struct sembuf operations[1];" - for manipulating a semaphore)
    2. Prepare "semaphore control data structure" ("union semun" - for initializing a semaphore)
    3. Set the initial value of a semaphore (argument.val = 1;)
    4. Create a semaphore ("semget")
    5. Initialize the new semaphore ("semctl")
    6. Use the semaphore by using "semaphore operation array" ("semop")
        - for "wait" operation
        - for "signal" operation
    7. Delete a semaphore ("semctl")