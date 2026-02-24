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
