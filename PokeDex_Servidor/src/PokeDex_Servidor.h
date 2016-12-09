/*
 * PokeDex_Servidor.h
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */

#ifndef POKEDEX_SERVIDOR_H_
#define POKEDEX_SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sockets.h"

pthread_mutex_t mutexG;

int PORT=0;

// Estructuras
typedef struct {
	int socketServer;
	int socketClient;
} t_serverData;

// Funciones de conexion
void startServerProg();
void newClients(void *parameter);
void handShake(void *parameter);
void processMessageReceived(void *parameter);

#endif /* POKEDEX_SERVIDOR_H_ */
