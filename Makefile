dwsn: main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o
	cc -o dwsn -lm main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o
	rm main.o node.o mcu_emulation.o mcu_functions.o file_output.o settings.o
main.o:
	cc -Wall -g -c src/main.c
node.o:
	cc -Wall -g -c src/node.c
mcu_emulation.o:
	cc -Wall -g -c src/mcu_emulation.c
mcu_functions.o:
	cc -Wall -g -c src/mcu_functions.c
file_output.o:
	cc -Wall -g -c src/file_output.c
settings.o:
	cc -Wall -g -c src/settings.c