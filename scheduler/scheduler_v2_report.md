# Scheduler V2 Implementation Report

**Student Info:**
*   Name 1, AM 1234567
*   Name 2, AM 2345678

## 1. Project Goal: Phase 2
The objective of Phase 2 was to extend the basic `scheduler_v0` to support I/O operations. In this phase, processes can block for I/O, allowing the scheduler to run other processes in the meantime.
1.  **I/O Simulation**: Processes signal the start and end of I/O using `SIGUSR1` and `SIGUSR2`.
2.  **State Management**: Addition of a `PROC_WAITING` (or `BLOCKED`) state.
3.  **Concurrency**: The scheduler must be able to switch to another process when the current one is waiting for I/O.

---

## 2. Evolution: Changes from `scheduler_v0` to `scheduler_v2`

### 2.1 Data Structure Additions
*   **`PROC_WAITING` State**: A new state constant to track processes that are currently performing I/O.
*   **All-Processes List**: Added an `all_next` pointer to `proc_t` and a global `all_procs` list.
    *   *Why?* The scheduler needs to find processes by PID when receiving `SIGUSR2` or `SIGCHLD`, even if they are not in the ready queue.

### 2.2 Architectural Shift: Event-Driven Scheduling
*   **v0 Approach**: `FCFS` was strictly serial, using blocking `waitpid()`.
*   **v2 Approach**: `FCFS` is now event-driven. It uses `pause()` to wait for signals (`SIGCHLD`, `SIGUSR1`, `SIGUSR2`) and a `fire_scheduler()` function to make decisions asynchronously.

---

## 3. Deep Dive: How the I/O Logic Works

### 3.1 Handling I/O Start (`SIGUSR1`)
When a process initiates I/O (via `perform_io()`):
1.  It sends `SIGUSR1` to the scheduler.
2.  The `sigusr1_handler` catches the signal, identifies the process, and changes its status to `PROC_WAITING`.
3.  The `running_proc` pointer is set to `NULL`, signifying the CPU is now free.
4.  The scheduler (in its main loop or after `pause()`) calls `fire_scheduler()`, which picks the next process from the ready queue and starts it.
5.  **Crucially**, the scheduler does *not* send `SIGSTOP` to the I/O process. This allows the process to continue its simulated I/O (e.g., `usleep()`).

### 3.2 Handling I/O End (`SIGUSR2`)
When a process finishes its I/O:
1.  It sends `SIGUSR2` to the scheduler.
2.  The `sigusr2_handler` catches the signal and changes the process status from `PROC_WAITING` to `PROC_STOPPED` (Ready).
3.  The process is added back to the end of the `global_q` (Ready Queue).
4.  The process then stops itself (`raise(SIGSTOP)`), waiting to be resumed by the scheduler later.

### 3.3 Process Termination (`SIGCHLD`)
When any process exits:
1.  The `sigchld_handler` updates its status to `PROC_EXITED`.
2.  The `active_procs` counter is decremented.
3.  If the exiting process was the `running_proc`, the pointer is cleared.
4.  The main loop continues, and `fire_scheduler()` is called to fill the empty CPU slot.

---

## 4. Key Functions

*   **`fire_scheduler()`**: Checks if the CPU is free. If so, it dequeues a process from the ready queue. If the process is `NEW`, it forks/execs. If it is `STOPPED`, it sends `SIGCONT`.
*   **`fcfs()`**: A simple loop that calls `fire_scheduler()` and then `pause()` to wait for events, continuing as long as there are active processes.
*   **`rr()`**: Similar to `fcfs()`, but uses `nanosleep()` to implement the time quantum. If a process exceeds its quantum, it is `SIGSTOP`ed and moved to the back of the queue.

---

## 5. Verification
We verified the implementation using the `mixed.txt` workload, which contains both CPU-bound and I/O-bound processes.

**Observation**:
*   `work5x2_io` started and then triggered I/O.
*   The scheduler immediately switched to `work7`.
*   While `work7` was running, `work5x2_io` signaled it finished I/O and was added to the queue.
*   After `work7` and `work6` finished, the scheduler resumed `work5x2_io`, which then completed its execution.
*   This proves that the scheduler successfully overlapped I/O with computation from other processes.
