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
