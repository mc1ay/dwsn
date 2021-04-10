CC = gcc
CFLAGS = -Wall -g -c
dwsn: main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o state.o timers.o messages.o ground.o
	$(CC) -o dwsn main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o state.o timers.o messages.o ground.o -lm -linih
	rm main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o state.o timers.o messages.o ground.o
main.o:
	$(CC) $(CFLAGS) src/main.c
node.o:
	$(CC) $(CFLAGS) src/node.c
mcu_emulation.o:
	$(CC) $(CFLAGS) src/mcu_emulation.c
mcu_functions.o:
	$(CC) $(CFLAGS) src/mcu_functions.c
file_output.o:
	$(CC) $(CFLAGS) src/file_output.c
settings.o:
	$(CC) $(CFLAGS) src/settings.c
state.o:
	$(CC) $(CFLAGS) src/state.c
timers.o:
	$(CC) $(CFLAGS) src/timers.c
messages.o:
	$(CC) $(CFLAGS) src/messages.c
ground.o:
	$(CC) $(CFLAGS) src/ground.c
