test:exch.o test.o
	gcc -o test exch.o test.o

tcp_server:tcp_server.o exch.o
	gcc -o tcp_server tcp_server.o exch.o

tcp_client:tcp_client.o exch.o
	gcc -o tcp_client tcp_client.o exch.o

exch.o:exch.c exch.h
	gcc -c exch.c -o exch.o

test.o:test.c exch.h
	gcc -c test.c -o test.o

tcp_server.o:tcp_server.c exch.h
	gcc -c tcp_server.c -o tcp_server.o

tcp_client.o:tcp_client.c exch.h
	gcc -c tcp_client.c -o tcp_client.o



clean:
	rm -f *.o
		
