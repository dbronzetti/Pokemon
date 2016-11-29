/*
 * Mapa.h
 *
 *  Created on: 31/8/2016
 *      Author: utnso
 */

#ifndef MAPA_H_
#define MAPA_H_

#include "sockets.h"
#include "metadata.h"
#include <pthread.h>
#include <semaphore.h>
#include <curses.h>
#include <dirent.h>
#include <tad_items.h>
#include <nivel.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include <pkmn/battle.h>
#include <pkmn/factory.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/config.h>

//Variables
t_log* logMapa;
char *mapa;
t_metadataMapa metadataMapa;
DIR *dipPokenest;
struct dirent *ditPokenest;
DIR *dipPokemones;
struct dirent *ditPokemones;
t_list* listaDePokenest;
t_list* items;
t_list* listaDeEntrenadores;
t_list* entrenadoresDeadLock;
t_queue* colaDeListos;
t_queue* colaDeBloqueados;
pthread_mutex_t setFDmutex;
pthread_mutex_t setEntrenadoresMutex;
pthread_mutex_t colaDeListosMutex;
pthread_mutex_t colaDeBloqueadosMutex;
pthread_mutex_t itemsMutex;
pthread_mutex_t listaDePokenestMutex;
pthread_mutex_t setRecibirMsj;
pthread_mutex_t metadataMutex;
sem_t processNextMessageSem;
sem_t borradoDePersonajesSem;
int SEM_INIT_VALUE = 1;//This is for receiving the 1st trainer


char* rutaMetadata;//needed as global for reloading metadata file
char entrenadorAnterior;
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
} t_pokemones;

typedef struct {
	char simbolo;
	time_t timeIngreso;
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
	int distancia;
	int	seMovioEnX;
	int estaBloqueado;
} t_entrenador;

typedef struct {
	char entrenador;
	t_list* pokemonesAsignados;	//esta lista contendra elementos del tipo t_pokemones_Asignacion
} t_entrenador_Asignacion;

typedef struct {
	char pokemon_id;
	int cantidad;
} t_pokemones_Asignacion;

// Funciones de conexion
void startServerProg();
void newClients(int *socketServer, fd_set *master, int *fdmax);
void handShake(void *parameter);
void processMessageReceived(void *parameter);

// Funcion de mapa que la lib no traia

// Funciones
int recorrerdirDePokenest(char* rutaDirPokenest);
int recorrerCadaPokenest(char* rutaDeUnaPokenest, char* nombreDelaPokenest);
t_metadataPokenest crearArchivoMetadataPokenest(char* rutaMetadataPokenest, const char* nombreDeLaPokenest);
int levantarNivelDelPokemon(char* rutaDelPokemon);
void dibujarMapa();
void crearEntrenadorYDibujar(char simbolo, int socket);
void eliminarEntrenador(char simbolo);
void devolverPokemones(t_list* pokemones);
void sumarRecurso(t_list* items, char id);
void planificar();
void planificarRR();
void ejecutarAccionEntrenador(t_entrenador* entrenador, int* i);
bool existePokenest(char idPokemon);
char* convertirPosicionesAString(int posX, int posY);
void moverEntrenador(int* pos_x, int* pos_y, int posD_x, int posD_y, int* seMovioEnX);
void planificarSRDF();
void ordenarColaEntrenadores();
void calcularCantidadMovimientos(t_entrenador* entrenador);
void detectarDeadlocks();
t_list* detectarInterbloqueo();
void cargarListaAsignacion(t_list *asignacion);
void cargarPokemonesExistentes(t_list *pokemonesList);
void cargarPokeNests(t_list *pokemonesAsignados, t_list* pokemonesList);
void cargarListaSolicitud(t_list *solicitud);
void cargarEntrenadores(t_list *entrenadores);
void quitarEntrenadoresSinAsignacion(t_list *asignacion, t_list *entrenadoresNoBloqueados);
void cargarCantidadPokemonesExistentes(t_list *pokemonesList);
t_pokemones* dameTuMejorPokemon(t_entrenador* entrenador);
void matar(t_entrenador* entrenador);
void recibirSignal();
void reloadMetadata();
void evaluarEstadoEntrenador(t_entrenador* entrenador); //evalua en que estado se encuentra y segun su estado le indica que tiene que hacer

#endif /* MAPA_H_ */
