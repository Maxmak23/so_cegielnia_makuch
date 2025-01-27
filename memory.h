#ifndef MESSAGE_H
#define MESSAGE_H

#define QUEUE_KEY 12345
#define SHM_KEY 123456

#define MSG_WYSLANIE_CEGIEL 1
#define MSG_ODEBRANIE_CEGIEL 2
#define MSG_ODEBRANIE_TERAZ_CEGIEL 3
#define MSG_CIEZAROWKA_ODJAZD 4

#define SENDER_TYPE_WORKER 1
#define SENDER_TYPE_TRUCK 2

struct Memory {
    int ceglyIlosc;
    int ceglyWaga;

    int truck1CzyWolne;
    int truck1Ladownosc;
    int truck1CzyNagle;
    int truck1PobranaIlosc;
    int truck1PobranaWaga;

    int truck2CzyWolne;
    int truck2Ladownosc;
    int truck2CzyNagle;
    int truck2PobranaIlosc;
    int truck2PobranaWaga;

    int pid_main;
    int pid_worker1;
    int pid_worker2;
    int pid_worker3;
    int pid_truck1;
    int pid_truck2;
};



struct Message{
    int messageType;
    int senderType;
    int senderId;
    int rozmiarCegly;
};

#endif // MESSAGE_H