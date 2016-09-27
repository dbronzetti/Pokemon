/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <errno.h>

char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile){

	int tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	int tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;

	int desde = tamanioQueOcupaElHeader + tamanioDelBitMapa + tamanioTablaDeArchivos + tamanioQueOcupaLaTablaDeAsignacion;

	int tamanioQueOcupaElBloqueDeDatos = OSADA_BLOCK_SIZE* osadaHeaderFile->data_blocks;

	//unsigned char *bloqueDeDatos = malloc(sizeof(char) * osadaHeaderFile->data_blocks);OLD
	unsigned char *bloqueDeDatos = malloc(sizeof(char) * OSADA_BLOCK_SIZE * osadaHeaderFile->data_blocks);

	memcpy(bloqueDeDatos, &osada[desde], tamanioQueOcupaElBloqueDeDatos );
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
	int numeroBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	int tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;

	int *arrayTabla = malloc(sizeof(int) * numeroBloques * OSADA_BLOCK_SIZE);

	int tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	int desde = tamanioQueOcupaElHeader + tamanioDelBitMapa + tamanioTablaDeArchivos;

	memcpy(arrayTabla, &osada[desde], tamanioQueOcupaLaTablaDeAsignacion );
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
	osada_file *tablaDeArchivo = malloc(2048*sizeof(osada_file));
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioQueOcupaLaTablaDeArchivos = 2048*sizeof(osada_file);
	int desde = OSADA_BLOCK_SIZE + tamanioDelBitMapa;

	//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
	memcpy(tablaDeArchivo, &osada[desde], tamanioQueOcupaLaTablaDeArchivos);

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
	int tamanioQueOcupaElBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int desde = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER

	unBitMapSinFormato = malloc(tamanioQueOcupaElBitMapa );
	memcpy(unBitMapSinFormato, &osada[desde], tamanioQueOcupaElBitMapa );
	bitMap = bitarray_create(unBitMapSinFormato, tamanioQueOcupaElBitMapa );

	contarBloques(osada, osadaHeaderFile, bitMap);

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

	//mostrarHeader(osadaHeaderFile);

	return osadaHeaderFile;

}

int obtenerTamanioDelArchivo(int archivoID){
	struct stat buffer;
	fstat(archivoID, &buffer);
	return buffer.st_size;
}

int obtenerIDDelArchivo(char *ruta){
	printf("ruta: %s\n", ruta);
	return open(ruta, O_RDWR, (mode_t)0777);
}

unsigned char *inicializarOSADA(int archivoID, int tamanio){
	unsigned char *osada;

	/************************************************************/
	printf("ERROR: %s\n", strerror(errno));
	printf("archivoID: %i\n", archivoID);
	printf("tamanio: %i\n", tamanio);
	/************************************************************/

	osada = mmap(0, tamanio, PROT_READ|PROT_WRITE,MAP_SHARED, archivoID, 0);
	int statusCerrar = close(archivoID);
	return osada;

}


