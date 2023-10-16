            #include <stdio.h>
            #include <stdlib.h>
            #include <string.h>
            #include <signal.h>
            #include <stdbool.h>
            #include <unistd.h>
            #include <sys/types.h>
            #include <sys/wait.h>
            #include <pthread.h>
            #include <sys/time.h>

            #define MAX_INPUT_SIZE 256
            #define MAX_COMMANDS 10

            typedef struct {
                char* command;
                int priority;
            } Command;

            typedef struct {
                Command queue[MAX_COMMANDS];
                int size;
            } PriorityQueue;

            PriorityQueue priorityQueue;
            bool displayQueue = false;
            pthread_mutex_t mutex;
            int numActiveProcesses = 0;

            void initPriorityQueue(PriorityQueue* pq) {
                pq->size = 0;
            }

            void enqueue(PriorityQueue* pq, const char* command, int priority) {
                if (pq->size < MAX_COMMANDS) {
                    Command newCommand;
                    newCommand.command = strdup(command);
                    newCommand.priority = priority;

                    // Find the correct position to insert the new command based on priority
                    int insertPosition = pq->size;
                    for (int i = 0; i < pq->size; i++) {
                        if (priority > pq->queue[i].priority) {
                            insertPosition = i;
                            break;
                        }
                    }

                    // Shift existing commands to make space for the new command
                    for (int i = pq->size; i > insertPosition; i--) {
                        pq->queue[i] = pq->queue[i - 1];
                    }

                    // Insert the new command at the correct position
                    pq->queue[insertPosition] = newCommand;
                    pq->size++;
                } else {
                    printf("Priority queue is full. Cannot enqueue.\n");
                }
            }

            Command dequeue(PriorityQueue* pq) {
                if (pq->size > 0) {
                    Command cmd = pq->queue[0];
                    for (int i = 1; i < pq->size; i++) {
                        pq->queue[i - 1] = pq->queue[i];
                    }
                    pq->size--;
                    return cmd;
                } else {
                    Command emptyCommand = {NULL, -1};
                    return emptyCommand;
                }
            }

            void cleanupPriorityQueue(PriorityQueue* pq) {
                for (int i = 0; i < pq->size; i++) {
                    free(pq->queue[i].command);
                }
                pq->size = 0;
            }

            void handleCtrlR(int sig) {
                if (sig == SIGINT) {
                    displayQueue = !displayQueue;
                }
            }

            void displayPriorityQueue() {
                if (displayQueue) {
                    printf("Priority Queue:\n");
                    for (int i = 0; i < priorityQueue.size; i++) {
                        printf("Priority %d: %s\n", priorityQueue.queue[i].priority, priorityQueue.queue[i].command);
                    }
                } else {
                    printf("Press Ctrl+R to display the queue.\n");
                }
            }

            void executeProcess(Command cmd, int index, struct timeval* start_time, int TSLICE) {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    if (execlp(cmd.command, cmd.command, NULL) == -1) {
                        perror("execlp failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    // Parent process
                    struct timeval end_time;
                    gettimeofday(&end_time, NULL);
                    double execution_time = (end_time.tv_sec - start_time->tv_sec) + 
                                        (end_time.tv_usec - start_time->tv_usec) / 1000000.0;

                    if (execution_time < TSLICE) {
                        sleep(TSLICE - execution_time);
                    } else {
                        kill(pid, SIGSTOP);
                        printf("%s (Index %d) paused after %f seconds\n", cmd.command, index, execution_time);
                    }

                    waitpid(pid, NULL, 0);

                    if (execution_time >= TSLICE) {
                        // Process took more than TSLICE, add it back to the queue
                        enqueue(&priorityQueue, cmd.command, cmd.priority);
                        printf("%s (Index %d) added back to the queue\n", cmd.command, index);
                    }

                    printf("%s (Index %d) finished execution\n", cmd.command, index);
                    free(cmd.command);
                } else {
                    perror("fork failed");
                }
            }

    void runProcesses(int TSLICE, int NCPUS) {
        while (priorityQueue.size > 0) {
            for (int i = 0; i < NCPUS; i++) {
                if (priorityQueue.size > 0) {
                    Command cmd = dequeue(&priorityQueue);
                    struct timeval start_time;
                    gettimeofday(&start_time, NULL);
                    executeProcess(cmd, i, &start_time, TSLICE);
                }
            }
        }
    }


    int main(int argc, char* argv[]) {
        if (argc != 3) {
            printf("Usage: %s <NCPUS> <TSLICE>\n", argv[0]);
            return 1;
        }
        int NCPUS = atoi(argv[1]);
        int TSLICE = atoi(argv[2]);

                initPriorityQueue(&priorityQueue);
                signal(SIGINT, handleCtrlR);
                pthread_mutex_init(&mutex, NULL);

                char input[MAX_INPUT_SIZE];

                while (1) {
                    printf("Custom Shell> ");
                    if (fgets(input, sizeof(input), stdin) == NULL) {
                        break; // End of input
                    }

                    input[strcspn(input, "\n")] = '\0';

                    if (strcmp(input, "run") == 0) {
                        // Execute processes in a round-robin manner
                        runProcesses(TSLICE, NCPUS);
                    } else {
                        char command[MAX_INPUT_SIZE];
                        int priority;

                        if (sscanf(input, "submit %s %d", command, &priority) == 2) {
                            if (priority >= 1) {
                                enqueue(&priorityQueue, command, priority);
                                printf("Command \"%s\" with priority %d added to the queue.\n", command, priority);
                            } else {
                                printf("Invalid priority. Priority should be a positive integer.\n");
                            }
                        } else if (strcmp(input, "Ctrl+R") == 0) {
                            displayQueue = !displayQueue;
                            displayPriorityQueue();
                        } else {
                            printf("Invalid input. Use the format: submit <command> <priority> or press Ctrl+R to display the queue.\n");
                        }
                    }

                    if (displayQueue) {
                        displayPriorityQueue();
                    }
                }

                cleanupPriorityQueue(&priorityQueue);
                pthread_mutex_destroy(&mutex);

                return 0;
            }
