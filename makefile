all: ChatroomServer ChatroomClient

ChatroomServer: ChatroomServer
	gcc -Wall -g -o chatserver ChatroomServer.c -lpthread
	
ChatroomClient: ChatroomClient
	gcc -Wall -g -o chatroom ChatroomClient.c -lpthread

clean:
	rm -f *.o chatroom
	rm -f *.o chatserver
