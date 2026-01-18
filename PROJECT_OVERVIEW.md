# Operating Systems Project: Extending a Process Scheduler

## 1. Project Goal

The main goal of this project is to extend a basic process scheduler in a Unix-like environment. You will start with a simple scheduler and add more advanced features in two phases:
1.  Support for process priorities and staggered arrival times.
2.  Handling of I/O operations.

## 2. Provided Code

You are given a starting codebase that includes:
-   `scheduler_v0.c`: A simple scheduler that implements `FCFS` (First-Come, First-Served) and `RR` (Round-Robin) policies.
-   It assumes all processes are CPU-bound and arrive at the same time (time 0).
-   `work/`: A directory with source code for dummy workload programs that the scheduler will execute.
-   `Makefile`: To compile the scheduler and workloads.
-   `run.sh`: A script to demonstrate how to run the scheduler.
-   Sample input files (`homogeneous.txt`, `reverse.txt`).

## 3. Project Phases

The project is divided into two main implementation phases.

### Phase 1: Priorities and Different Arrival Times (`scheduler_v1.c`)

In this phase, you will create `scheduler_v1.c` to add support for process priorities and allow processes to arrive at different times.

**Requirements:**
-   **Modified Input:** The input file format will be extended.
    -   Each line can specify a program and its priority (0 for low, 10 for high). Example: `./work/load1 5`
    -   A new command `sleep X` will be introduced to pause the arrival of new processes for `X` seconds, simulating different arrival times.
-   **New Scheduling Policies:** You need to implement two new priority-based scheduling policies:
    1.  `FCFS_PNP` (Priority Non-Preemptive): When a process finishes, the scheduler chooses the waiting process with the highest priority. If priorities are equal, the one that arrived first is chosen. The running process is *not* interrupted.
    2.  `FCFS_PRIO` (Priority Preemptive): If a new process arrives with a higher priority than the currently running process, the running process is immediately paused (`SIGSTOP`), and the new higher-priority process starts.

### Phase 2: I/O Operation Support (`scheduler_v2.c`)

In this phase, you will create `scheduler_v2.c` to handle processes that perform I/O, making them block and unblock.

**Requirements:**
-   **Simulating I/O:** Processes will simulate I/O operations using a provided function `perform_io()`. This function will:
    1.  Notify the scheduler it's starting I/O by sending a `SIGUSR1` signal.
    2.  Sleep for a specified duration.
    3.  Notify the scheduler it has finished I/O by sending a `SIGUSR2` signal.
    4.  Pause itself by sending a `SIGSTOP` signal, waiting for the scheduler to resume it later.
-   **Scheduler Responsibilities:**
    -   The scheduler must catch `SIGUSR1` and `SIGUSR2` to manage process states.
    -   When a process blocks for I/O (`SIGUSR1`), the scheduler must run another process from the ready queue.
    -   When the process finishes I/O (`SIGUSR2`), it should be moved back to the ready queue, becoming eligible for execution again.
    -   You will likely need to add a new process state, like `WAITING` or `BLOCKED`.
-   **Scope:** This functionality is required for the `FCFS` policy. Implementing it for `RR` as well will earn you bonus points.
-   **Note:** This phase is independent of Phase 1. You can base your `scheduler_v2.c` on the original `scheduler_v0.c`.

## 4. Deliverables

1.  **Source Code:**
    -   `scheduler_v1.c` implementing the Phase 1 requirements.
    -   `scheduler_v2.c` implementing the Phase 2 requirements.
2.  **Packaging:** A single `.zip` file containing the entire project directory, including your modified code. **Do not include executable files.**
3.  **Report:** A PDF report that includes:
    -   A description of your design and implementation for both phases.
    -   Diagrams for process state transitions.
    -   Screenshots of your programs running to prove they work correctly.
    -   A discussion of any problems you faced and how you solved them.
