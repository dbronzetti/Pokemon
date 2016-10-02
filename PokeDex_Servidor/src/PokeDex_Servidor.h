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
#include <pthread.h>
#include "sockets.h"
#include "commons/log.h"

t_log* logPokeDexServer;
int PORT;

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
