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

void _recorrerDirectoriosPadres(char* key,t_list *datos) {

	osada_file *archivo =  malloc(sizeof(osada_file));
	archivo = list_get(datos, 0);
	printf("Diccionario - Carpeta en el root: %s\n", key);
	printf("Un elemento de la lista list, que seria el primer hijo del root: %s\n", archivo->fname);
	printf("archivo->parent_directory: %i\n", archivo->parent_directory);

	//free(archivo);

}

void reconocerDirectorio(osada_file *archivo, int pos, t_dictionary *dictionary){

	t_list *list = list_create();

	if (archivo->state == DIRECTORY  && archivo->parent_directory == 65535){
		char str[10];
		sprintf(str, "%d", pos);
		list_add(list, archivo);

		//printf("pos: %s", str);

		//dictionary_put(dictionary, (char *)archivo->fname , list);
		dictionary_put(dictionary, str , list);

	}

}

void reconocerDirectorioHijos(osada_file *archivo, int pos, t_dictionary *dictionary, t_dictionary *dictionaryDirRoot){

	t_list *list = list_create();

	if (archivo->state == DIRECTORY){
		char str[10];
		sprintf(str, "%d", pos);
		list_add(list, archivo);

		//printf("pos: %s", str);

		//dictionary_put(dictionary, (char *)archivo->fname , list);
		dictionary_put(dictionary, str , list);

	}

}

t_dictionary *crearArbolDeDirectoriosDelRoot(osada_file *tablaDeArchivo){
	t_dictionary *dictionaryDirRoot = dictionary_create();
	int pos=0;

	for (pos=0; pos <= 2047; pos++){
		reconocerDirectorio(&tablaDeArchivo[pos], pos, dictionaryDirRoot);
	}

	dictionary_iterator(dictionaryDirRoot, (void*) _recorrerDirectoriosPadres);

	return dictionaryDirRoot;
}

t_dictionary *crearArbolDeDirectoriosHijos(osada_file *tablaDeArchivo, t_dictionary *dictionaryDirRoot){
	t_dictionary *dictionaryDirHijos = dictionary_create();
	int pos=0;

	for (pos=0; pos <= 2047; pos++){
		reconocerDirectorioHijos(&tablaDeArchivo[pos], pos, dictionaryDirHijos, dictionaryDirRoot);
	}

	dictionary_iterator(dictionaryDirHijos, (void*) _recorrerDirectoriosPadres);

	return dictionaryDirHijos;
}

