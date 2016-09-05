/*
 * metadata.h
 *
 *  Created on: 2/9/2016
 *      Author: utnso
 */

#ifndef METADATA_H_
#define METADATA_H_

#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/queue.h"

typedef struct {
	char* nombre;
	char* simbolo;
	char** hojaDeViaje;
	t_queue* obj;
	int vidas;
	int reintentos;
} t_metadataEntrenador;

typedef struct {
	int tiempoChequeoDeadlock;
	int batalla;
	char** algoritmo;
	int quantum;
	int retardo;
	char* ip;
	int puerto;
} t_metadataMapa;



#endif /* METADATA_H_ */
