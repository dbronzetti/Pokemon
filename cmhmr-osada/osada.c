/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <errno.h>
#include <commons/collections/list.h>

void guardarEnOsada(unsigned char *osada, int desde, void *elemento, int tamaniaDelElemento){
	memcpy(&osada[desde], elemento, tamaniaDelElemento );
	int status = munmap(osada, tamaniaDelElemento);

	if (status == -1)
		printf("Estado del munmap: %i\n", status);
}


/*********************************************************************/

unsigned char *osada;
static int dataBlocks;

void _iterarParaVerContenido(int bloque){

	char *bloqueDeDatos = malloc(OSADA_BLOCK_SIZE);
	int bloque2 = bloque *64;
	int i;
	//tamanioQueOcupaElBloqueDeDatos ir de atras con los bloques.
	//printf("%i\n", dataBlocks);
	memcpy(bloqueDeDatos, &osada[dataBlocks+bloque2], OSADA_BLOCK_SIZE );

//	for(i=1; i<=64; i++){
		printf("%s", bloqueDeDatos);
	//}
	//printf("\nTERMINO\n");
	free(bloqueDeDatos);



}

void verContenidoDeArchivo(t_list *conjuntoDeBloques, unsigned char *osada2){
	osada = osada2;
	list_iterate(conjuntoDeBloques, (void*) _iterarParaVerContenido);
}

/*********************************************************************/




char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile){
	//unsigned char *bloqueDeDatos = malloc(sizeof(char) * osadaHeaderFile->data_blocks);OLD
	unsigned char *bloqueDeDatos = malloc(sizeof(char) * OSADA_BLOCK_SIZE * osadaHeaderFile->data_blocks);

	memcpy(bloqueDeDatos, &osada[desdeParaBloqueDeDatos], tamanioQueOcupaElBloqueDeDatos );
	return bloqueDeDatos;
}

void mostarAsignacion(int asignado){
	printf("Array tabla asignada: %i\n",asignado);
}

void mostrarTodosLosAsignados(int *arrayTabla, int numeroBloques){
	int pos = 0;

	for (pos = 0; pos < numeroBloques; pos++){
		mostarAsignacion(arrayTabla[pos]);
	}
}

int *obtenerTablaDeAsignacion(unsigned char *osada, osada_header *osadaHeaderFile){
	int *arrayTabla = malloc(tamanioQueOcupaLaTablaDeAsignacion);

	memcpy(arrayTabla, &osada[desdeParaTablaAsigancion], tamanioQueOcupaLaTablaDeAsignacion );

	//mostrarTodosLosAsignados(arrayTabla, numeroBloques);

	return arrayTabla;
}

void mostrarTodaLaTablaDeArchivos(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarStructDeArchivos(tablaDeArchivo[pos], pos);
	}
}

void mostrarStructDeArchivos(osada_file tablaDeArchivo, int pos){
	printf("Empieza: %i****************\n",pos);
	printf("state_%i: %c\n",pos, tablaDeArchivo.state);
	printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
	printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
	printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
	printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
	printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
	printf("Termina: %i****************\n",pos);
}

osada_file *obtenerTablaDeArchivos(unsigned char *osada, osada_header *osadaHeaderFile){
	osada_file *tablaDeArchivo = malloc(tamanioTablaDeArchivos);

	//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
	memcpy(tablaDeArchivo, &osada[desdeParaTablaDeArchivos], tamanioTablaDeArchivos);

	//mostrarTodaLaTablaDeArchivos(tablaDeArchivo);

	return tablaDeArchivo;
}

void contarBloques(unsigned char *osada, osada_header *osadaHeaderFile, t_bitarray *bitMap){
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	for (i=0; i < osadaHeaderFile->fs_blocks; i++){//para 150k

		if(bitarray_test_bit(bitMap, i) == 0){
			bloquesLibres++;
		}

		if(bitarray_test_bit(bitMap, i) == 1){
			bloquesOcupados++;
		}

	}
	printf("Bloques Ocupados: %i\n",bloquesOcupados);
	printf("Bloques Libres: %i\n",bloquesLibres);

}

