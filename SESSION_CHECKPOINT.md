# Session Checkpoint: Phase 1 Complete

## Status
**Date:** January 18, 2026
**Current Phase:** Phase 1 (Priorities & Arrival Times) - **COMPLETE**
**Next Phase:** Phase 2 (I/O Support)

## Accomplishments
1.  **Implemented `scheduler_v1.c`**:
    *   Derived from `scheduler_v0.c`.
    *   Added `priority` field to `proc_t` struct.
    *   **Input Parsing**: Switched to `fgets`/`sscanf` to handle optional priority arguments (default 0) and `sleep` commands.
    *   **Architecture**: Refactored `main` loop to be event-driven (interleaving parsing and execution).
    *   **Concurrency**: Implemented `wait_safe()` to handle `SIGCHLD` interruptions during `sleep` commands, ensuring the scheduler wakes up to handle process termination.
    *   **Policies Implemented**:
        *   `FCFS_PNP` (Non-Preemptive): Runs highest priority to completion.
        *   `FCFS_PRIO` (Preemptive): Preempts running process if a higher priority one arrives (using `SIGSTOP`/`SIGCONT`).

2.  **Build System**:
    *   Updated `scheduler/Makefile` to build `scheduler_v1`.

3.  **Documentation**:
    *   Created `scheduler/scheduler_v1_report.md` detailing the changes from v0 to v1, architectural decisions, and logic explanations.
    *   Added placeholders for Student Name/AM in both the C file header and the report.

4.  **Verification**:
    *   Verified `FCFS_PNP` (High prio waits).
    *   Verified `FCFS_PRIO` (High prio preempts).
    *   Verified default priority handling (No arg -> priority 0).

## File State
*   `scheduler/scheduler_v1.c`: **Finalized for Phase 1.**
*   `scheduler/scheduler_v1_report.md`: **Finalized.**
*   `scheduler/Makefile`: **Finalized.**
*   `scheduler/priority.txt` (and other test files): **Deleted** (Clean state).

## Next Steps (Phase 2)
1.  Read `PROJECT_OVERVIEW.md` and `scheduler-info.md` to refresh on Phase 2 requirements (I/O Support).
2.  Copy `scheduler_v0.c` (or `v1` if you want to combine features, though the PDF implies v2 can be independent/based on v0) to `scheduler_v2.c`.
    *   *Note:* The PDF says "You can base your scheduler_v2.c on the original scheduler_v0.c" but also "scheduler_v2.c implementing the Phase 2 requirements". Usually, v2 is independent of v1 features unless specified. **Important:** Check PDF Page 6: "The version v2 of the scheduler is completely independent from version v1... The usage remains as in the initial version (v0)". This implies v2 does **not** need priorities or sleep commands, just I/O support.
3.  Implement `SIGUSR1` (I/O Start/Blocking) and `SIGUSR2` (I/O End/Unblocking) handling.
4.  Implement `WAITING` state.
