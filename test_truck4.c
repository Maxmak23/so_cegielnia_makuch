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

void test_nieprawidlowy_czas_jazdy() {
    printf("Test: Nieprawidlowy czas jazdy\n");

    // Tworzenie pamiecy wspoldzielonej
    int shmId = shmget(SHM_KEY, sizeof(struct Memory), IPC_CREAT | 0666);
    assert(shmId != -1);

    struct Memory *sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    assert(sharedMemory != (void *)-1);

    memset(sharedMemory, 0, sizeof(struct Memory));

    pid_t pid = fork();
    if (pid == 0) {
        // Proces dziecka
        execl("./truck", "truck", "1", "100", "-5", NULL);
        perror("Blad podczas uruchamiania programu truck");
        exit(EXIT_FAILURE);
    }


    int status;
    //kill(sharedMemory->pid_truck1, SIGTERM);
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("OK: Program poprawnie zglosil blad przy nieprawidlowym czasie jazdy.\n\n");
        printf("\033[1;32mTest zdany\033[0m\n");
    } else {
        printf("Blad: Program nie zglosil bledu przy nieprawidlowym czasie jazdy.\n\n");
    }

    // Usuwanie pamieci wspoldzielonej
    shmdt(sharedMemory);
    shmctl(shmId, IPC_RMID, NULL);
}

int main() {
    printf("Rozpoczynanie testow programu truck...\n\n");

    test_nieprawidlowy_czas_jazdy();

    printf("Testy zakonczone.\n");
    return 0;
}
