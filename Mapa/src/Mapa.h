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
#include "metadata.h"
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <dirent.h>
#include <errno.h>
#include <tad_items.h>
#include <nivel.h>
#include <curses.h>

t_log* logMapa;
t_metadataMapa metadataMapa;
DIR *dipPokenest;
struct dirent *ditPokenest;
DIR *dipPokemones;
struct dirent *ditPokemones;
int semaforo_wait;
t_list* listaDePokenest;
t_list* items;
t_list* listaDeEntrenadores;
t_queue* colaDeListos;
t_queue* colaDeBloqueados;
pthread_mutex_t setFDmutex;
pthread_mutex_t setEntrenadoresMutex;
pthread_mutex_t colaDeListosMutex;
pthread_mutex_t colaDeBloqueadosMutex;
pthread_mutex_t itemsMutex;
pthread_mutex_t listaDePokenestMutex;
pthread_mutex_t setRecibirMsj;
pthread_mutex_t borradoDeEntrenadores;

// Estructuras
typedef struct {
	int socketServer;
	int socketClient;
	fd_set *masterFD;
} t_serverData;

typedef struct {
	int* pokemon;
	t_list* listaDePokemones;
	t_metadataPokenest metadata;
} t_pokenest;

typedef struct {
	int nivel;
	char id;
	char* tipo;
	char* nombre;
} t_pokemon;

typedef struct {
	char simbolo;
	int pos_x;
	int pos_y;
	int posD_x; //posicion deseada (a la que quiere ir)
	int posD_y;
	int socket; //filedescriptor del socket asociado al entrenador
	enum_messages accion; //accion que pretende hacer (conocer pokenest, moverse, etc)
	char pokemonD; //pokemon deseado por el entrenador;
	t_list* listaDePokemonesCapturados;
	int estaEnTurno;
	int seEstaMoviendo;
} t_entrenador;

// Funciones de conexion
void startServerProg();
void newClients(int *socketServer, fd_set *master, int *fdmax);
void handShake(void *parameter);
void processMessageReceived(void *parameter);

// Funciones
int recorrerdirDePokenest(char* rutaDirPokenest); //Se encarga de recorrer las carpetas que esta dentro de la pokenest (pikachu,bulbasaur,charmander,etc...)
int recorrerCadaPokenest(char* rutaDeUnaPokenest, char* nombreDelaPokenest); //Se encarga de recorrer lo que esta ADENTRO de las carpetas pokenest (pikachu001,pikachu002,metadata.dat,etc..)
t_metadataPokenest crearArchivoMetadataPokenest(char* rutaMetadataPokenest, char* nombreDeLaPokenest);
int levantarNivelDelPokemon(char* rutaDelPokemon);
void dibujarMapa();
void crearEntrenadorYDibujar(char simbolo, int socket);
bool igualarACaracterCondicion(void* paramatrer);
void eliminarEntrenador(char simbolo);
void planificar();
char* convertirPosicionesAString(int posX, int posY);
void moverEntrenador(t_entrenador* entrenador);

#endif /* MAPA_H_ */
