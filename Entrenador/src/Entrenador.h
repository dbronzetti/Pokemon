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

//Conexion
int exitCode;

//Rutas y relacionados
char* rutaMetadata;
char* rutaDirDeBill;
char* rutaMedallas;
char* mapaActual;
char *pokedex;

//Hilos
pthread_t hiloSignal; //un hio para detectar la signals que se le envia
pthread_t hiloEscuchar; //un hilo para escuchar los msjs del server

//Variables del juego en si. (funcion jugar)
char* pokemonCapturado;
char* posicionPokenest;
t_queue* colaDeObjetivos;
enum_messages turno;

//Mutex
pthread_mutex_t turnoMutex;
pthread_mutex_t pokemonCapturadoMutex;

//Metadata
t_metadataEntrenador metadataEntrenador;
t_metadataMapa metadataMapa;

//Funciones
void crearArchivoMetadata(char *rutaMetadata);
void imprimirArray(char** array);
t_queue* parsearObjetivos(char** objetivos); //convierte un array de  strings con objetivos a una cola donde cada elemento es un objetivo
void jugar();
void recibirMsjs();
void copiarArchivos(char* archivoOrigen, char* archivoDestino);
void yoYaGane();

// Funciones de Conexion
int connectTo(enum_processes processToConnect, int *socketClient);
void enviarSimbolo(char* simbolo, int socketClient);
int reconectarse();

//Funciones de signal
void recibirSignal();
void sumarVida();
void restarVida();
void desconectarse();
void borrarArchivos(char* rutaDeleted);
void cerrarEntrenador();

//Chau Blanco
char* str_replace(const char *strbuf, const char *strold, const char *strnew)  ;
#endif /* ENTRENADOR_H_ */

//Funcion Limpiar entrenador
void limpiarColasMetadaEtrenador();
