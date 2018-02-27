CC=gcc
CFLAGS=-ansi -pedantic -Wall -I.

.PHONY: compile producer consumer clean

compile: producer.x consumer.x

producer: producer.x
	./producer.x
producer.x: producer.o elevator_syscalls.o
	$(CC) $(CFLAGS) -o producer.x producer.o elevator_syscalls.o
producer.o: producer.c elevator_syscalls.h
	$(CC) $(CFLAGS) -o producer.o -c producer.c

consumer: consumer.x
	./consumer.x
consumer.x: consumer.o elevator_syscalls.o
	$(CC) $(CFLAGS) -o consumer.x consumer.o elevator_syscalls.o
consumer.o: consumer.c elevator_syscalls.h
	$(CC) $(CFLAGS) -o consumer.o -c consumer.c

elevator_syscalls.o: elevator_syscalls.c elevator_syscalls.h
	$(CC) $(CFLAGS) -o elevator_syscalls.o -c elevator_syscalls.c

clean:
	rm -f *.x *.o
