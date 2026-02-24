/*
 * This is the template of Project #1.
 *
 * It performs:
 *
 * (1) Create a chunk of shared memory
 * (2) Create four child processes (C1 to C4)
 * (3) Let the four new child processes to wait
 * until all the four child processes are
 * created and active.
 * (4) When the main (parent) process sees that all *
 * four child processes are active, then it lets*
 * the four processes to start their work.
 * (5) The main process becomes the fifth worker.
 * (6) Each of the five processes repeats N times.
 * (7) When each child process ends its work, it
 * notifies (signals) the parent process (and
 * terminates).
 * (8) The parent process waits until all the four
 * child processes to leave.
 * (9) The parent process deletes the shared memory 
 * (10) The parent process leaves (the parent is
 * always the last one to leave).
 * (11) Then, it implements two boilermen and three *
 * bathers, as we discussed in the classroom.
 *
 *
 * For CS314-001, Spring 2026 semester
 * for os.cs.siue.edu
 *
 *
 * Jan. 15, 2026
 */

#include <stdio.h>          // for printf
#include <stdlib.h>         // for rand
#include <sys/types.h>
#include <unistd.h>         // for usleep
#include <time.h>           // for time
// the followings are for shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
// the followings are for semaphore
#include <sys/sem.h>

#define NUM_REPEAT                 50       // each boiler-man repeats

#define SEM_KEY_01	               9010	    // the semaphore key (MUTEX)
#define SEM_KEY_02	               9012	    // the semaphore key (SAFETY)
#define SHM_KEY	                   4512	    // the shared memory key

#define MAX_RAND_01                1        // the max. random value for boilermen
#define MAX_RAND_02                4        // the max. random value for bathers

#define BATHER_TIME_01_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_01_B           800000   // 800ms = 0.8 seconds

#define BATHER_TIME_02_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_02_B           800000   // 800ms = 0.8 seconds

#define BATHER_TIME_03_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_03_B           800000   // 800ms = 0.8 seconds

#define BOILERMAN_TIME_01_A        1200000  // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_01_B        1600000  // 1600ms = 1.6 seconds

#define BOILERMAN_TIME_02_A        1200000  // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_02_B        1600000  // 1600ms = 1.6 seconds

#define SAFEGUARD_TIME_A           1200000  // 1200ms = 1.2 seconds
#define SAFEGUARD_TIME_B           1600000  // 1600ms = 1.6 seconds

/* prototypes ------------------------------------------------------- */
int rand(int sleep_time);          // the random slee-time generator

/* module main ====================================================== */
int main (void)
{
    int  i;              // loop counter
    int  sleep_time;     // sleep time in micro-seconds

    /* shuffle the random seed -------------------------------------- */
    srand((unsigned int)time(NULL)); 

    for (i = 0; i < NUM_REPEAT; i++)
    {
       sleep_time = rand (BOILERMAN_TIME_01_A);
       
       printf ("I start sleeping for %d microseconds (%f seconds) ...\n\n", sleep_time, (float)(sleep_time)/1000000.0);
       usleep (sleep_time);
    }

    return (0);
}

/* the random slee-time generator ----------------------------------- */
int rand(int sleep_time)
{
   float temp_rand;       
   float temp_sleeptime;

   temp_rand = (float)((rand() % 100))/100.0;

   temp_sleeptime = ((float) sleep_time) * temp_rand;

   return ((int) temp_sleeptime);
}

/* END OF THE LINES ================================================== */


February 5, 2026
CS314-003 Project #1
Semaphore and Shared Memory Keys

