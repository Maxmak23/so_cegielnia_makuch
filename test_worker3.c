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


// Funkcja do tworzenia kolejki wiadomosci i pamieci wspoldzielonej
void setup_message_queue_and_shared_memory() {
    int msgid = msgget(QUEUE_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Nie udalo sie utworzyc kolejki wiadomosci");
        exit(EXIT_FAILURE);
    }

    int shmId = shmget(SHM_KEY, sizeof(struct Memory), 0666 | IPC_CREAT);
    if (shmId == -1) {
        perror("Nie udalo sie utworzyc pamieci wspoldzielonej");
        exit(EXIT_FAILURE);
    }

    struct Memory *sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    if (sharedMemory == (void *)-1) {
        perror("Nie udalo sie podlaczyc pamieci wspoldzielonej");
        exit(EXIT_FAILURE);
    }

    sharedMemory->pid_worker1 = 0;
    sharedMemory->pid_worker2 = 0;
    sharedMemory->pid_worker3 = 0;
    shmdt(sharedMemory);
}

// Funkcja do usuwania kolejki wiadomosci i pamieci wspoldzielonej
void cleanup_message_queue_and_shared_memory() {
    int msgid = msgget(QUEUE_KEY, 0666);
    if (msgid != -1) {
        msgctl(msgid, IPC_RMID, NULL);
    }

    int shmId = shmget(SHM_KEY, sizeof(struct Memory), 0666);
    if (shmId != -1) {
        shmctl(shmId, IPC_RMID, NULL);
    }
}

// Test interakcji z kolejka wiadomosci
void test_message_queue_interaction() {
    pid_t pid = fork();
    if (pid == 0) {
        // Proces dziecka: uruchomienie worker
        execl("./worker", "worker", "2", NULL);
        perror("Nie udalo sie uruchomic worker");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Proces rodzica: odczyt z kolejki wiadomosci
        int msgid = msgget(QUEUE_KEY, 0666);
        assert(msgid != -1);

        struct Message msg;
        sleep(3); // Czekanie na wyslanie wiadomosci

        int result = msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 0, IPC_NOWAIT);
        assert(result != -1);
        printf("msg.senderId %d\n",msg.senderId);
        printf("msg.rozmiarCegly %d\n",msg.rozmiarCegly);
        assert(msg.senderId == 2); // Sprawdzanie ID worker
        assert(msg.rozmiarCegly == 2); // Sprawdzanie rozmiaru cegly

        kill(pid, SIGTERM); // Zatrzymanie worker
        wait(NULL);        // Czekanie na zakonczenie procesu dziecka
        printf("Test zaliczony: Poprawna interakcja z kolejka wiadomosci\n");
        printf("\033[1;32mTest zdany\033[0m\n");
    }
}

int main() {
    setup_message_queue_and_shared_memory();

    test_message_queue_interaction();

    cleanup_message_queue_and_shared_memory();
    return 0;
}
