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
void handleShutdown(int sig) {
    running = 0;
}

int ceglyMaxIlosc;
int ceglyMaxWaga;
int ceglyIlosc = 0;
int ceglyWaga = 0;
int* tasma;
struct Memory *sharedMemory;
struct Message msg;


int ceglyDodaj(int cegla){//jezeli jest miejsce na tasmie, dodaje cegle
    if(ceglyIlosc>=ceglyMaxIlosc) return 0;
    if(ceglyWaga+cegla>ceglyMaxWaga) return 0;
    tasma[ceglyIlosc] = cegla;
    ceglyIlosc += 1;
    ceglyWaga += cegla;
    return 1;
}

void ceglyRaport(){
    printf("%d cegiel o wadze %d\n", ceglyIlosc, ceglyWaga);
}


int pob_IloscPobranych = 0;
int pob_CurrentSum = 0;
int odebranieCegiel(int ladownosc, int czyNagle){
    int doOdebrania = ladownosc;
    if(czyNagle){
        doOdebrania = ceglyWaga;
        if(doOdebrania>ladownosc) doOdebrania = ladownosc;
    }

    if(ceglyWaga>=doOdebrania){
        int currentSum = 0;
        int i, shiftIndex = 0, iloscPobranych = 0;

        // Pobieraj cegly z tasmy
        for (i = 0; i < ceglyMaxIlosc; i++) {
            if (currentSum + tasma[i] > doOdebrania || tasma[i]==0) break;
            currentSum += tasma[i];
            tasma[i] = 0; // Mark removed elements as 0
            iloscPobranych += 1;
        }

        // Przesun wszystkie liczby w lewo
        for (int j = 0; j < ceglyMaxIlosc; j++) {
            if (tasma[j] != 0) {
                tasma[shiftIndex++] = tasma[j];
            }
        }

        // zastap puste miejsca zerami
        for (int j = shiftIndex; j < ceglyMaxIlosc; j++) {
            tasma[j] = 0;
        }
        ceglyIlosc -= iloscPobranych;
        ceglyWaga -= currentSum;

        pob_IloscPobranych = iloscPobranych;
        pob_CurrentSum = currentSum;
        
        return 1;
    }
    
    
    return 0;
}


void wyslanieCiezarowek(){
    int sukces = 0;

    if(sharedMemory->truck1Ladownosc>0 && sharedMemory->truck1CzyWolne>0){
        sukces = odebranieCegiel(sharedMemory->truck1Ladownosc,sharedMemory->truck1CzyNagle);
        if(sukces){
            sharedMemory->truck1PobranaIlosc = pob_IloscPobranych;
            sharedMemory->truck1PobranaWaga = pob_CurrentSum;
            sharedMemory->truck1CzyWolne = 0;
            sharedMemory->truck1CzyNagle = 0;
        }
    }

    if(sharedMemory->truck2Ladownosc>0 && sharedMemory->truck2CzyWolne>0){
        sukces = odebranieCegiel(sharedMemory->truck2Ladownosc,sharedMemory->truck2CzyNagle);
        if(sukces){
            sharedMemory->truck2PobranaIlosc = pob_IloscPobranych;
            sharedMemory->truck2PobranaWaga = pob_CurrentSum;
            sharedMemory->truck2CzyWolne = 0;
            sharedMemory->truck2CzyNagle = 0;
        }
    }
}










int main(int argc, char *argv[]) {
    signal(SIGINT, handleShutdown);

    if (argc != 3) {
        fprintf(stderr, "Poprawne uzycie: %s <maksymalna ilosc cegiel> <maksymalna waga cegiel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ceglyMaxIlosc = atoi(argv[1]);
    ceglyMaxWaga = atoi(argv[2]);
    if(ceglyMaxIlosc<=0 || ceglyMaxWaga<=0){
        fprintf(stderr, "<maksymalna ilosc cegiel> oraz<maksymalna waga cegiel> musza byc wieksze od zera\n");
        exit(EXIT_FAILURE);
    }

    tasma = (int*)malloc(ceglyMaxIlosc * sizeof(int));
    for(int i=0;i<ceglyMaxIlosc;i+=1) tasma[i] = 0;




    //Tworzenie kolejki
    int msgid = msgget(QUEUE_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Blad, nie utworzono kolejki");
        exit(EXIT_FAILURE);
    }


    //tworzenie pamieci wspoldzielonej
    int shmId = shmget(SHM_KEY, sizeof(struct Memory), IPC_CREAT | 0666);
    if (shmId == -1) {
        perror("Nie stworzono pamieci wspoldzielonej");
        exit(EXIT_FAILURE);
    }

    //tworzenie polaczenia do pamieci wspoldzielonej
    sharedMemory = (struct Memory *)shmat(shmId, NULL, 0);
    if (sharedMemory == (void *)-1) {
        perror("Nie poloczona sie z pamiecia wspoldzielona");
        exit(EXIT_FAILURE);
    }
    
    //czyszczenie pamieci wspoldzieloenj
    memset(sharedMemory, 0, sizeof(struct Memory));
    sharedMemory->truck1CzyWolne = -1;
    sharedMemory->truck1Ladownosc = -1;
    sharedMemory->truck1CzyNagle = 0;

    sharedMemory->truck2CzyWolne = -1;
    sharedMemory->truck2Ladownosc = -1;
    sharedMemory->truck2CzyNagle = 0;

    sharedMemory->pid_main = 0;
    sharedMemory->pid_worker1 = 0;
    sharedMemory->pid_worker2 = 0;
    sharedMemory->pid_worker3 = 0;
    sharedMemory->pid_truck1 = 0;
    sharedMemory->pid_truck2 = 0;

    sharedMemory->pid_main = getpid();



    printf("Program Main rozpoczyna prace\n");

    while (running) {

        // Otrzymywanie wiadomosci
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 0, 0) >= 0) {
            
            if(msg.messageType == MSG_WYSLANIE_CEGIEL){
                //msg.messageType = MSG_WYSLANIE_CEGIEL;
                //msg.senderType = SENDER_TYPE_WORKER;
                //msg.senderId = workerId;
                //msg.rozmiarCegly = 0;

                int odpowiedz = ceglyDodaj(msg.rozmiarCegly);
                if(odpowiedz==1){
                    ceglyRaport();
                }
                wyslanieCiezarowek();
                sharedMemory->ceglyIlosc = ceglyIlosc;
                sharedMemory->ceglyWaga = ceglyWaga;
            }

        } else {
            perror("Blad otrzymywania wiadomosci");
            break;
        }




        


    }

    //Usuwanie polaczenia do pamieci wspoldzielonej
    if (shmdt(sharedMemory) == -1) {
        perror("Nie odloczono pamieci wspoldzielonej");
    }

    //Usuwanie pamieci wspoldzielonej
    if (shmctl(shmId, IPC_RMID, NULL) == -1) {
        perror("Nie usunieto pamieci wspoldzielonej");
    }

    //Usuwanie kolejki
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Nie usunięto kolejki");
        exit(EXIT_FAILURE);
    }


    //Usuwanie z pamieci tablicy tasma
    free(tasma);
    printf("Koniec programu main...\n");

    return 0;
}