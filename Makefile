CFLAGS=-g -Wall -I./lib

all: bin/lecteur bin/serveur bin/client

bin/lecteur: obj/lecteur.o obj/audio.o
	gcc -o bin/lecteur obj/lecteur.o obj/audio.o

bin/serveur: obj/audioserver.o obj/audio.o obj/packet.o obj/clientserver.o
	gcc -o bin/server obj/audioserver.o obj/audio.o obj/packet.o obj/clientserver.o

bin/client: obj/audioclient.o obj/audio.o obj/clientserver.o obj/packet.o
	gcc -o bin/client obj/audioclient.o obj/audio.o obj/packet.o obj/clientserver.o

obj/audioserver.o: src/audioserver.c
	gcc $(CFLAGS) -c  src/audioserver.c -o obj/audioserver.o

obj/audioclient.o: src/audioclient.c 
	gcc $(CFLAGS) -c  src/audioclient.c -o obj/audioclient.o

obj/lecteur.o: src/lecteur.c include/lib/sysprog-audio-1.5/audio.h
	gcc $(CFLAGS) -c  src/lecteur.c -o obj/lecteur.o

obj/audio.o: include/lib/sysprog-audio-1.5/audio.c include/lib/sysprog-audio-1.5/audio.h
	gcc $(CFLAGS) -c include/lib/sysprog-audio-1.5/audio.c -o obj/audio.o

obj/packet.o: include/packet.c include/packet.h
	gcc $(CFLAGS) -c include/packet.c -o obj/packet.o

obj/clientserver.o: include/clientserver.c include/clientserver.h
	gcc $(CFLAGS) -c include/clientserver.c -o obj/clientserver.o

clean :
	rm obj/*.o bin/*

