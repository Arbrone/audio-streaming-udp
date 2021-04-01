CFLAGS=-g -Wall -I./lib

all: folder bin/serveur bin/client


bin/serveur: obj/audioserver.o obj/audio.o obj/packet.o obj/clientserver.o obj/filters.o
	gcc -o bin/server obj/audioserver.o obj/audio.o obj/packet.o obj/clientserver.o obj/filters.o

bin/client: obj/audioclient.o obj/audio.o obj/clientserver.o obj/packet.o obj/filters.o
	gcc -o bin/client obj/audioclient.o obj/audio.o obj/packet.o obj/clientserver.o obj/filters.o

obj/audioserver.o: src/audioserver.c
	gcc $(CFLAGS) -c  src/audioserver.c -o obj/audioserver.o

obj/audioclient.o: src/audioclient.c 
	gcc $(CFLAGS) -c  src/audioclient.c -o obj/audioclient.o

obj/audio.o: include/lib/sysprog-audio-1.5/audio.c include/lib/sysprog-audio-1.5/audio.h
	gcc $(CFLAGS) -c include/lib/sysprog-audio-1.5/audio.c -o obj/audio.o

obj/packet.o: include/packet.c include/packet.h
	gcc $(CFLAGS) -c include/packet.c -o obj/packet.o

obj/clientserver.o: include/clientserver.c include/clientserver.h
	gcc $(CFLAGS) -c include/clientserver.c -o obj/clientserver.o

obj/filters.o: include/filters.c include/filters.h
	gcc $(CFLAGS) -c include/filters.c -o obj/filters.o

folder:
	mkdir -p bin obj

clean :
	rm obj/*.o bin/*

