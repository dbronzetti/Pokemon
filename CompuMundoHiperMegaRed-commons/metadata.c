/*
 * metadata.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */
#include "metadata.h"

/*Defino esta funcion aca, ya que tanto el Entrenador como el Mapa van a tener que usarla asi no repetimos codigo*/
void crearArchivoMetadataDelMapa(char* rutaMetadataMapa, t_metadataMapa *metadataMapa, t_log *log){
	t_config* metadata;
	metadata = config_create(rutaMetadataMapa);

	metadataMapa->tiempoChequeoDeadlock = config_get_int_value(metadata, "TiempoChequeoDeadlock");
	metadataMapa->batalla = config_get_int_value(metadata, "Batalla");
	metadataMapa->algoritmo = config_get_string_value(metadata, "algoritmo");
	metadataMapa->quantum = config_get_int_value(metadata, "quantum");
	metadataMapa->retardo = config_get_int_value(metadata, "retardo");
	metadataMapa->ip = config_get_string_value(metadata, "IP");
	metadataMapa->puerto = config_get_int_value(metadata, "Puerto");

	log_info(log, "@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	log_info(log, "Tiempo de checkeo de deadlock: %d\n", metadataMapa->tiempoChequeoDeadlock);
	log_info(log, "Batalla: %d\n", metadataMapa->batalla);
	log_info(log, "Algoritmo: %s\n", metadataMapa->algoritmo);
	log_info(log, "Quantum: %d\n", metadataMapa->quantum);
	log_info(log, "Retardo: %d\n", metadataMapa->retardo);
	log_info(log, "IP: %s\n", metadataMapa->ip);
	log_info(log, "Puerto: %d\n", metadataMapa->puerto);
	log_info(log, "@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	free(metadata->properties);
	free(metadata->path);
	free(metadata);
}

