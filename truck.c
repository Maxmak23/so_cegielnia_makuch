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

struct Memory *sharedMemory;
int running = 1;
int truckId = 0;
int ladownosc;
int czasJazdy;
int czyNagle;

void handleSignal(int sig) {
    if(sig==SIGINT){
        //printf("handleSignal %d %d %d\n", sig, SIGINT, SIGTERM);
        if(truckId == 1){
            sharedMemory->truck1CzyNagle = 1;
        }else if(truckId == 2){
            sharedMemory->truck2CzyNagle = 1;
        }
    }else if(sig==SIGTERM){
       // printf("handleShutdown %d %d %d\n", sig, SIGINT, SIGTERM);
        running = 0;
    }
}

int main(int argc, char *argv[]){
    srand(time(NULL));
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    
    if (argc != 4) {
        fprintf(stderr, "Poprawne uzycie: %s <identyfikator ciezarowki> <ladownosc> <czas odjazdu>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    truckId = atoi(argv[1]);
    ladownosc = atoi(argv[2]);
    czasJazdy = atoi(argv[3]);
    czyNagle = 0;

    if(!(truckId==1 || truckId==2)){
        fprintf(stderr, "<identyfikator ciezarowki> musi wynosic 1 lub 2\n");
        exit(EXIT_FAILURE);
    }

    if(!(ladownosc>0)){
        fprintf(stderr, "<ladownosc> musi byc wieksze od 0\n");
        exit(EXIT_FAILURE);
    }

    if(!(czasJazdy>0)){
        fprintf(stderr, "<czas odjazdu> musi byc wieksze od 0\n");
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
    int prevValue = 1, isDiff = 0;

    if(truckId == 1){
        sharedMemory->truck1CzyWolne = 1;
        sharedMemory->truck1Ladownosc = ladownosc;
        sharedMemory->truck1CzyNagle = 0;
    }else if(truckId == 2){
        sharedMemory->truck2CzyWolne = 1;
        sharedMemory->truck2Ladownosc = ladownosc;
        sharedMemory->truck2CzyNagle = 0;
    }

    if(truckId==1) sharedMemory->pid_truck1 = getpid();
    if(truckId==2) sharedMemory->pid_truck2 = getpid();


    printf("Ciezarowka %d rozpoczyna prace...\n", truckId);

    while (running){
        isDiff = 0;
        if(truckId == 1){
            isDiff = sharedMemory->truck1CzyWolne != prevValue;
        }else if(truckId == 2){
            isDiff = sharedMemory->truck2CzyWolne != prevValue;
        }

        if(isDiff){
            if(truckId == 1){
                printf("Ciezarowka %d pobrala %d cegiel o wadze %d i jedzie na %d sekund.\n",truckId,sharedMemory->truck1PobranaIlosc,sharedMemory->truck1PobranaWaga,czasJazdy);
            }else if(truckId == 2){
                printf("Ciezarowka %d pobrala %d cegiel o wadze %d i jedzie na %d sekund.\n",truckId,sharedMemory->truck2PobranaIlosc,sharedMemory->truck2PobranaWaga,czasJazdy);
            }
            sleep(czasJazdy);
            if(truckId == 1){
                sharedMemory->truck1CzyWolne = 1;
                printf("Ciezarowka %d wrocila i jest gotowa pobrac nowy ladunek.\n\n",truckId);
            }else if(truckId == 2){
                sharedMemory->truck2CzyWolne = 1;
                printf("Ciezarowka %d wrocila i jest gotowa pobrac nowy ladunek.\n\n",truckId);
            }
        }

        if(truckId == 1){
            prevValue = sharedMemory->truck1CzyWolne;
        }else if(truckId == 2){
            prevValue = sharedMemory->truck2CzyWolne;
        }
        sleep(1);
    }

    //Odłączanie połączenia do pamięci współdzielonej
    if (shmdt(sharedMemory) == -1) {
        perror("Nie odloczono pamieci wspoldzielonej");
    }


    printf("Koniec pracy...\n");

    return 0;
}
