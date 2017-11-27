CC=g++
RM=rm -f

default: all
all: WebProxy 
WebProxy: WebProxy.cpp
	$(CC) -o WebProxy WebProxy.cpp -lcrypto
clean:
	$(RM) WebProxy client
