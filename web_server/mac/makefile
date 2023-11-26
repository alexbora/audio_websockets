
.PHONY: clean all debug poll kqueue
all: poll debug kqueue 

kqueue:
	gcc -O3 server_mac_kqueue.c -o server_mac_kqueue

poll: 
	gcc -O3 server_mac_poll.c ../log.c -o server_mac_poll

debug: 
	gcc -g -O2 -Wall server_mac_kqueue.c -o server_mac_kqueue -DDEBUG
	gcc -g -O2 -Wall server_mac_poll.c ../log.c -o server_mac_poll -DDEBUG

clean: 
	rm server_mac_kqueue *.dSYM server_mac_poll
