# OS Scheduler Project: Step-by-Step Development Plan

This plan will guide you through the project, from understanding the base code to implementing the final features.

## Phase 0: Setup and Code Analysis (Foundation)

Before writing any new code, get familiar with the existing project.

1.  **Explore the Existing Code:**
    -   **Read `scheduler/scheduler_v0.c`:** This is the most critical step. Understand:
        -   The `struct process` in `scheduler.h` (you'll need to create this header or find the struct).
        -   The linked list used as a ready queue.
        -   How `fork()` and `execvp()` are used to create new processes.
        -   The logic for `FCFS` and `RR` policies.
        -   How `SIGCHLD` is handled when a process terminates.
        -   How the timer (`setitimer` or `nanosleep`) and `SIGALRM` are used for the Round-Robin quantum.
2.  **Compile and Run:**
    -   Open a terminal in the `scheduler` directory.
    -   Run `make` to compile `scheduler_v0.c`.
    -   Execute `./run.sh` to see how the scheduler works.
    -   Compare the output with the `sample_output.txt` and trace the code's logic to understand the flow.
3.  **Understand the Workloads:**
    -   Go to the `work` directory and run `make`.
    -   Examine the simple C files (`work.c`, `work_io.c`) to see what the "work" processes do.

**Important Note:** The `fcfs()` function in `scheduler_v0.c` is a simple serial loop (`fork` -> `waitpid`). It runs one process to completion before starting the next. The `rr()` function, however, manages multiple running/stopped processes at once. **Your new schedulers for both Phase 1 and Phase 2 must be concurrent like `rr()`**, not serial like `fcfs()`. You will need to handle arriving processes while others are already in the system. Therefore, use the structure of the `rr()` function as a guide for your new implementations.

## Phase 1: Priorities and Arrival Times (`scheduler_v1.c`)

1.  **Setup:**
    -   In the `scheduler` directory, copy `scheduler_v0.c` to `scheduler_v1.c`.
    -   Update the `Makefile` to add a rule to compile `scheduler_v1.c` into a `scheduler_v1` executable.
2.  **Modify Data Structures and Input Parsing:**
    -   Add `int priority;` and `time_t arrival_time;` to your process structure definition.
    -   Modify the main loop that reads the input file. This loop will now be responsible for simulating arrival times.
        -   When you read a line, check if it's a `sleep` command. If so, call `sleep()`.
        -   If it's a process, parse the name and priority. Set its `arrival_time` to the current time.
3.  **Implement `FCFS_PNP` (Non-Preemptive Priority):**
    -   The main change is in the scheduling logic that runs when a process finishes (in the `SIGCHLD` handler).
    -   Instead of just taking the head of the queue, you must search the *entire* ready queue.
    -   Find the process with the highest `priority`.
    -   If there's a tie in priority, choose the one with the earliest `arrival_time`.
4.  **Implement `FCFS_PRIO` (Preemptive Priority):**
    -   This is the most complex part. Preemption must occur the moment a higher-priority process arrives.
    -   After you read and add a new process to the queue, you must:
        -   Check if another process is currently `RUNNING`.
        -   If `new_process.priority > running_process.priority`:
            -   Send `SIGSTOP` to the running process.
            -   Change its state to `READY` (or `STOPPED`).
            -   Start the new, higher-priority process.
    -   The logic when a process terminates is the same as for `FCFS_PNP`: pick the highest-priority waiting process.
5.  **Testing:**
    -   Create a `priority.txt` file with different priorities and `sleep` commands to test your logic.
    -   Test preemption: have a low-priority process run for a while, then have a high-priority one arrive.
    -   Modify `run.sh` or create a `run_v1.sh` to execute your new scheduler with the new policies.

## Phase 2: I/O Support (`scheduler_v2.c`)

1.  **Setup:**
    -   Copy `scheduler_v0.c` to `scheduler_v2.c`. (This phase is independent of v1).
    -   Update the `Makefile` to compile `scheduler_v2.c` into a `scheduler_v2` executable.
2.  **Handle I/O Signals:**
    -   Add a new state to your process enum, e.g., `WAITING`.
    -   Implement signal handlers for `SIGUSR1` and `SIGUSR2`.
        -   **`SIGUSR1` Handler (I/O Start):**
            -   Find which process sent the signal (it should be the currently `RUNNING` one).
            -   Change its state to `WAITING`.
            -   This process will stop itself, so the scheduler's main job is to immediately call the scheduler function to pick and run a new process from the ready queue.
        -   **`SIGUSR2` Handler (I/O End):**
            -   A process has finished its I/O. You need to identify it (the signal handler might receive the PID).
            -   Change its state from `WAITING` back to `READY`.
            -   Add it to the end of the ready queue. It is now eligible to be run again.
3.  **Update Scheduler Logic (FCFS):**
    -   The scheduler must now be invoked in two cases:
        1.  A process terminates (`SIGCHLD`).
        2.  A process blocks for I/O (`SIGUSR1`).
    -   In both cases, it should pick the next process from the head of the ready queue and run it with `SIGCONT`.
4.  **Testing:**
    -   Use the provided `work_io.c` and `mixed.txt` to test.
    -   Compile `work_io.c` by running `make` in the `work` directory.
    -   Run `$ ./scheduler_v2 FCFS mixed.txt` and trace the output to ensure processes correctly block for I/O and other processes run in their place.
    -   (Bonus) If you have time, add this logic to the `RR` policy.

## Final Phase: Documentation and Submission

1.  **Write the Report:**
    -   Document your design for `v1` and `v2`.
    -   Create state transition diagrams (e.g., NEW -> READY -> RUNNING -> WAITING -> READY -> ... -> EXITED).
    -   Include screenshots of your tests as proof.
2.  **Package for Submission:**
    -   Run `make clean` in all directories to remove executables and object files.
    -   Create a `.zip` archive of the entire `OS-project` folder.
    -   Submit the `zip` file and your PDF report.
