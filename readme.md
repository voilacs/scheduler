-> A process scheduler is made in C that executes processes in a round robin manner with tslice as specified by the user.

-> The processes are added to the priority queue whenever submit is invoked also the queue is sorted with the processes with higher priorities placed always first.

-> Scheduler implemented from scratch which makes use of a very simple queue with the usual enqueue and dequeue operations. The priority scheduling has an easy solution as you can just sort them in reverse order to impplement that.

-> Each process runs for a maximum of tslice time and then sigstop signals is sent to the process to stop its execution. 

-> During the next iteration of the round robin loop sigcont with process pid is invoked to continue with the execution of process from where it was left off in the previous cycle.

link to the repo: https://github.com/voilacs/scheduler

Pair:

Anmol Kumar 2022081: Responsible for the final compilation of the scheduler and the shell. Input was invoked from the shell and sent to the scheduler.

Dhawal Garg 2022160 : Scheduler implemented from scratch which makes use of a very simple queue with the usual enqueue and dequeue operations. The priority scheduling has an easy solution as you can just sort them in reverse order to impplement that.