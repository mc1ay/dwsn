dwsn: main.o
	cc main.o -o dwsn
main.o:
	cc -c src/main.c
