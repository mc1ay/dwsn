dwsn: main.o node.o mcu_emulation.o
	cc -o dwsn -lm main.o node.o mcu_emulation.o
	rm main.o node.o mcu_emulation.o
main.o:
	cc -c src/main.c
node.o:
	cc -c src/node.c
mcu_emulation.o:
	cc -c src/mcu_emulation.c
