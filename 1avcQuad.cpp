#include <stdio.h>
#include <time.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"

int main(){
	init();
	char serverName[15] = "130.195.6.196";
	int port = 1024;
	char message[24] = "Please";

	int connection = connect_to_server(serverName, port);
	int sent;
	if (connection == 0){
		sent = send_to_server(message);
	}
	int received;
	if (sent == 0){
		receive_from_server(message);
	}
	int sent2 = send_to_server(message);
	if (sent2 == 0){
		printf("Yay, I opened the gate!");
	}
	return 0;
}
