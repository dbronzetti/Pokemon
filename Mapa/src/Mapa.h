/*
 * Mapa.h
 *
 *  Created on: 31/8/2016
 *      Author: utnso
 */

#ifndef MAPA_H_
#define MAPA_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"
#include "commons/log.h"

t_log* logMapa;

// Estructuras
typedef struct {
	int socketServer;
	int socketClient;
} t_serverData;


// Funciones de conexion
void startServerProg();
void newClients(void *parameter);
void handShake(void *parameter);

#endif /* MAPA_H_ */
