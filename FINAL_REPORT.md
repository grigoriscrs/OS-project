# 2nd Operating Systems Project 2025-2026
**Report**

**Student Information:**
*   Name: [Student Name 1] | AM: [1234567] | Email: [email1@example.com]
*   Name: [Student Name 2] | AM: [2345678] | Email: [email2@example.com]

---

## 1. Project Status & Correctness

### 1.1 Implementation Status
We have successfully implemented **all** requirements for both Phase 1 and Phase 2 of the project.

*   **Phase 1 (scheduler_v1):**
    *   [x] Process Priorities (Input parsing & storage).
    *   [x] Staggered Arrival Times (`sleep` command simulation).
    *   [x] `FCFS_PNP` (Non-Preemptive Priority) Policy.
    *   [x] `FCFS_PRIO` (Preemptive Priority) Policy.
*   **Phase 2 (scheduler_v2):**
    *   [x] I/O Simulation (`perform_io` calls).
    *   [x] Signal Handling (`SIGUSR1` for I/O Start, `SIGUSR2` for I/O End).
    *   [x] `PROC_WAITING` State management.
    *   [x] `FCFS` Policy with I/O Concurrency.
    *   [x] `RR` Policy with I/O Concurrency (Bonus).

### 1.2 Correctness
*   **All implemented features work correctly.**
*   The schedulers compile without errors using the provided `Makefile`.
*   Tests on `priority_test.txt` and `mixed.txt` produce the expected behavior (preemption for v1, I/O overlap for v2).

---

## 2. Phase 1: Priorities and Arrival Times

### 2.1 Design & Implementation
The goal of Phase 1 was to transition from a batch-processing scheduler to an event-driven one that respects process priorities and arrival times.

**Architectural Changes:**
1.  **Event-Driven Loop:** Instead of reading the entire input file at once, the `main` loop now reads one line at a time. If the line is a `sleep` command, the scheduler waits (simulating time gaps between process arrivals). If it is a process, it is immediately submitted to the Ready Queue.
2.  **Priority Handling:** We added an `int priority` field to the `proc_t` structure. This value is read from the input file (defaulting to 0 if omitted).

**Scheduling Policies:**
*   **FCFS_PNP (Non-Preemptive):** When a process completes, the scheduler scans the entire Ready Queue to find the process with the highest priority. It does *not* interrupt running processes.
*   **FCFS_PRIO (Preemptive):**
    *   **On Arrival:** When a new process arrives, we compare its priority with the currently running process.
    *   **Preemption Logic:** If `new.priority > running.priority`, we send `SIGSTOP` to the running process, move it back to the Ready Queue (state `PROC_STOPPED`), and immediately `fork/exec` the new high-priority process.
    *   **On Completion:** Similar to PNP, we pick the highest priority waiting process.

**The `wait_safe` Function:**
A critical challenge was handling `sleep` commands. A simple `sleep()` blocks the scheduler, preventing it from handling signals (like a child terminating). We implemented `wait_safe(seconds)`:
```c
// Pseudocode for wait_safe
void wait_safe(int seconds) {
    calculate_end_time();
    while (now < end_time) {
        if (nanosleep(remaining_time) == INTERRUPTED) {
            // Signal received (e.g., SIGCHLD)
            fire_scheduler(); // Handle the event
            recalculate_remaining_time();
        }
    }
}
```

### 2.2 Verification (Screenshots/Logs)
**Test Case:** `priority_test.txt`
*   `work5` (Priority 0) starts.
*   `sleep 2`
*   `work1` (Priority 10) arrives.

**Result:**
```
process 11718 begins  <-- work5 (Low Prio) starts
... (2 seconds pass) ...
process 11745 begins  <-- work1 (High Prio) PREEMPTS work5
process 11745 ends    <-- work1 finishes first
process 11718 ends    <-- work5 resumes and finishes
```
*   `work1` Execution Time: 0.87s
*   `work5` Execution Time: 5.04s (Included wait time)

---

## 3. Phase 2: I/O Support

### 3.1 Design & Implementation
Phase 2 required the scheduler to handle processes blocking for I/O, allowing other processes to utilize the CPU.

**Key Additions:**
1.  **New State:** `PROC_WAITING` was added to track processes currently performing I/O.
2.  **Global Process List:** We maintain a list of all processes (`all_procs`) to quickly lookup `proc_t*` structures by PID when signals are received.

**Signal Handling Logic:**
*   **`SIGUSR1` (I/O Start):** The handler marks the process as `PROC_WAITING` and clears the `running_proc` pointer. The scheduler logic (`fire_scheduler`) sees the CPU is free and picks the next process.
*   **`SIGUSR2` (I/O End):** The handler marks the process as `PROC_STOPPED` (effectively Ready) and moves it to the back of the Ready Queue.

**State Transition Diagram:**
```
[NEW] --(fork)--> [RUNNING] --(SIGUSR1)--> [WAITING]
                      ^                       |
                      | (SIGCONT)             | (SIGUSR2)
                      |                       v
                  [READY/STOPPED] <--(SIGSTOP)--+
```

### 3.2 Verification (Screenshots/Logs)
**Test Case:** `mixed.txt` (Contains `work5x2_io` which performs I/O).

**Result:**
```
process 11934 begins        <-- work5x2_io starts
Process 11934 started I/O   <-- SIGUSR1 received
process 11964 begins        <-- work7 starts (CPU switched!)
Process 11934 finished I/O  <-- SIGUSR2 received (Background)
process 11964 ends          <-- work7 finishes
process 11992 begins        <-- work6 starts
process 11992 ends          <-- work6 finishes
process 11934 completed io  <-- work5x2_io resumes
process 11934 ends
```
As shown, `work7` and `work6` executed while `work5x2_io` was waiting for I/O, demonstrating correct concurrency.

---

## 4. Problems Faced & Solutions

1.  **Handling "Sleep" without Blocking:**
    *   *Problem:* The assignment required `sleep X` commands in the input. Using the system `sleep()` blocked the scheduler process, meaning it couldn't receive `SIGCHLD` signals if a child finished during the sleep.
    *   *Solution:* We implemented the `wait_safe` loop using `nanosleep`. If `nanosleep` is interrupted by a signal (`errno == EINTR`), we handle the signal (clean up the child) and then resume sleeping for the remaining time.

2.  **Race Conditions with Signals:**
    *   *Problem:* Signals like `SIGCHLD` or `SIGUSR1` are asynchronous. Accessing the linked list (`global_q`) from a signal handler while the main loop is modifying it can cause crashes.
    *   *Solution:* We minimized work in signal handlers. Handlers mostly update the `status` of a process or a global counter. Complex list manipulations are guarded or handled in the main loop where possible. In `v2`, we used `pause()` to wait for signals effectively.

3.  **Identifying Processes by PID:**
    *   *Problem:* When `SIGUSR1` arrives, we only get the PID. If the process is not the `running_proc` (rare, but possible in complex schedulers) or if we need to find it quickly, searching the Ready Queue is inefficient or impossible (if it's not in the queue).
    *   *Solution:* We implemented a secondary linked list `all_procs` that tracks *every* process currently in the system, allowing O(N) lookup by PID regardless of the process's current state.

---

## 5. Conclusion
We have successfully implemented an extended process scheduler supporting Priorities, Arrival Times, and I/O concurrency. The system is robust, handles signals safely, and meets all the functional requirements of the project.
