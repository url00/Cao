CROSS_PATH=../../Cross

.PHONY: init clean cleanDebug

default: Debug

Debug: init bin/Debug/Cao

init:
	mkdir -p bin/Debug
	mkdir -p obj/Debug

cleanDebug: clean

clean:
	rm -rf bin obj


bin/Debug/Cao: Cao.h Cao.cpp obj/Debug/Cross.o
	g++ `pkg-config --cflags gtk+-3.0` -g -DCao_Linux -I$(CROSS_PATH) obj/Debug/Cross.o Cao.cpp -o bin/Debug/Cao `pkg-config --libs gtk+-3.0`

obj/Debug/Cross.o: $(CROSS_PATH)/Cross.h $(CROSS_PATH)/Cross.cpp
	g++ -c -g -DCao_Linux -I$(CROSS_PATH) $(CROSS_PATH)/Cross.cpp -o obj/Debug/Cross.o