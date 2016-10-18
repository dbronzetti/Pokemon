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
#include <commons/collections/list.h>
#include "osada.h"

typedef enum{
	ACCEPTED=0,
	ENTRENADOR,
	MAPA,
	POKEDEX_CLIENTE,
	POKEDEX_SERVIDOR
} enum_processes;

typedef enum{
	NUEVO=0,
	CONOCER,
	IR,
	CAPTURAR,
	DESCONECTAR
} enum_messages;

typedef enum{
	READ=0,
	CREATE_FILE,
	WRITE_FILE,
	MODIFY_FILE,
	DELETE_FILE,
	CREATE_DIR,
	CREATE_SUBDIR,
	DELETE_DIR,
	RENAME_FILE
} enum_FUSEOperations;

typedef struct{
	enum_processes process;
	char *message;
} t_MessageGenericHandshake;

typedef struct{
	enum_messages tipo;
	char* mensaje;
} t_Mensaje;

typedef struct{
	enum_FUSEOperations operation;
	char *mensaje;
} t_MessagePokeServer_Client;

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
int sendClientMessage(int *socketClient, char* mensaje, enum_messages tipoMensaje); //Envia un string + un enum que dice que tipo de msj es.
void serializeClientMessage(t_Mensaje *value, char *buffer, int valueSize);
void deserializeClientMessage(t_Mensaje *value, char *bufferReceived);
char *serializeListaBloques(t_list* listaASerializar);
void deserializeListaBloques(t_list* listaBloques, char* listaSerializada);
char *serializeBloque(osada_file* unaPosicion, char* value, int *offset);
void deserializeBloque(osada_file* unaPosicion, char* posicionRecibida, int *offset);

char *getProcessString (enum_processes process);

#endif /* SOCKETS_H_ */
