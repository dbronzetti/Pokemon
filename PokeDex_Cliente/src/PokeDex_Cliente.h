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
int socketPokeServer = 0;

//Logger
t_log* logPokeCliente;

// Funciones de Conexion
int connectTo(enum_processes processToConnect, int *socketClient);
t_list * obtenerDirectorio(const char* path, enum_FUSEOperations fuseOperation);

#endif /* POKEDEX_CLIENTE_H_ */

//Librerias y estructuras pra el FUSE
#include <fuse/fuse.h>
#include <fuse/fuse_common.h>
#include <fuse/fuse_compat.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>

#define FUSE_USE_VERSION 26

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
#define DIRECTORIO_RAIZ "/"

//#include "cache.c"

static char initial_working_dir[PATHLEN_MAX+1] ={ '\0' };
static char cached_mountpoint[PATHLEN_MAX+1] ={ '\0' };
static int save_dir;

#ifdef NEVER

static char *local=".";
static inline const char *rel(const char *path)
{
  if(strcmp(path,"/")==0)
    return local;
  else
    return (path+1);
}
#endif

const char *full(const char *path); /* Anade un punto de montaje */;//Esto da un warning no importa!

// Fin de cosillas para el FUSE
