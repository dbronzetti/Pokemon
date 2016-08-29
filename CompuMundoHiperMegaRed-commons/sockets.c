/*
 * sockets.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */
#include "sockets.h"

int openServerConnection(int newSocketServerPort, int *socketServer){
	int exitcode = EXIT_SUCCESS; //Normal completition

	struct sockaddr_in newSocketInfo;

	assert(("ERROR - Server port is the DEFAULT (=0)", newSocketServerPort != 0)); // FAILS if the Server port is equal to default value (0)

	newSocketInfo.sin_family = AF_INET; //AF_INET = IPv4
	newSocketInfo.sin_addr.s_addr = INADDR_ANY; //
	newSocketInfo.sin_port = htons(newSocketServerPort);

	*socketServer = socket(AF_INET, SOCK_STREAM, 0);

	int socketActivated = 1;

	//This line is to notify to the SO that the socket created is going to be reused
	setsockopt(*socketServer, SOL_SOCKET, SO_REUSEADDR, &socketActivated, sizeof(socketActivated));

	if (bind(*socketServer, (void*) &newSocketInfo, sizeof(newSocketInfo)) != 0){
		perror("Failed bind in openServerConnection()\n"); //TODO => Agregar logs con librerias
		printf("Please check whether another process is using port: %d \n",newSocketServerPort);

		//Free socket created
		close(*socketServer);

		exitcode = EXIT_FAILURE;
		return exitcode;
	}

	listen(*socketServer, SOMAXCONN); //SOMAXCONN = Constant with the maximum connections allowed by the SO

	return exitcode;
}

int openClientConnection(char *IPServer, int PortServer, int *socketClient){
	int exitcode = EXIT_SUCCESS; //Normal completition

	struct sockaddr_in serverSocketInfo;

	serverSocketInfo.sin_family = AF_INET; //AF_INET = IPv4
	serverSocketInfo.sin_addr.s_addr = inet_addr(IPServer); //
	serverSocketInfo.sin_port = htons(PortServer);

	*socketClient = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(*socketClient, (void*) &serverSocketInfo, sizeof(serverSocketInfo)) != 0){
		perror("Failed connect to server in OpenClientConnection()"); //TODO => Agregar logs con librerias
		printf("Please check whether the server '%s' is up or the correct port is: %d \n",IPServer, PortServer);

		//Free socket created
		close(*socketClient);
		exitcode = EXIT_FAILURE;
		return exitcode;
	}

	return exitcode;
}

int acceptClientConnection(int *socketServer, int *socketClient){
	int exitcode = EXIT_SUCCESS; //Normal completition
	struct sockaddr_in clientConnection;
	unsigned int addressSize = sizeof(clientConnection); //The addressSize has to be initialized with the size of sockaddr_in before passing it to accept function

	*socketClient = accept(*socketServer, (void*) &clientConnection, &addressSize);

	if (*socketClient != -1){
		//printf("The was received a connection in socket: %d.\n", *socketClient); --> adding logs in programs
		exitcode = EXIT_SUCCESS;
	}else{
		perror("Failed to get a new connection"); //TODO => Agregar logs con librerias
		exitcode = EXIT_FAILURE;
	}

	return exitcode;
}

int sendClientHandShake(int *socketClient, enum_processes process){
	int exitcode = EXIT_SUCCESS; //Normal completition
	int bufferSize = 0;
	int messageLen = 0;
	int payloadSize = 0;
	char *processString;

	t_MessageGenericHandshake *messageACK = malloc(sizeof(t_MessageGenericHandshake));
	messageACK->process = process;

	processString = getProcessString(process);

	messageACK->message = string_new();
	string_append_with_format(&messageACK->message,"The process '%s' is trying to connect you!\0",processString);
	messageLen = strlen(messageACK->message);

	payloadSize = sizeof(messageACK->process) + sizeof(messageLen) + messageLen + 1; // process + length message + message + 1 (+1 because of '\0')
	bufferSize = sizeof(bufferSize) + payloadSize ;//+1 because of '\0'

	char *buffer = malloc(bufferSize);
	serializeHandShake(messageACK, buffer, payloadSize);//has to be sent the PAYLOAD size!!

	exitcode = send(*socketClient, (void*) buffer, bufferSize,0) == -1 ? EXIT_FAILURE : EXIT_SUCCESS ;

	free(buffer);
	free(messageACK->message);
	free(messageACK);

	return exitcode;
}

