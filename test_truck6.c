#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "memory.h"

void test_signal_handling() {
    printf("Test test_invalid_arguments...\n");

    // Tworzenie pamiecy wspoldzielonej
    int shmId = shmget(SHM_KEY, sizeof(struct Memory), IPC_CREAT | 0666);
    assert(shmId != -1);

    struct Memory *sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    assert(sharedMemory != (void *)-1);

    memset(sharedMemory, 0, sizeof(struct Memory));

    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = {"./truck", "2", "200", "5", NULL};
        execl(args[0], args[0], args[1], args[2], args[3], NULL);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }else{
        sleep(2);

        kill(pid, SIGINT);
        sleep(1);
        assert(sharedMemory->truck2CzyNagle == 1);

        kill(sharedMemory->pid_truck2, SIGTERM);
        waitpid(pid, NULL, 0);

        // Usuwanie pamieci wspoldzielonej
        shmdt(sharedMemory);
        shmctl(shmId, IPC_RMID, NULL);

        printf("Test test_signal_handling zdany\n");
        printf("\033[1;32mTest zdany\033[0m\n");
    }
}

int main() {
    test_signal_handling();
    return 0;
}
