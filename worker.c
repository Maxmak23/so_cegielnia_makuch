#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include "memory.h"


int running = 1;
struct Message msg;
struct Memory *sharedMemory;

int randomNumber(int min, int max) {
    return rand() % (max - min + 1) + min;
}
void handleShutdown(int sig) {
    running = 0;
}



int main(int argc, char *argv[]) {
    signal(SIGINT, handleShutdown);
    signal(SIGTERM, handleShutdown);

    if (argc != 2) {
        fprintf(stderr, "Poprawne uzycie: %s <identyfikator pracownkia>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int workerId = atoi(argv[1]);
    if(!(workerId==1 || workerId==2 || workerId==3)){
        fprintf(stderr, "<identyfikator pracownkia> musi wynosic 1,2 lub 3\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    //Łączenie z kolejką
    int msgid = msgget(QUEUE_KEY, 0666);
    if (msgid == -1) {
        perror("Blad, nie połączono z kolejką");
        exit(EXIT_FAILURE);
    }

    //Uzyskiwanie dostępu do pamieci wspoldzielonej
    int shmId = shmget(SHM_KEY, sizeof(struct Memory), 0666);
    if (shmId == -1) {
        perror("Blad laczenia z pamiecia wspoldzielona");
        exit(EXIT_FAILURE);
    }

    //Tworzenie polaczenia z pamiecia wspoldzielona
    sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    if (sharedMemory == (void *)-1) {
        perror("Nie poloczona sie z pamiecia wspoldzielona");
        exit(EXIT_FAILURE);
    }

    if(workerId==1) sharedMemory->pid_worker1 = getpid();
    if(workerId==2) sharedMemory->pid_worker2 = getpid();
    if(workerId==3) sharedMemory->pid_worker3 = getpid();

    printf("Pracownik %d rozpoczyna prace...\n", workerId);

    msg.messageType = MSG_WYSLANIE_CEGIEL;
    msg.senderType = SENDER_TYPE_WORKER;
    msg.senderId = workerId;
    msg.rozmiarCegly = 0;

    while (running) {
        msg.rozmiarCegly = workerId;
        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) >= 0) {
            printf("Pracownik %d, cegla rozmiar %d\n", workerId, msg.rozmiarCegly);
        } else {
            perror("Blad otrzymywania wiadomosci");
            break;
        }
        sleep(2);
    }


    //Odłączanie połączenia do pamięci współdzielonej
    if (shmdt(sharedMemory) == -1) {
        perror("Nie odloczono pamieci wspoldzielonej");
    }

    printf("Koniec pracy...\n");

    return 0;
}