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
#include "commons/config.h"

/*Atencion!: Cada vez que se desencola un elemento de la hojaDeViaje se debe desencolar su objetivo */
typedef struct {
	char* nombre;
	char* simbolo;
	t_queue* hojaDeViaje; //Una cola en la que cada elemento es el nombre de un mapa, ejemplo de elementos: "Pueblo Paleta", "Ciudad Verde"
	t_queue* obj; //Una cola en la que cada elementos son los pokemones que debe capturar del mapa, ejemplo de elementos: "A, B , C", "F , H , J"
	int vidas;
	int reintentos;
} t_metadataEntrenador;

typedef struct {
	char* tipo;
	int pos_x;
	int pos_y;
	char id;
} t_metadataPokenest;

typedef struct {
	int tiempoChequeoDeadlock;
	int batalla;
	char* algoritmo;
	int quantum;
	int retardo;
	char* ip;
	int puerto;
} t_metadataMapa;


//Funciones
void crearArchivoMetadataDelMapa(char* rutaMetadataMapa, t_metadataMapa* metadataMapa);

#endif /* METADATA_H_ */
