all:
		gcc -o battleshipServer battleshipServer.c
		gcc -o battleshipClient battleshipClient.c

battleshipServer:
	gcc -o battleshipServer battleshipServer.c

battleshipClient:
	gcc -o battleshipClient battleshipClient.c

clean:
	rm battleshipServer
	rm battleshipClient