t_bitarray *obtenerBitmap(unsigned char *osada, osada_header *osadaHeaderFile){
	t_bitarray *bitMap;
	unsigned char *unBitMapSinFormato;

	unBitMapSinFormato = malloc(tamanioDelBitMap );
	memcpy(unBitMapSinFormato, &osada[desdeParaBitmap], tamanioDelBitMap );
	bitMap = bitarray_create(unBitMapSinFormato, tamanioDelBitMap );

	//contarBloques(osada, osadaHeaderFile, bitMap);

	return bitMap;

}

void mostrarHeader(osada_header *osadaHeaderFile){
	printf("magic_number 2: %s\n",  osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %s\n",   osadaHeaderFile->padding);
}

osada_header *obtenerHeader(unsigned char *osada){
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	memcpy(osadaHeaderFile, osada, OSADA_BLOCK_SIZE);


	mostrarHeader(osadaHeaderFile);

	return osadaHeaderFile;

}

/*SETEA Y GUARDA EN LA VARIABLE GLOBAL, ADEMAS SI SE NECESITA SE DEVUELVE EL TAMAÑO PARA USARLOS EN MOCKS*/
int setearTamanioDelArchivo(int archivoID){
	struct stat buffer;
	fstat(archivoID, &buffer);
	tamanioDelArchivoOSADAEnBytes = buffer.st_size;
	return tamanioDelArchivoOSADAEnBytes;
}

int obtenerIDDelArchivo(char *ruta){
	printf("ruta: %s\n", ruta);
	return open(ruta, O_RDWR, (mode_t)0777);
}
void setearConstantesDePosicionDeOsada(osada_header *osadaHeaderFile){
	tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	tamanioDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;
	tamanioQueOcupaLaTablaDeAsignacionEnBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	tamanioQueOcupaElBloqueDeDatos = OSADA_BLOCK_SIZE* osadaHeaderFile->data_blocks;
	dataBlocks= (osadaHeaderFile->fs_blocks - osadaHeaderFile->data_blocks)*64;
	//dataBlocks=  osadaHeaderFile->allocations_table_offset + tamanioQueOcupaLaTablaDeAsignacionEnBloques;

	printf("osadaHeaderFile->fs_blocks - osadaHeaderFile->data_blocks: %i\n",osadaHeaderFile->fs_blocks - osadaHeaderFile->data_blocks);
	printf("dataBlocks: %i\n",dataBlocks);

	desdeParaBitmap = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	desdeParaTablaDeArchivos = OSADA_BLOCK_SIZE + tamanioDelBitMap;
	desdeParaTablaAsigancion = tamanioQueOcupaElHeader + tamanioDelBitMap + tamanioTablaDeArchivos;
	desdeParaBloqueDeDatos = tamanioQueOcupaElHeader + tamanioDelBitMap + tamanioTablaDeArchivos + tamanioQueOcupaLaTablaDeAsignacion;
	printf("desdeParaTablaAsigancion: %i\n",desdeParaTablaAsigancion);
	printf("desdeParaBloqueDeDatos: %i\n",desdeParaBloqueDeDatos);
}


unsigned char *inicializarOSADA(int archivoID){
	unsigned char *osada;

	/************************************************************/
	printf("Que paso?: %s\n", strerror(errno));
	printf("archivoID: %i\n", archivoID);
	printf("tamanio: %i\n", tamanioDelArchivoOSADAEnBytes);
	/************************************************************/


	osada = mmap(0, tamanioDelArchivoOSADAEnBytes, PROT_READ|PROT_WRITE,MAP_SHARED, archivoID, 0);
	int statusCerrar = close(archivoID);
	return osada;

}


