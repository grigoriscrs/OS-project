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

#define PROC_NEW 0
#define PROC_STOPPED 1
#define PROC_RUNNING 2
#define PROC_EXITED 3
#define PROC_WAITING 4

typedef struct proc_desc {
	struct proc_desc *next;
	struct proc_desc *all_next;
	char name[80];
	int pid;
	int status;
	double t_submission, t_start, t_end;
} proc_t;

struct single_queue {
	proc_t	 *first;
	proc_t	 *last;
	long members;
};

struct single_queue global_q;
proc_t *all_procs = NULL;
proc_t *running_proc = NULL;
int active_procs = 0;

#define FCFS 0
#define RR 1

int policy = FCFS;
int quantum = 100;    /* ms */
double global_t;

// --- Queue Management ---

void proc_queue_init(struct single_queue *q) {
	q->first = q->last = NULL;
	q->members = 0;
}

void proc_to_rq_end(proc_t *proc) {
	if (global_q.first == NULL) {
		global_q.first = global_q.last = proc;
	} else {
		global_q.last->next = proc;
		global_q.last = proc;
	}
	proc->next = NULL;
	global_q.members++;
}

proc_t *proc_rq_dequeue() {
	proc_t *proc = global_q.first;
	if (proc != NULL) {
		global_q.first = proc->next;
		if (global_q.first == NULL) global_q.last = NULL;
		proc->next = NULL;
		global_q.members--;
	}
	return proc;
}

// --- Helpers ---

double proc_gettime() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) (tv.tv_sec + tv.tv_usec / 1000000.0);
}

void err_exit(char *msg) {
	printf("Error: %s\n", msg);
	exit(1);
}

proc_t *find_proc(int pid) {
	proc_t *curr = all_procs;
	while (curr) {
		if (curr->pid == pid) return curr;
		curr = curr->all_next;
	}
	return NULL;
}

// --- Signal Handlers ---

void sigchld_handler(int signo, siginfo_t *info, void *context) {
	proc_t *proc = find_proc(info->si_pid);
	if (!proc) return;

	if (info->si_code == CLD_EXITED || info->si_code == CLD_KILLED) {
		printf("child %d exited\n", info->si_pid);
		proc->status = PROC_EXITED;
		proc->t_end = proc_gettime();
		active_procs--;
		
		if (running_proc && running_proc->pid == info->si_pid) {
			running_proc = NULL;
		}

		printf("PID %d - CMD: %s\n", proc->pid, proc->name);
		printf("\tElapsed time = %.2lf secs\n", proc->t_end - proc->t_submission);
		printf("\tExecution time = %.2lf secs\n", proc->t_end - proc->t_start);
		printf("\tWorkload time = %.2lf secs\n", proc->t_end - global_t);
	}
}

void sigusr1_handler(int signo, siginfo_t *info, void *context) {
	// I/O Start
	proc_t *proc = find_proc(info->si_pid);
	if (proc && proc->status == PROC_RUNNING) {
		printf("Process %d started I/O\n", info->si_pid);
		proc->status = PROC_WAITING;
		if (running_proc == proc) {
			running_proc = NULL;
		}
	}
}

void sigusr2_handler(int signo, siginfo_t *info, void *context) {
	// I/O End
	proc_t *proc = find_proc(info->si_pid);
	if (proc && proc->status == PROC_WAITING) {
		printf("Process %d finished I/O\n", info->si_pid);
		proc->status = PROC_STOPPED;
		proc_to_rq_end(proc);
	}
}

// --- Scheduler Logic ---

void fire_scheduler() {
	if (running_proc != NULL) return;

	proc_t *next = proc_rq_dequeue();
	if (!next) return;

	if (next->status == PROC_NEW) {
		next->t_start = proc_gettime();
		int pid = fork();
		if (pid == -1) err_exit("fork failed");
		if (pid == 0) {
			printf("executing %s\n", next->name);
			execl(next->name, next->name, NULL);
			perror("execl failed");
			exit(1);
		} else {
			next->pid = pid;
			next->status = PROC_RUNNING;
			running_proc = next;
		}
	} else if (next->status == PROC_STOPPED) {
		next->status = PROC_RUNNING;
		running_proc = next;
		kill(next->pid, SIGCONT);
	}
}

void fcfs() {
	while (active_procs > 0) {
		fire_scheduler();
		pause();
	}
}

void rr() {
	struct timespec req, rem;
	req.tv_sec = quantum / 1000;
	req.tv_nsec = (quantum % 1000) * 1000000;

	while (active_procs > 0) {
		fire_scheduler();
		if (running_proc) {
			if (nanosleep(&req, &rem) == -1) {
				// Interrupted by signal (e.g., child exited or I/O)
				// If running_proc still exists and is still RUNNING, we can continue or stop
			}
			
			if (running_proc && running_proc->status == PROC_RUNNING) {
				kill(running_proc->pid, SIGSTOP);
				running_proc->status = PROC_STOPPED;
				proc_to_rq_end(running_proc);
				running_proc = NULL;
			}
		} else {
			// No process ready, just wait for I/O to finish
			pause();
		}
	}
}

int main(int argc, char **argv) {
	FILE *input;
	char exec[80];
	proc_t *proc;

	if (argc < 2) {
		err_exit("usage: ./scheduler_v2 [FCFS|RR <quantum>] <input_file>");
	}

	if (argc == 2) {
		policy = FCFS;
		input = fopen(argv[1], "r");
	} else if (!strcmp(argv[1], "FCFS")) {
		policy = FCFS;
		input = fopen(argv[2], "r");
	} else if (!strcmp(argv[1], "RR")) {
		policy = RR;
		quantum = atoi(argv[2]);
		input = fopen(argv[3], "r");
	} else {
		err_exit("invalid usage");
	}

	if (!input) err_exit("cannot open input file");

	proc_queue_init(&global_q);

	while (fscanf(input, "%s", exec) != EOF) {
		proc = malloc(sizeof(proc_t));
		proc->next = NULL;
		strcpy(proc->name, exec);
		proc->pid = -1;
		proc->status = PROC_NEW;
		proc->t_submission = proc_gettime();
		
		proc->all_next = all_procs;
		all_procs = proc;
		
		proc_to_rq_end(proc);
		active_procs++;
	}
	fclose(input);

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	sa.sa_sigaction = sigchld_handler;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_sigaction = sigusr1_handler;
	sigaction(SIGUSR1, &sa, NULL);

	sa.sa_sigaction = sigusr2_handler;
	sigaction(SIGUSR2, &sa, NULL);

	global_t = proc_gettime();

	if (policy == FCFS) fcfs();
	else rr();

	printf("WORKLOAD TIME: %.2lf secs\n", proc_gettime() - global_t);
	printf("scheduler exits\n");

	return 0;
}