default: 
	gcc -lm -o restool restool.c

install:
	install -p -s restool /usr/local/bin

uninstall:
	rm -f /usr/local/bin/restool
