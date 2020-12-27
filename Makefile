dwsn: main.o node.o mcu_emulation.o
	cc -o dwsn -lm main.o node.o mcu_emulation.o
	rm main.o node.o mcu_emulation.o
main.o:
	cc -Wall -g -c src/main.c
node.o:
	cc -Wall -g -c src/node.c
mcu_emulation.o:
	cc -Wall -g -c src/mcu_emulation.c
