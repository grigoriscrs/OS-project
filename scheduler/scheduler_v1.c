/* Placeholder Name 1, AM 1234567 */
/* Placeholder Name 2, AM 2345678 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX_LINE_LENGTH 100

#define PROC_NEW 0
#define PROC_STOPPED 1
#define PROC_RUNNING 2
#define PROC_EXITED 3

#define FCFS_PNP 2
#define FCFS_PRIO 3

typedef struct proc_desc {
	struct proc_desc *next;
	char name[80];
	int pid;
	int status;
    int priority;
	double t_submission, t_start, t_end;
} proc_t;

struct single_queue {
	proc_t	*first;
	proc_t	*last;
	long members;
};

struct single_queue global_q;
proc_t *running_proc = NULL;
int policy = FCFS_PNP;
double global_t;

// --- Helper Functions ---

double proc_gettime() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) (tv.tv_sec + tv.tv_usec / 1000000.0);
}

void err_exit(char *msg) {
	printf("Error: %s\n", msg);
	exit(1);
}

void proc_queue_init(struct single_queue *q) {
	q->first = q->last = NULL;
	q->members = 0;
}

void proc_to_rq(proc_t *proc) {
	if (global_q.first == NULL) {
		global_q.first = global_q.last = proc;
	} else {
		global_q.last->next = proc;
		global_q.last = proc;
	}
    proc->next = NULL;
    global_q.members++;
}

void proc_remove_from_rq(proc_t *proc) {
    if (global_q.first == NULL) return;

    if (global_q.first == proc) {
        global_q.first = proc->next;
        if (global_q.first == NULL) global_q.last = NULL;
    } else {
        proc_t *curr = global_q.first;
        while (curr->next != NULL && curr->next != proc) {
            curr = curr->next;
        }
        if (curr->next == proc) {
            curr->next = proc->next;
            if (curr->next == NULL) global_q.last = curr;
        }
    }
    proc->next = NULL;
    global_q.members--;
}

proc_t *pick_next_proc() {
    if (global_q.first == NULL) return NULL;

    proc_t *best = global_q.first;
    proc_t *curr = global_q.first->next;

    while (curr != NULL) {
        if (curr->priority > best->priority) {
            best = curr;
        }
        curr = curr->next;
    }
    return best;
}

void sigchld_handler(int signo, siginfo_t *info, void *context) {
	if (running_proc && running_proc->pid == info->si_pid) {
        running_proc->status = PROC_EXITED;
		running_proc->t_end = proc_gettime();
        
		printf("PID %d - CMD: %s\n", running_proc->pid, running_proc->name);
		printf("\tElapsed time = %.2lf secs\n", running_proc->t_end - running_proc->t_submission);
		printf("\tExecution time = %.2lf secs\n", running_proc->t_end - running_proc->t_start);
		printf("\tWorkload time = %.2lf secs\n", running_proc->t_end - global_t);
	}
}

void start_proc(proc_t *proc) {
    int pid = fork();
    if (pid == -1) {
        err_exit("fork failed");
    }
    
    if (pid == 0) {
        signal(SIGCHLD, SIG_DFL);
        execl(proc->name, proc->name, NULL);
        perror("exec failed");
        exit(1);
    } else {
        proc->pid = pid;
        proc->status = PROC_RUNNING;
        proc->t_start = proc_gettime();
        running_proc = proc;
        proc_remove_from_rq(proc);
    }
}

void resume_proc(proc_t *proc) {
    kill(proc->pid, SIGCONT);
    proc->status = PROC_RUNNING;
    running_proc = proc;
    proc_remove_from_rq(proc);
}

void stop_proc(proc_t *proc) {
    kill(proc->pid, SIGSTOP);
    proc->status = PROC_STOPPED;
    proc_to_rq(proc);
}

void fire_scheduler() {
    if (running_proc && running_proc->status == PROC_EXITED) {
        free(running_proc);
        running_proc = NULL;
    }

    if (running_proc && running_proc->status == PROC_RUNNING) {
        return;
    }

    proc_t *next = pick_next_proc();
    if (!next) return;

    if (next->status == PROC_NEW) {
        start_proc(next);
    } else if (next->status == PROC_STOPPED) {
        resume_proc(next);
    }
}

void wait_safe(int seconds) {
    double end_time = proc_gettime() + seconds;
    
    while (1) {
        double now = proc_gettime();
        if (now >= end_time) break;
        
        double left = end_time - now;
        struct timespec req, rem;
        req.tv_sec = (time_t)left;
        req.tv_nsec = (long)((left - req.tv_sec) * 1e9);
        
        if (req.tv_sec == 0 && req.tv_nsec < 1000) {
             req.tv_nsec = 1000; 
        }

        if (nanosleep(&req, &rem) == -1) {
            if (errno == EINTR) {
                if (running_proc && running_proc->status == PROC_EXITED) {
                    fire_scheduler();
                }
            }
        }
    }
}

int main(int argc, char **argv) {
	FILE *input = NULL;
	char line[MAX_LINE_LENGTH];
    char arg1[80];
    int arg2;

	if (argc != 3) {
		err_exit("usage: ./scheduler_v1 <POLICY> <INPUT_FILE>");
	}

    if (!strcmp(argv[1], "FCFS_PNP")) {
        policy = FCFS_PNP;
    } else if (!strcmp(argv[1], "FCFS_PRIO")) {
        policy = FCFS_PRIO;
    } else {
        err_exit("Invalid policy. Use FCFS_PNP or FCFS_PRIO");
    }

    input = fopen(argv[2], "r");
    if (input == NULL) err_exit("Cannot open input file");

    proc_queue_init(&global_q);
    
    struct sigaction sig_act;
	sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	sig_act.sa_sigaction = sigchld_handler;
	sigaction(SIGCHLD, &sig_act, NULL);

    global_t = proc_gettime();

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        int count = sscanf(line, "%s %d", arg1, &arg2);
        if (count < 1) continue; // Empty line

        if (strcmp(arg1, "sleep") == 0) {
            if (count < 2) {
                 printf("Warning: sleep command without duration, ignoring\n");
                 continue;
            }
            wait_safe(arg2);
        } else {
            // New process
            proc_t *proc = malloc(sizeof(proc_t));
            strcpy(proc->name, arg1);
            // Default priority is 0 if not specified
            proc->priority = (count >= 2) ? arg2 : 0;
            proc->pid = -1;
            proc->status = PROC_NEW;
            proc->t_submission = proc_gettime();
            proc_to_rq(proc);

            if (policy == FCFS_PRIO && running_proc && running_proc->status == PROC_RUNNING) {
                if (proc->priority > running_proc->priority) {
                    stop_proc(running_proc);
                    running_proc = NULL;
                }
            }
            
            fire_scheduler();
        }
    }

    while (running_proc != NULL || global_q.first != NULL) {
        if (running_proc == NULL && global_q.first != NULL) {
             fire_scheduler();
        }
        wait_safe(1);
    }

	printf("WORKLOAD TIME: %.2lf secs\n", proc_gettime() - global_t);
	printf("scheduler exits\n");
    fclose(input);
	return 0;
}