int sendClientAcceptation(int *socketClient){
	int exitcode = EXIT_SUCCESS; //Normal completition
	int bufferSize = 0;
	int messageLen = 0;
	int payloadSize = 0;

	t_MessageGenericHandshake *messageACK = malloc(sizeof(t_MessageGenericHandshake));
	messageACK->process = ACCEPTED;
	messageACK->message = string_new();
	string_append(&messageACK->message,"The server has accepted your connection!\0");//ALWAYS put \0 for finishing the string
	messageLen = strlen(messageACK->message);

	payloadSize = sizeof(messageACK->process) + sizeof(messageLen) + messageLen + 1; // process + length message + message + 1 (+1 because of '\0')
	bufferSize = sizeof(bufferSize) + payloadSize ;//+1 because of '\0'

	char *buffer = malloc(bufferSize);
	serializeHandShake(messageACK, buffer, payloadSize);//has to be sent the PAYLOAD size!!

	exitcode = send(*socketClient, (void*) buffer, bufferSize,0) == -1 ? EXIT_FAILURE : EXIT_SUCCESS ;

	free(buffer);
	free(messageACK->message);
	free(messageACK);
	return exitcode;
}

int sendMessage (int *socketClient, void *buffer, int bufferSize){
	int exitcode = EXIT_SUCCESS; //Normal completition

	exitcode = send(*socketClient, buffer, bufferSize,0) == -1 ? EXIT_FAILURE : EXIT_SUCCESS ;

	return exitcode;
}

int receiveMessage(int *socketClient, void *messageRcv, int bufferSize){

	int receivedBytes = recv(*socketClient, messageRcv, bufferSize, 0);

	return receivedBytes;
}

void serializeHandShake(t_MessageGenericHandshake *value, char *buffer, int valueSize){
    int offset = 0;

    //0)valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

    //1)process
    memcpy(buffer + offset, &value->process, sizeof(value->process));
    offset += sizeof(value->process);

    //2)message length
    int messageLen = strlen(value->message) + 1;//+1 because of '\0'
	memcpy(buffer + offset, &messageLen, sizeof(messageLen));
	offset += sizeof(messageLen);

    //3)message
    memcpy(buffer + offset, value->message, messageLen);

}

void deserializeHandShake(t_MessageGenericHandshake *value, char *bufferReceived){
    int offset = 0;
    int messageLen = 0;

    //1)process
    memcpy(&value->process, bufferReceived, sizeof(value->process));
    offset += sizeof(value->process);

    //2)message length
	memcpy(&messageLen, bufferReceived + offset, sizeof(messageLen));
	offset += sizeof(messageLen);

    //3)message
	value->message = malloc(messageLen);
    memcpy(value->message, bufferReceived + offset, messageLen);

}

char *getProcessString (enum_processes process){

	char *processString;
	switch (process){
		case ENTRENADOR:{
			processString = "ENTRENADOR";
			break;
		}
		case MAPA:{
			processString = "MAPA";
			break;
		}
		case POKEDEX_CLIENTE:{
			processString = "POKEDEX_CLIENTE";
			break;
		}
		case POKEDEX_SERVIDOR:{
			processString = "POKEDEX_SERVIDOR";
			break;
		}
		default:{
			perror("Process not recognized");//TODO => Agregar logs con librerias
			printf("Invalid process '%d' tried to send a message\n",(int) process);
			processString = NULL;
		}
	}
	return processString;
}
