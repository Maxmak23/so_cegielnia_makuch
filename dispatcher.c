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


void SendSignal(int signal, const char* PROGNAME, int pid) {
    if(pid>0){
        if(signal==SIGINT) printf("Wysylanie sygnalu SIGINT do %s (PID: %d)\n", PROGNAME, pid);
        else if(signal==SIGTERM) printf("Wysylanie sygnalu SIGTERM do %s (PID: %d)\n", PROGNAME, pid);
        else printf("Wysylanie sygnalu do %s (PID: %d)\n", PROGNAME, pid);
        if (kill(pid, signal) == 0) {
        } else {
            perror("Nie wyslano sygnalu");
        }
    }else{
        perror("Blad PID programu");
    }
}



void menu() {
    int choice;
    while (running) {
        printf("\n=== Dispatcher Menu ===\n");
        printf("1. Zakoncz Dispatcher\n");
        printf("2. Zakoncz Program Main\n");

        printf("3. Zakoncz Worker 1\n");
        printf("4. Zakoncz Worker 2\n");
        printf("5. Zakoncz Worker 3\n");
        printf("6. Zakoncz All Workers\n");

        printf("7. Zakoncz Truck 1\n");
        printf("8. Zakoncz Truck 2\n");
        printf("9. Zakoncz All Trucks\n");

        printf("10. Truck 1 pobiera celgy natychmiast\n");
        printf("11. Truck 2 pobiera celgy natychmiast\n");

        printf("12. Zakoncz prace wszystkiego\n");

        printf("Twoj wybor: ");


        scanf("%d", &choice);

        switch (choice) {
            case 1:
                running = 0;
                break;
            case 2:
                SendSignal(SIGTERM,"Main", sharedMemory->pid_main);
                break;


            case 3:
                SendSignal(SIGTERM,"Worker 1", sharedMemory->pid_worker1);
                break;
            case 4:
                SendSignal(SIGTERM,"Worker 2", sharedMemory->pid_worker2);
                break;
            case 5:
                SendSignal(SIGTERM,"Worker 3", sharedMemory->pid_worker3);
                break;
            case 6:
                SendSignal(SIGTERM,"Worker 1", sharedMemory->pid_worker1);
                SendSignal(SIGTERM,"Worker 2", sharedMemory->pid_worker2);
                SendSignal(SIGTERM,"Worker 3", sharedMemory->pid_worker3);
                break;


            case 7:
                SendSignal(SIGTERM,"Truck 1", sharedMemory->pid_truck1);
                break;
            case 8:
                SendSignal(SIGTERM,"Truck 2", sharedMemory->pid_truck2);
                break;
            case 9:
                SendSignal(SIGTERM,"Truck 1", sharedMemory->pid_truck1);
                SendSignal(SIGTERM,"Truck 2", sharedMemory->pid_truck2);
                break;


            case 10:
                SendSignal(SIGINT,"Truck 1", sharedMemory->pid_truck1);
                break;
            case 11:
                SendSignal(SIGINT,"Truck 2", sharedMemory->pid_truck2);
                break;


            case 12:
                SendSignal(SIGTERM,"Worker 1", sharedMemory->pid_worker1);
                SendSignal(SIGTERM,"Worker 2", sharedMemory->pid_worker2);
                SendSignal(SIGTERM,"Worker 3", sharedMemory->pid_worker3);
                SendSignal(SIGTERM,"Truck 1", sharedMemory->pid_truck1);
                SendSignal(SIGTERM,"Truck 2", sharedMemory->pid_truck2);
                SendSignal(SIGTERM,"Main", sharedMemory->pid_main);
                break;


            default:
                printf("Nie ma takiej opcji.\n");
        }
    }
}




int main(int argc, char *argv[]) {


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


    menu();


    //Odłączanie połączenia do pamięci współdzielonej
    if (shmdt(sharedMemory) == -1) {
        perror("Nie odloczono pamieci wspoldzielonej");
    }

    printf("Koniec pracy...\n");
    return 0;
}
