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


void test_nieprawidlowy_id() {
    printf("Test: Nieprawidlowy identyfikator ciezarowki\n");
    pid_t pid = fork();
    if (pid == 0) {
        // Proces dziecka
        execl("./truck", "truck", "3", "100", "10", NULL);
        perror("Blad podczas uruchamiania programu truck");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("OK: Program poprawnie zglosil blad przy nieprawidlowym ID ciezarowki.\n\n");
        printf("\033[1;32mTest zdany\033[0m\n");    
    } else {
        printf("Blad: Program nie zglosil bledu przy nieprawidlowym ID ciezarowki.\n\n");
    }
}

int main() {
    printf("Rozpoczynanie testow programu truck...\n\n");

    test_nieprawidlowy_id();

    printf("Testy zakonczone.\n");
    return 0;
}
