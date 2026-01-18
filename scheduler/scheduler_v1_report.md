# Scheduler V1 Implementation Report

**Student Info:**
*   Name 1, AM 1234567
*   Name 2, AM 2345678

## 1. Project Goal: Phase 1
The objective of Phase 1 was to evolve the basic `scheduler_v0` (which handled simple FCFS/RR for simultaneous arrivals) into `scheduler_v1`, a scheduler capable of handling:
1.  **Process Priorities**: Some processes are more important (higher priority value) and should run first.
2.  **Arrival Times**: Processes do not all arrive at once; they arrive sequentially, simulated by `sleep` commands in the input.
3.  **Two New Policies**:
    *   `FCFS_PNP` (Priority Non-Preemptive).
    *   `FCFS_PRIO` (Priority Preemptive).

---

## 2. Evolution: Changes from `scheduler_v0` to `scheduler_v1`

To achieve the goals above, we made specific "additions" and "modifications" to the base `v0` code.

### 2.1 Data Structure Additions
*   **Struct `proc_desc`**: Added `int priority`.
    *   *Why?* To store the priority read from the input file so the scheduler can make decisions based on it.

### 2.2 Architectural Shift: From "Batch" to "Event-Driven"
*   **v0 Approach**: Read *all* lines from the file into a list -> Start the loop -> Run them.
*   **v1 Approach (New)**: The `main` loop now interleaves **reading input** and **running the scheduler**.
    *   It reads a line.
    *   If it's a process: It "arrives" immediately. We add it to the queue and check if it should run.
    *   If it's a `sleep`: The scheduler waits, effectively simulating time passing before the *next* process arrives.

### 2.3 Input Parsing Logic
*   **Changed `fscanf` to `fgets` + `sscanf`**:
    *   *Why?* The priority argument is optional (e.g., `./prog` vs `./prog 5`). `fscanf` struggles with optional trailing numbers on a line. `fgets` reads the whole line, and `sscanf` lets us count how many arguments were provided. If only 1, we default priority to 0.

---

## 3. Deep Dive: How the Logic Works

### 3.1 The `wait_safe` Function (Crucial Helper)
One of the hardest parts of this phase is handling the `sleep X` command. We cannot just call `sleep(X)` because the scheduler must remain responsive.
*   **Problem**: If we `sleep(5)` and a child process finishes at second 2, we miss the `SIGCHLD` signal (or handle it late) and leave the CPU idle for 3 seconds unnecessarily.
*   **Solution (`wait_safe`)**:
    *   We use `nanosleep` in a loop.
    *   If a signal (like `SIGCHLD` from a finishing child) interrupts the sleep, `nanosleep` returns early with `EINTR`.
    *   We catch this, run the scheduler (`fire_scheduler`) to clean up the zombie and potentially start a new process, and then go back to sleep for the *remaining* time.

### 3.2 Policy Implementation

#### FCFS_PNP (Priority Non-Preemptive)
*   **Logic**: "If I am running, I keep running. If I finish, pick the highest priority person waiting."
*   **Implementation**:
    *   **Arrival**: Just add the new process to the queue. Do *not* disturb the running process.
    *   **Completion**: When the running process exits (`SIGCHLD`), scan the *entire* ready queue. The process with the highest `priority` is selected. If there's a tie, the one closer to the head of the list (arrived earlier) wins.

#### FCFS_PRIO (Priority Preemptive)
*   **Logic**: "If I am running, but someone more important shows up, I must pause immediately."
*   **Implementation**:
    *   **Arrival**: When a new process is read from input:
        1.  Compare `new_process.priority` vs `running_proc.priority`.
        2.  If `new > running`, call `stop_proc(running_proc)`.
        3.  `stop_proc` sends `SIGSTOP` to the running process and moves it back to the Ready Queue.
        4.  The CPU is now free, so we call `fire_scheduler` to pick the new "best" process (which is likely the one that just arrived).
    *   **Completion**: Same as PNP. Pick the highest priority waiting process.

---

## 4. Key Functions Explained

*   **`proc_to_rq(proc)`**: Adds a process to the *end* of the linked list (Ready Queue).
*   **`pick_next_proc()`**: Scans the list to find the process with the max `priority`.
*   **`start_proc(proc)`**: Calls `fork()` and `exec()`. Only used for `PROC_NEW` processes.
*   **`stop_proc(proc)`**: Sends `SIGSTOP`. Changes state to `PROC_STOPPED`. Used for preemption.
*   **`resume_proc(proc)`**: Sends `SIGCONT`. Changes state to `PROC_RUNNING`. Used when a preempted process is chosen to run again.
*   **`fire_scheduler()`**: The "brain". It checks if the CPU is free. If yes, it picks the next process and starts/resumes it.

---

## 5. Verification
We verified the implementation with two scenarios:

1.  **PNP Test**: Low priority runs, high priority arrives.
    *   *Result*: High priority waited until low priority finished.
2.  **PRIO Test**: Low priority runs, high priority arrives.
    *   *Result*: Low priority was paused (preempted), high priority ran to completion, then low priority resumed.