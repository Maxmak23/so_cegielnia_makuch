#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include "memory.h"

void test_obsluga_sygnalu_SIGTERM() {
    printf("Test: Obsluga sygnalu SIGTERM\n");

    // Tworzenie pamiecy wspoldzielonej
    int shmId = shmget(SHM_KEY, sizeof(struct Memory), IPC_CREAT | 0666);
    assert(shmId != -1);

    struct Memory *sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    assert(sharedMemory != (void *)-1);

    memset(sharedMemory, 0, sizeof(struct Memory));


    pid_t pid = fork();
    if (pid == 0) {
        // Proces dziecka
        execl("./truck", "truck", "2", "200", "15", NULL);
        perror("Blad podczas uruchamiania programu truck");
        exit(EXIT_FAILURE);
    }

    sleep(1); // Czekanie, az proces dziecka sie uruchomi
    kill(sharedMemory->pid_truck2, SIGTERM);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("OK: Program poprawnie obsluzyl sygnal SIGTERM.\n\n");
        printf("\033[1;32mTest zdany\033[0m\n");
    } else {
        printf("Blad: Program nie obsluzyl poprawnie sygnalu SIGTERM.\n\n");
    }

    // Usuwanie pamieci wspoldzielonej
    shmdt(sharedMemory);
    shmctl(shmId, IPC_RMID, NULL);
}

int main() {
    printf("Rozpoczynanie testow programu truck...\n\n");

    test_obsluga_sygnalu_SIGTERM();

    printf("Testy zakonczone.\n");
    return 0;
}
