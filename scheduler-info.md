# Scheduler v0 Analysis

`scheduler/scheduler_v0.c` serves as the baseline for the OS project, implementing a basic process scheduler with two primary scheduling policies.

## 1. Data Structures and State Management

### Process Descriptor (`proc_t`)
The core of the scheduler is the `proc_t` structure, which tracks the lifecycle of each process:
- **Metadata**: Executable name and Process ID (`pid`).
- **State**: Tracked via `status` using four constants:
    - `PROC_NEW`: Initial state after reading from the input file.
    - `PROC_RUNNING`: Currently executing on the CPU.
    - `PROC_STOPPED`: Paused by the scheduler (Round Robin only).
    - `PROC_EXITED`: Finished execution.
- **Metrics**: Timestamps for submission, start, and end times to calculate elapsed and execution time.

### Ready Queue
- **Structure**: A simple linked list (`single_queue`) with a head (`first`) and tail (`last`) pointer.
- **Operations**:
    - `proc_to_rq_end`: Adds a process to the back (Standard FCFS/RR behavior).
    - `proc_rq_dequeue`: Removes the process from the front.

---

## 2. Execution Flow

### Initialization
1. Parses command-line arguments to determine the policy (`FCFS` or `RR`) and `quantum` size.
2. Reads the input text file line-by-line.
3. Each line (program path) is wrapped in a `proc_t` and added to the `global_q`.

### First-Come, First-Served (FCFS)
- **Non-Preemptive**: Once a process starts, it runs until completion.
- **Mechanism**:
    - Dequeues a process.
    - `fork()`s and `execl()`s the workload.
    - Uses `waitpid()` to block the scheduler until the child process terminates.

### Round-Robin (RR)
- **Preemptive**: Processes are limited by a time `quantum`.
- **Signal Handling**: Uses `SIGCHLD` and a dedicated handler (`sigchld_handler`) to catch process exits asynchronously since the scheduler doesn't block on `waitpid`.
- **Mechanism**:
    - Dequeues a process.
    - If `PROC_NEW`: `fork()` and `execl()`.
    - If `PROC_STOPPED`: Sends `SIGCONT` to resume.
    - **Quantum Control**: Calls `nanosleep(quantum)`.
    - **Context Switch**: After the sleep, if the process is still running, the scheduler sends `SIGSTOP` and moves the process to the end of the queue.

---

## 3. Current Limitations (Addressed in Future Phases)
- **Arrival Times**: All processes are currently assumed to arrive at time 0.
- **Priorities**: There is no mechanism to prioritize one process over another.
- **I/O Blocking**: The scheduler assumes all workloads are 100% CPU-bound. If a process blocks for I/O, the scheduler currently remains idle or waits for the quantum to expire.
