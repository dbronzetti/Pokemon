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

//Librerias y estructuras pra el FUSE
#include <asm-generic/errno-base.h>
#include <bits/socket_type.h>
#include <fuse/fuse.h>
#include <fuse/fuse_compat.h>
#include <netinet/in.h>
#include <stddef.h>
#include <sys/stat.h>

#define FUSE_USE_VERSION 26
#define DIRECTORIO_RAIZ "/"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux

/* para pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#define PATHLEN_MAX 1024


//#include "cache.c"

static char initial_working_dir[PATHLEN_MAX+1] ={ '\0' };
static char cached_mountpoint[PATHLEN_MAX+1] ={ '\0' };
static int save_dir;

#ifdef NEVER
//TODO ver que pasa aca
#endif

static char *local=".";
// Fin de cosillas para el FUSE
