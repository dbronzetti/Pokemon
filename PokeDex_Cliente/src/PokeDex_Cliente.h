/*
 * PokeDex_Cliente.h
 *
 *  Created on: 30/9/2016
 *      Author: utnso
 */

#ifndef POKEDEX_CLIENTE_H_
#define POKEDEX_CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"
#include "commons/log.h"
#include "commons/config.h"
#include "metadata.h"

int PORT;
char *IP_SERVER;

//Logger
t_log* logPokeCliente;

// Funciones de Conexion
int connectTo(enum_processes processToConnect, int *socketClient);

#endif /* POKEDEX_CLIENTE_H_ */
