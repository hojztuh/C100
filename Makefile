OBJSERVER=server.o
OBJCLIENT=client.o
CFLAGS+=-c -Wall -g -std=c++11
GG=g++

server:$(OBJSERVER)
	$(GG) $(OBJSERVER) -o $@

client:$(OBJCLIENT)
	$(GG) $(OBJCLIENT) -lpthread -o $@

*.o:*.cpp
	$(GG) $(CFLAGS) $^ -o $@

clean:
	$(RM) -rf server client *.o

all:server client