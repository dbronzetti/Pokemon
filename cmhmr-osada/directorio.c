/*
 * directorio.c
 *
 *  Created on: 16/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

//Desarrollar las funciones para leer el contenido del árbol de directorios del filesystem OSADA.
//comprobar cual es root
//

//LEER LA SECUENCIA DE BLOQUES
/*
 * A - LEER first_block
 * B - IR A TABLA ASIGANACION Y LEER EL BLOQUE
 * C - EL BLOQUE NOS DA EL PROXIMO BLOQUE A LEER(POSICIONES DEL ARRAY PONELE) Y EL FIN ES FFFFFFF
 * D - SUPONGO QUE ESOS BLOQUE SON LAS POSICIONES DEL BLOQUE DATOS
 *
 *
 * */
/****************LISTAR TODO *************************************************/
void mostrarLosDirectorios(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DIRECTORY){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}

void mostrarLosRegulares(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == REGULAR){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}

void mostrarLosBorrados(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DELETED){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}


void dameTodosLosDirectorios(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarLosDirectorios(tablaDeArchivo[pos], pos);
	}
}

void dameTodosLosArchivosRegulares(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarLosRegulares(tablaDeArchivo[pos], pos);
	}
}

void dameTodosLosBorrados(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarLosBorrados(tablaDeArchivo[pos], pos);
	}
}
/****************FIN LISTAR TODO *************************************************/
//UN DICCIONARIO CON LISTAS PARA REPRESENTAR JERARQUIAS.
int iterator_count=0;

void _assertion(char* key,t_list *datos) {
	//osada_file osadaFile = malloc(64);
	//osada_file osadaFile;
	//osadaFile = (osada_file)list_get(datos, 0);
	printf("Diccionario: %s\n", key);
	//printf("list: %c\n", osadaFile);
	//free(osadaFile);
}

void reconocerDirectorio(osada_file tablaDeArchivo, int pos, t_dictionary *dictionary){
	if (tablaDeArchivo.state == DIRECTORY  && tablaDeArchivo.parent_directory == 65535){
		t_list *list = list_create();
		 list_add(list, &tablaDeArchivo);
		 list_get(list, 0);
		 //list_add(list, "test");
		dictionary_put(dictionary, tablaDeArchivo.fname , list);

	}
}

void crearArbolDeDirectorios(osada_file *tablaDeArchivo){
	t_dictionary *dictionary = dictionary_create();
	int pos=0;

	for (pos=0; pos <= 2047; pos++){
		reconocerDirectorio(tablaDeArchivo[pos], pos, dictionary);
	}

	dictionary_iterator(dictionary, (void*) _assertion);

	t_list *list = dictionary_get(dictionary, "Viridian City");

	//list_add(list, "Tew");

	//dictionary_put(dictionary, "directorio", list);
	//list = dictionary_get(dictionary, "directorio");
	//osada_file *nombre = list_get(list, 0);
	//char *nombre = list_get(list, 0);
	//->
	//printf("ver contenido de: %s\n", &nombre->fname);
	//printf("ver contenido de: %s\n", nombre);

	printf("cantidad de padres %i\n", iterator_count);

}