The followings are the list of the semaphore and the shared memory keys assigned to each of you.
Use only the keys assigned to you (not anyone else's keys). (NOTE FROM THE USER: I have just pasted my row specifically below because that is all you need).

3-DIGITS	semaphores	    shared memory
510	        8370-8379	    7205-7209


0. Programming Project #1 Description/Requirements

1. IntroductionProject Due: 12a.m. on February 28th
In this individual programming project, we will develop a solution for "synchronized swimming pool control problem" using UNIX standard semaphores (but not POSIX semaphores) and shared memory. The details of the problem and the project's requirements will be presented using a PPT presentation (Project_01_Required_Algorithm_Structure.ppt).

The "synchronized swimming pool control problem" consists of the following processes: three bather processes (A1, A2, and A3), two boiler-man processes (B1 and B2), and the safeguard process (S).

Bather: Each bather process represents a bather, who repeats the following activities: enjoys bathing by staying the pool-side for some time and once in a while (randomly) enters the swimming pool.

Boiler-man: Each boiler-man process becomes active once in a while to start heating the water in the swimming pool.

Safeguard: The safeguard periodically inspects the condition of the swimming pool after kicking out any bather and boiler-man from the pool (as shown in the figure below).

2. Program requirements

Your final program should satisfy the following requirements (please use the following list for your "check list"):

(a) Only one C source code file should be submitted for implementing "synchronized swimming pool control problem".
(b) Each of the three bathers, two boiler men, and the safeguard should be implemented as a process and concurrently running (multi-tasked).
(c) The first process becomes the safeguard (S) and should create other five processes (B1, B2, A1, A2, and A3 in that order) using fork system call.
(d) Newly created child processes must wait for all processes to be created and start running.
(e) When each process enters the critical section, each process outputs the following message:
    - "S starts inspection", when the safeguard process enters the critical section
    - "B# starts his water heater ...", when a boiler-man process (# = 1 or 2 (boiler-man number)) enters the critical section
    - "A# is entering the swimming pool", when a bather process (# = 1, 2, or 3 (bather number)) enters the critical section
(f) When a bather is leaving the swimming pool, each bather outputs the following message: "A# is leaving the swimming pool"
(g) When a boiler-man finishes water-heating, the boiler-man outputs the following message: "B# finishes water heating"
(h) When the safeguard process finishes his/her inspection, the safeguard process outputs the following message: "S finishes inspection"
(i) Each process waits for a random amount of time each time in the swimming pool and outside of the pool.
(j) Boiler men continue to repeat until each of them finishes repeating NUM_REPEAT times. The three bathers terminate after both boiler men finish.
(k) The safeguard process should repeat until all the other five processes finish. The safeguard process should delete the semaphore(s) and the shared memory after all A1, A2, A3, B1, and B2 finish.
(l) To control the activity timing of the bathers, the boiler-men, and the safeguard processes (the timing parameters will be changed when each submission is tested) the following five labels should be declared at the top of your source code and be used in the critical section as specified below (including the following labels in your C source code is required).
    - NUM_REPEAT: the number of times each boiler-man does his work (heat up the water)
    - BATHER_TIME_01_A: average time interval a bather A1 stays outside of the pool
    - BATHER_TIME_02_A: average time interval a bather A2 stays outside of the pool
    - BATHER_TIME_03_A: average time interval a bather A3 stays outside of the pool
    - BATHER_TIME_01_B: average time interval a bather A1 stays in the pool
    - BATHER_TIME_02_B: average time interval a bather A2 stays in the pool
    - BATHER_TIME_03_B: average time interval a bather A3 stays in the pool
    - BOLIERMAN_TIME_01_A: average time interval a boiler-man B1 waits before next water heat-up
    - BOLIERMAN_TIME_02_A: average time interval a boiler-man B2 waits before next water heat-up
    - BOLIERMAN_TIME_01_B: average time B1 needs to do heat up the water
    - BOLIERMAN_TIME_02_B: average time B2 needs to do heat up the water
    - SAFEGURAD_TIME_A: average time interval the safe guard waits before next inspection
    - SAFEGURAD_TIME_B: average time the safe guard needs to finish his/her inspection

Note: The above timing parameters as "labels" (should be in "micro-second" order) are posted to the CS314 course home ("required_test_parameters.c").

(m) Your C source code is required to use the following "required structures" (Figure 2, 3 and 4):

Figure 2 shows the required structure of the critical section for each bather.

long int sleep_time;
while ( ... )
{
     sleep_time = rand(BATHER_TIME_0x_A);
     usleep(sleep_time);

     [   your semaphore(s) here  ]

     // the critical section starts here -------------------
     printf ("T%d is entering the bathing area ..\n", my_TID);
	 
     sleep_time = rand(BATHER_TIME_0x_B);
     usleep (sleep_time);
     printf ("T% is leaving the bathing area ..\n", my_TID);

    [   your semaphore(s) here  ]

     // the critical section ends here --------------------
}

Note: "required" in "the required structure" means:
    - The loop structure (the type of the loop structures, such as while and for) should not be changed.
    - The printf statements in the structure should not deleted or (logically) skipped.

Figure 3 shows the required structure of the critical section for each boiler man.

long int sleep_time;

for (i = 0; i < NUM_REPEAT; i ++)
{
     sleep_time = rand (BOILERMAN_TIME_0x_A);
     usleep (sleep_time);

     [   your semaphore(s) here  ]

     // the critical section starts here -------------------
     printf ("B%d starts his boiler ...\n", my_BID);

     sleep_time = rand (BOILERMAN_TIME_0x_B);
     usleep(sleep_time);

     printf ("B%d is leaving the bathing area ..\n", my_BID);

     [   your semaphore(s) here  ]

     // the critical section ends here --------------------
}

Figure 4 shows the required structure of the critical section for the safe guard process.

while (one of the three bathers is still active)
{
     sleep_time = rand (SAFEGUARD_TIME_A);
     usleep (sleep_time);

     [   your semaphore(s) here  ]

     // the critical section starts here -------------------
     printf ("S starts inspection ...\n");

     sleep_time = rand (SAFEGUARD_TIME_B);
     usleep(sleep_time);

     printf ("S is leaving the bathing area ..\n");

     [   your semaphore(s) here  ]

     // the critical section ends here --------------------
}

3. Requirements
Your program must follow the following requirements:

(a) No starvation should occur.
(b) No violation of mutual exclusion should occur for boiler men and the safeguard.
(c) No deadlock should occur (all the processes should be always completed)
(d) Multiple bathers should be able to enter the swimming pool (at least several times in a program execution).
(e) All outputs from your program should be made only by the printf specified by Figure 2, 3 and 4.
(f) No busy loop (a.k.a., no "spin wait") should be used.

Figure 5 an example of "spin wait"

while (some_condition != TRUE)
{
}

4. Objectives
This project is designed for the following objectives:
    - To understand the concept of process
    - To understand the concept of inter process communication (IPC)
    - To understand the concept of process synchronization
    - To develop system programming skills to manage race condition and critical section
    - To develop programming skills to avoid process starvation and deadlock
    - To have UNIX system programming experience
    - To be familiar with UNIX-based operating systems

5. Grading Criteria
    - Every submission will be tested only by the CS314 timesharing server (os.cs.siue.edu). The course instructor can not be responsible for any error/trouble for submissions that are not tested by the server.
    - After submission, any update/modification will not be allowed (except for those parameters the course instructor/TA is supposed to change).
    - Any compile-time error (-80%)
    - Minor run-time error: -5% (for each)
    - A program that satisfies (a) no race condition, (b) no violation for mutual condition, and (c) concurrent bathers: 85% of the credit
    - Design and implement "starvation-free" for the two boiler-men: 15% of the credit
    - Major run-time error (failing the meet any of requirement (a) through (d): depends.
    - Program style (program structure and in-line comments): 10%
    - For confirming the authorship for submitted source code files, an interview may be conducted.
    - The exams cover this project (i.e., some questions about this project will be asked in the exam(s)). For seriously low performance (e.g., less than 50%) for the questions about this project in the midterm exam can result in some "discount" to your project grade.

6. Guidelines for acceptable activities
    - You ARE allowed to use A.I. for this programming project, but you are nonetheless responsible for understanding the code and concepts. You will be tested over them.
    - This programming project is an individual programming project.
    - Exchanging ideas with your classmates: Not allowed
    - Exchanging source codes with your classmates: Not allowed
    - Requesting someone else (but yourself) to write any code for you for this assignment: Not allowed.
    - Exams will ask you some questions regarding your code. If you fail to convince Dr. Fujinoki your work, your assignment credit can be cancelled.
    - Using the sample source code files posted to the course (CS314, Spring 2026) home by the course instructor (Fujinoki): Allowed (recommended/required).

7. Required submission
Program softcopy, as "P1_your_3_digits.c" (should be submitted by emails) by the project due (12 a.m. on February 28th).

8. Late Submission
    - Penalty of -10% will be given for every 12 hours after the due (i.e., -10% for a submission within the first 12 hours after the due).
    - Submission more than 48 hours after the due will not be accepted.

9. Suggested progresses

PHASE-1: Test the sample code for UNIX POSIX semaphores and shared memory (1 hour)
    - Download it ("with_semaphore.cpp") from the CS314 home
    - Compile it and run its binary executable
    - Make sure you successfully/correctly create a semaphore and shared-memory by "ipcs" command
    - Identify the essential system calls for semaphore and shared-memory controls by studying "with_semaphore.cpp".
PHASE-2: Test the sample code for fork system call (1 hour)
    - Download it ("fork_test.cpp") from the CS314 home
    - Compile and run its binary executable
    - Understand the basic program structure for "fork" system call
PHASE-3: Create five processes: A1, A2, A3, B1, and B2 (1 to 4 hours)
    - Let each of them sleep as soon as each of them is created
    - Apply "ps -a" to make sure you create exactly five processes
    - Let each of the five processes to terminate
PHASE-4: Using a semaphore, synchronize the five processes (1 to 2 days)
    - While the first process is creating a process at time, let each of the five processes wait until the first process finishes creates all five processes
    - Let all five processes wait for some time (e.g., 3 seconds)
    - Let each of the five processes terminate (their order of termination does not matter)
    - Let the first process waits for all the five processes terminate then let the first process terminate.
PHASE-5: Implement the boiler-man and bathers logics to what you have correctly implemented for (d) (1 to 3 days) using semaphores and shared memory

11. Guideline for "seeking helps"

(a) Dr. Fujinoki will not debug your project source code file. Please do not send (attach) your *.c source code file unless you are asked to do so.
(b) Designing your program logic is a part of this project.
(c) When you ask questions/advices, please describe the following details:
    1. The symptom of the problem(s) (what is wrong and when it happens)
    2. Your analysis of the problem(s) (how it is happening)
    3. What you tried so far
(d) Within 48 hours before the final submission due, no new question will be answered (except for continuing discussions).

APPENDIX-1: the required "labels (constants)"
The five constant labels for defining time interval should be placed at the beginning of your source code file as shown below:

/*
 * **********************************
 * CS314 Project #1 solution
 *
 * Your last-three:
 * Your course section #:
 *
 * Spring 2026
 *
 *
 * You can "copy & paste" this to your source code file
 *
 *
 *
 **********************************
 ****/

#define NUM_REPEAT 50 // each boiler-man repeats

#define BATHER_TIME_01_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_01_B 800000 // 800ms = 0.8 seconds
#define BATHER_TIME_02_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_02_B 800000 // 800ms = 0.8 seconds
#define BATHER_TIME_03_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_03_B 800000 // 800ms = 0.8 seconds

#define BOILERMAN_TIME_01_A 1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_01_B 1600000 // 1600ms = 1.6 seconds
#define BOILERMAN_TIME_02_A 1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_02_B 1600000 // 1600ms = 1.6 seconds

#define SAFEGUARD_TIME_A 1200000 // 1200ms = 1.2 seconds
#define SAFEGUARD_TIME_B 1600000 // 1600ms = 1.6 seconds

#include <........>

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

Additional Test cases for project 1 (dont use this in the source code file but these will be used for testing when the project is graded, so feel free to test with these before hand):
#define NUM_REPEAT                     15       // each boiler-man repeats

#define BATHER_TIME_01_A               500000   //   500,000us = 0.5 seconds
#define BATHER_TIME_01_B               300000   //   300,000us = 0.3 seconds

#define BATHER_TIME_02_A               500000   //   500,000us = 0.5 seconds
#define BATHER_TIME_02_B               300000   //   300,000us = 0.6 seconds

#define BATHER_TIME_03_A              5000000   // 5,000,000us = 5.0 seconds
#define BATHER_TIME_03_B               300000   //   300,000us = 0.3 seconds

#define BOILERMAN_TIME_01_A           1800000   // 1,800,000us = 1.8 seconds
#define BOILERMAN_TIME_01_B            600000   //   600,000us = 0.6 seconds

#define BOILERMAN_TIME_02_A    	      1800000   // 1,800,000us = 1.8 seconds
#define BOILERMAN_TIME_02_B            600000   //   600,000us = 0.6 seconds

#define SAFEGURAD_TIME_A	      1800000   // 1,800,000us = 1.8 seconds
#define SAFEGURAD_TIME_B	       600000   //   600,000us = 0.6 second

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
