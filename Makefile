CC=g++
RM=rm -f

default: all
all: proxy 
proxy: proxy.cpp
	$(CC) -o proxy proxy.cpp -lcrypto
clean:
	$(RM) proxy client
