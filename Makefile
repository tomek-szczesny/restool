default: 
	gcc -o restool restool.c -lm

install:
	install -p -s restool /usr/local/bin

uninstall:
	rm -f /usr/local/bin/restool
