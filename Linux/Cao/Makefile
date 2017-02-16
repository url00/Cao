CROSS_PATH=../../Cross

.PHONY: init clean

default: bin/Cao

init:
	mkdir -p bin
	mkdir -p obj

clean:
	rm -f bin/* obj/*

bin/Cao: Cao.h Cao.cpp
	gcc `pkg-config --cflags gtk+-3.0` -g -DCao_Linux Cao.cpp -o bin/Cao `pkg-config --libs gtk+-3.0`

obj/Cross.o: $(CROSS_PATH)/Cross.h $(CROSS_PATH)/Cross.cpp
	gcc -c -g -DCao_Linux -I$(CROSS_PATH) $(CROSS_PATH)/Cross.cpp -o obj/Cross.o