# so_cegielnia_makuch
Projekt cegielnia

Instrukcja Kompilacjie

gcc -c memory.c -o memory.o
gcc main.c memory.o -o main
gcc worker.c memory.o -o worker
gcc truck.c memory.o -o truck
gcc dispatcher.c memory.o -o dispatcher



Instrukcja Uruchomienia 
./main 100 150
./worker 1
./worker 2
./worker 3
./truck 1 50 10
./truck 2 100 25
./dispatcher


Instrukcja uruchamiania testów
gcc test_worker1.c memory.o -o test_worker1
gcc test_worker2.c memory.o -o test_worker2
gcc test_worker3.c memory.o -o test_worker3

gcc test_truck1.c memory.o -o test_truck1
gcc test_truck2.c memory.o -o test_truck2
gcc test_truck3.c memory.o -o test_truck3
gcc test_truck4.c memory.o -o test_truck4
gcc test_truck5.c memory.o -o test_truck5
gcc test_truck6.c memory.o -o test_truck6

./test_worker1
./test_worker2
./test_worker3
./test_truck1
./test_truck2
./test_truck3
./test_truck4
./test_truck5
./test_truck6
