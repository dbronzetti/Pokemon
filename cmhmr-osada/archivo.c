/*
 * archivo.c

 *
 *  Created on: 3/10/2016
 *      Author: utnso
 */
#include <commons/collections/list.h>
#include <commons/string.h>

#include "osada.h"

unsigned char *osada;

void _iterarBloques(int bloque){
	printf("el proximo: %i\n", bloque);
}



t_list *crearPosicionesDeBloquesParaUnArchivo(int *arrayTabla, int numeroBloques){
	int elProximo = 0;
	t_list *proximo = list_create();

	list_add(proximo, numeroBloques);
	while ((elProximo = arrayTabla[numeroBloques]) != -1){
		list_add(proximo, elProximo);
		numeroBloques = elProximo;

	}

	list_iterate(proximo, (void*) _iterarBloques);

	return proximo;
}

osada_block_pointer devolverBloque(osada_file tablaDeArchivo, char *nombre){
	char *nac;
	char *n;
	nac = string_duplicate(&tablaDeArchivo.fname);
	n = string_duplicate(nombre);
	string_trim(&nac);
	string_trim(&n);

	if (tablaDeArchivo.state == REGULAR && strcmp(nac, n) == 0){
		free(nac);
		free(n);
		printf("state_: %c\n", tablaDeArchivo.state);
		printf("parent_directory_: %i\n", tablaDeArchivo.parent_directory);
		printf("fname_: %s\n",&tablaDeArchivo.fname);
		printf("file_size_: %i\n",tablaDeArchivo.file_size);
		printf("lastmod_: %i\n", tablaDeArchivo.lastmod);
		printf("first_block_: %i\n",tablaDeArchivo.first_block);

		return tablaDeArchivo.first_block;
	}

	free(nac);
	free(n);
	return -666;
}

osada_block_pointer buscarArchivo(osada_file *tablaDeArchivo, char *nombre){
	int pos=0;
	osada_block_pointer posicion = 0;

	for (pos=0; pos <= 2047; pos++){
		if ((posicion = devolverBloque(tablaDeArchivo[pos],  nombre)) != -666){
			printf("enbcontro>! , %i\n", pos);
			return posicion;
		};
	}
	return posicion;
}

