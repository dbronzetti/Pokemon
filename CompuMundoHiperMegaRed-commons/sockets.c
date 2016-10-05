/*
 * sockets.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */
#include "sockets.h"

int openSelectServerConnection(int newSocketServerPort, int *socketServer){
	int exitcode = EXIT_SUCCESS; //Normal completition
	int socketActivated = 1;
	int rv;
	struct addrinfo hints, *ai, *p;

	assert(("ERROR - Server port is the DEFAULT (=0)", newSocketServerPort != 0)); // FAILS if the Server port is equal to default value (0)

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, string_itoa(newSocketServerPort), &hints, &ai)) != 0) {
		printf("selectserver: %s\n", gai_strerror(rv));
		exitcode = EXIT_FAILURE;
		return exitcode;
	}

	for(p = ai; p != NULL; p = p->ai_next) {
    	*socketServer = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (*socketServer < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(*socketServer, SOL_SOCKET, SO_REUSEADDR, &socketActivated, sizeof(socketActivated));

		if (bind(*socketServer, p->ai_addr, p->ai_addrlen) < 0) {
			//Free socket created
			close(*socketServer);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		printf("Failed bind in openServerConnection() - Please check whether another process is using port: %d \n",newSocketServerPort);
		exitcode = EXIT_FAILURE;
		return exitcode;
	}

	freeaddrinfo(ai); // all done with this

	return exitcode;
}

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

int sendClientMessage(int *socketClient, char* mensaje, enum_messages tipoMensaje){
	int exitcode = EXIT_SUCCESS; //Normal completition
	int bufferSize = 0;
	int messageLen = 0;
	int payloadSize = 0;

	t_Mensaje *messageACK = malloc(sizeof(t_Mensaje));
	messageACK->tipo = tipoMensaje;

//	processString = getProcessString(process);

	messageACK->mensaje = string_new();
	string_append_with_format(&messageACK->mensaje,mensaje);
	messageLen = strlen(messageACK->mensaje);

	payloadSize = sizeof(messageACK->mensaje) + sizeof(messageLen) + messageLen + 1; // process + length message + message + 1 (+1 because of '\0')
	bufferSize = sizeof(bufferSize) + payloadSize ;//+1 because of '\0'

	char *buffer = malloc(bufferSize);
	serializeClientMessage(messageACK, buffer, payloadSize);//has to be sent the PAYLOAD size!!

	exitcode = send(*socketClient, (void*) buffer, bufferSize,0) == -1 ? EXIT_FAILURE : EXIT_SUCCESS ;

	free(buffer);
	free(messageACK->mensaje);
	free(messageACK);

	return exitcode;
}

void deserializeClientMessage(t_Mensaje *value, char *bufferReceived){
    int offset = 0;
    int messageLen = 0;

    //1)process
    memcpy(&value->tipo, bufferReceived, sizeof(value->tipo));
    offset += sizeof(value->tipo);

    //2)message length
	memcpy(&messageLen, bufferReceived + offset, sizeof(messageLen));
	offset += sizeof(messageLen);

    //3)message
	value->mensaje = malloc(messageLen);
    memcpy(value->mensaje, bufferReceived + offset, messageLen);

}

void serializeClientMessage(t_Mensaje *value, char *buffer, int valueSize){
    int offset = 0;

    //0)valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

    //1)process
    memcpy(buffer + offset, &value->tipo, sizeof(value->tipo));
    offset += sizeof(value->tipo);

    //2)message length
    int messageLen = strlen(value->mensaje) + 1;//+1 because of '\0'
	memcpy(buffer + offset, &messageLen, sizeof(messageLen));
	offset += sizeof(messageLen);

    //3)message
    memcpy(buffer + offset, value->mensaje, messageLen);

}
