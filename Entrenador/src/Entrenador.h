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

//Logger
t_log* logEntrenador;

// Funciones de Conexion
int connectTo(enum_processes processToConnect, int *socketClient);


#endif /* ENTRENADOR_H_ */
