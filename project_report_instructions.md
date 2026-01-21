# Project Report Instructions & Plan

Based on the university project requirements (`Project2-OS-2025-2026.pdf`) and the implementation details from our work (`scheduler_v1_report.md`, `scheduler_v2_report.md`), this document outlines the structure and content for the final project report.

## 1. Report Requirements (from PDF)

*   **Format:** PDF.
*   **Language:** Greek (implied by the PDF language) or English (often accepted, but we will stick to the language of the prompt or English as requested. The prompt is in English, but the PDF is in Greek. I will prepare the structure to support either, but English is the primary CLI language). *Note: The user's prompt is in English, so I will draft the report structure in English.*
*   **Cover Page:**
    *   Student Names and AM (Student IDs).
    *   Emails.
    *   Project Title ("2nd Operating Systems Project 2025-2026").
*   **Mandatory Sections (Start of Documentation):**
    *   **Implementation Status:** Explicitly state what parts were implemented and what parts are missing.
    *   **Correctness:** Explicitly state what works correctly and what doesn't.
*   **Core Content:**
    *   **Design Description:** Brief description of the design/implementation for each phase (v1 and v2). Use diagrams or pseudocode where necessary.
    *   **Problems & Solutions:** Briefly describe problems faced and the approaches taken to solve them. If multiple approaches were considered, explain the choice.
    *   **Verification:** Screenshots of program execution proving the functionality.
*   **Constraints:**
    *   **NO** full assignment text included.
    *   **NO** full source code included (describe logic/algorithms verbally).
    *   **References:** Cite sources if any code/ideas were borrowed (Standard Academic Integrity).

## 2. Content Source Map

We have generated most of the content in our intermediate reports. Here is how they map to the final report sections:

### Phase 1: Priorities & Arrival Times (`scheduler_v1`)
*   **Source:** `scheduler/scheduler_v1_report.md`
*   **Key Features:**
    *   Modified `proc_desc` struct (added priority).
    *   Event-driven main loop (interleaving `fgets` reading and execution).
    *   `wait_safe()` function for handling `sleep` commands without blocking signals.
    *   **Policies:**
        *   `FCFS_PNP` (Non-Preemptive): Scan queue for max priority on completion.
        *   `FCFS_PRIO` (Preemptive): Check incoming process priority vs running process; `SIGSTOP`/`SIGCONT`.
*   **Verification:**
    *   Scenario: Low priority running, High priority arriving (Preemption test).
    *   Scenario: Simultaneous arrival with different priorities.

### Phase 2: I/O Support (`scheduler_v2`)
*   **Source:** `scheduler/scheduler_v2_report.md`
*   **Key Features:**
    *   New State: `PROC_WAITING` (or `BLOCKED`).
    *   Signal Handlers: `SIGUSR1` (I/O Start), `SIGUSR2` (I/O End).
    *   Global Process List (`all_procs`): To find processes by PID even if they are not in the Ready Queue.
    *   **Policies:**
        *   `FCFS`: Modified to be event-driven (`pause()` loop). Switches context when `SIGUSR1` is received.
        *   `RR`: (Bonus) Handles I/O interruptions within the quantum sleep.
*   **Verification:**
    *   Scenario: `mixed.txt` (CPU-bound + I/O-bound processes).
    *   Evidence: Timestamps showing overlap of execution (Process A does I/O while Process B runs).

## 3. Execution Plan for Report Creation

1.  **Drafting the Content (Markdown Phase):**
    *   Create a single file `FINAL_REPORT.md`.
    *   **Section 1: Cover Info & Status:** Add the implementation checklist.
    *   **Section 2: Phase 1 Design:** Synthesize text from `scheduler_v1_report.md`. Add pseudocode for `wait_safe` and the Preemptive Scheduler logic.
    *   **Section 3: Phase 2 Design:** Synthesize text from `scheduler_v2_report.md`. Add a state transition diagram description (New -> Ready -> Running -> Waiting -> Ready...).
    *   **Section 4: Challenges:** Summarize the "Event-driven vs Batch" shift and the "Signal Safety" issues.
    *   **Section 5: Experimental Results:** We need to capture the output of our tests now to include them as text blocks (or simulate screenshots).

2.  **Generating Screenshots/Logs:**
    *   Run `scheduler_v1` with `FCFS_PRIO` and a custom input to show preemption.
    *   Run `scheduler_v2` with `FCFS` and `mixed.txt` to show I/O overlap.
    *   Save these outputs to include in the verification section.

3.  **Final Review:**
    *   Check against the "Mandatory Sections" list.

## 4. Required Action Items

*   [x] **Run Verification Tests:** Executed `scheduler_v1` (Preemption) and `scheduler_v2` (I/O) and captured logs.
*   [x] **Create `FINAL_REPORT.md`:** A complete draft has been generated in the project root.
*   [ ] **Convert to PDF:** Open `FINAL_REPORT.md` in a Markdown editor (like VS Code, Obsidian, or an online converter) and export/print to PDF.
*   [ ] **Add Student Info:** Replace the `[Student Name]` placeholders in the final PDF.

## 5. Verification Scenarios Run

The following tests were run to generate the logs in `FINAL_REPORT.md`:
*   **Test A (Phase 1):** `scheduler_v1 FCFS_PRIO priority_test.txt` -> Proven preemption.
*   **Test B (Phase 2):** `scheduler_v2 FCFS mixed.txt` -> Proven I/O concurrency.
