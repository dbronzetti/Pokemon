/*
 * Entrenador.h
 *
 *  Created on: 31/8/2016
 *      Author: utnso
 */

#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"
#include "commons/log.h"
#include "commons/config.h"
#include "metadata.h"
#include <signal.h>
#include <pthread.h>

//Logger
t_log* logEntrenador;

//Metadata
t_metadataEntrenador metadataEntrenador;
t_metadataMapa metadataMapa;

//Funciones
void crearArchivoMetadata(char *rutaMetadata);
void imprimirArray(char** array);
t_queue* parsearObjetivos(char** objetivos); //convierte un array de  strings con objetivos a una cola donde cada elemento es un objetivo
void jugar();
void recibirMsjs();

// Funciones de Conexion
int connectTo(enum_processes processToConnect, int *socketClient);
void enviarSimbolo(char* simbolo, int socketClient);

//Funciones de signal
void recibirSignal();
void sumarVida();
void restarVida();
void desconectarse();

#endif /* ENTRENADOR_H_ */
