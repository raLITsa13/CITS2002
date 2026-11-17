/*
    Name: Ralitsa Narayan   
    Student ID: 25340525
*/

#include <stdio.h>
#include <string.h>

#define NUM_PROCESSES 8
#define MAX_FAULTS 8
#define QUANTUM 10
#define FAULT_PENALTY 4

struct Process {
    char name[10];
    int priority;
    int total;
    int cpuTime;
    int faults[MAX_FAULTS];
    int nfaults;
    int nextFault;
    int completed;
};

int main(int argc, char *argv[])
{
    struct Process processes[NUM_PROCESSES];

    FILE *file;
    char line[200];

    int globalTime = 0;
    int numProcesses = 0;

    if (argc != 2) {
        printf("Usage: %s input_file\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");

    if (file == NULL) {
        printf("Could not open input file.\n");
        return 1;
    }

    /*
        Read each process from the input file.
    */
    while (numProcesses < NUM_PROCESSES &&
           fgets(line, sizeof(line), file) != NULL) {

        int matched;

        matched = sscanf(line,
                         "%9s %d %d %d %d %d %d %d %d %d %d %d",
                         processes[numProcesses].name,
                         &processes[numProcesses].priority,
                         &processes[numProcesses].total,
                         &processes[numProcesses].nfaults,
                         &processes[numProcesses].faults[0],
                         &processes[numProcesses].faults[1],
                         &processes[numProcesses].faults[2],
                         &processes[numProcesses].faults[3],
                         &processes[numProcesses].faults[4],
                         &processes[numProcesses].faults[5],
                         &processes[numProcesses].faults[6],
                         &processes[numProcesses].faults[7]);

        if (matched >= 4) {
            processes[numProcesses].cpuTime = 0;
            processes[numProcesses].nextFault = 0;
            processes[numProcesses].completed = 0;

            numProcesses++;
        }
    }

    fclose(file);

    /*
        Continue until every process has completed.
    */
    while (1) {

        int completedCount = 0;
        int ranThisPass[NUM_PROCESSES] = {0};

        /*
            Count completed processes.
        */
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].completed) {
                completedCount++;
            }
        }

        if (completedCount == numProcesses) {
            break;
        }

        /*
            Each pass gives every unfinished process one quantum.
            Processes are selected according to priority.
        */
        while (1) {

            int selected = -1;

            /*
                Find the highest-priority unfinished process
                that has not yet run in this pass.
            */
            for (int i = 0; i < numProcesses; i++) {

                if (processes[i].completed || ranThisPass[i]) {
                    continue;
                }

                if (selected == -1 ||
                    processes[i].priority < processes[selected].priority) {

                    selected = i;
                }
            }

            /*
                Every unfinished process has now run once.
                Start a new pass.
            */
            if (selected == -1) {
                break;
            }

            ranThisPass[selected] = 1;

            struct Process *p = &processes[selected];

            int oldCpuTime = p->cpuTime;
            int remaining = p->total - p->cpuTime;
            int execution;

            /*
                If a fault occurs exactly at the point where the
                previous quantum ended, apply its penalty now.
            */
            if (p->nextFault < p->nfaults &&
                p->faults[p->nextFault] == p->cpuTime) {

                globalTime += FAULT_PENALTY;
                p->nextFault++;
            }

            /*
                The process receives up to one full quantum.
            */
            if (remaining < QUANTUM) {
                execution = remaining;
            } else {
                execution = QUANTUM;
            }

            p->cpuTime += execution;
            globalTime += execution;

            /*
                Check for page faults occurring strictly inside
                this quantum. A fault exactly at the end of the
                quantum is handled when the process runs again.
            */
            while (p->nextFault < p->nfaults &&
                   p->faults[p->nextFault] > oldCpuTime &&
                   p->faults[p->nextFault] < p->cpuTime) {

                globalTime += FAULT_PENALTY;
                p->nextFault++;
            }

            /*
                A process is complete when its actual CPU progress
                reaches its required execution time.
            */
            if (p->cpuTime >= p->total) {

                /*
                    If the final execution point is also a page fault,
                    the fault must still be charged before termination.
                */
                if (p->nextFault < p->nfaults &&
                    p->faults[p->nextFault] == p->cpuTime) {

                    globalTime += FAULT_PENALTY;
                    p->nextFault++;
                }

                p->completed = 1;

                printf("%s %d\n", p->name, globalTime);
            }
        }
    }

    return 0;
}