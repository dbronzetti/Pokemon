/*
 * sockets.h
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <commons/string.h>

typedef enum{
	ACCEPTED=0,
	ENTRENADOR,
	MAPA,
	POKEDEX_CLIENTE,
	POKEDEX_SERVIDOR
} enum_processes;

typedef struct{
	enum_processes process;
	char *message;
} t_MessageGenericHandshake;

int openSelectServerConnection(int newSocketServerPort, int *socketServer);
int openServerConnection(int newSocketServerPort, int *socketServer);
int acceptClientConnection(int *socketServer, int *socketClient);
int openClientConnection(char *IPServer, int PortServer, int *socketClient);
int sendClientHandShake(int *socketClient, enum_processes process);
int sendClientAcceptation(int *socketClient);
int sendMessage (int *socketClient, void *buffer, int bufferSize);
int receiveMessage(int *socketClient, void *messageRcv, int bufferSize);
void serializeHandShake(t_MessageGenericHandshake *value, char *buffer, int valueSize);
void deserializeHandShake(t_MessageGenericHandshake *value, char *bufferReceived);

char *getProcessString (enum_processes process);

#endif /* SOCKETS_H_ */
