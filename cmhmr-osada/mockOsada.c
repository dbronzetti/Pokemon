/*
 * mockOsada.c
 *
 *  Created on: 23/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"
#include <errno.h>





void mock_Crear_La_Tabla_De_Asignacion_De_Los_Directorios(unsigned char *osada, int *tablaDeAsignacion, osada_header *osadaHeaderFile){
	int pos = 0;

	for (pos = 0; pos < tamanioQueOcupaLaTablaDeAsignacionEnBloques; pos++){
		tablaDeAsignacion[pos] = 6;
	}

	guardarEnOsada(osada, desdeParaTablaAsigancion, tablaDeAsignacion, tamanioTablaDeArchivos);

	//printf("Estado del munmap: %i\n", status);

}

void mock_Modificar_Los_Bloques_Del_Bitmap(unsigned char *osada, t_bitarray *bitMap, osada_header *osadaHeaderFile){
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	for (i=0; i < osadaHeaderFile->fs_blocks; i++){

		if(bitarray_test_bit(bitMap, i) == 0){
			//bloquesLibres++;
		}

		if(bitarray_test_bit(bitMap, i) == 1){
			//bloquesOcupados++;
			bitarray_set_bit(bitMap, i);
		}

	}

	guardarEnOsada(osada, desdeParaBitmap, bitMap, tamanioDelBitMap);
}

void mock_Crear_Directorios(unsigned char *osada, osada_header *osadaHeaderFile){
		/*TABLA DE ARCHIVO*/
		printf("creaDirectorios\n");
		char dir[5];
		char dir2[5];
		int k=0;

		osada_file *tablaDeArchivo = malloc(tamanioTablaDeArchivos);
		printf("tamanio: %i\n",tamanioTablaDeArchivos);

		//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
		memcpy(tablaDeArchivo, &osada[desdeParaTablaDeArchivos], tamanioTablaDeArchivos);

	 	for (k=0; k <= 2047; k++){

	 		strcpy(dir, "joel");
	 		sprintf(dir2, "%d",k);
	 		strcat(dir, dir2);

	 		tablaDeArchivo[k].state = DIRECTORY;
	 		tablaDeArchivo[k].parent_directory = k;
	 		memcpy(tablaDeArchivo[k].fname, dir, 17);
	 		tablaDeArchivo[k].file_size = 0;
	 		tablaDeArchivo[k].lastmod = 0;
	 		tablaDeArchivo[k].first_block= 0;

	 		//tablaDeArchivo[k] = updatearTablaDeArchivos();
			//memcpy(tablaDeArchivo[k], updatearTablaDeArchivos(), sizeof(osada_file));

		}

	 	//memcpy(&osada[64 + sizeOFDelBitMap], tablaDeArchivo, 2048*sizeof(osada_file));

		/*CERRAR TODO*/
		//printf("archivoID: %i\n", archivoID);
		printf("tamanio mockOsada: %i\n", tamanioDelArchivoOSADAEnBytes);

		//int status = munmap(osada, tamanioDelArchivo);
		guardarEnOsada(osada, desdeParaTablaDeArchivos, tablaDeArchivo, tamanioTablaDeArchivos);


		//printf("Estado del munmap: %i\n", status);
		printf("ERROR: %s\n", strerror(errno));

}

void mock_setearConstantesDePosicionDeOsada(osada_header *osadaHeaderFile){

	tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	tamanioDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;
	tamanioQueOcupaLaTablaDeAsignacionEnBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	tamanioQueOcupaElBloqueDeDatos = OSADA_BLOCK_SIZE* osadaHeaderFile->data_blocks;

	desdeParaBitmap = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	desdeParaTablaDeArchivos = OSADA_BLOCK_SIZE + tamanioDelBitMap;
	desdeParaTablaAsigancion = tamanioQueOcupaElHeader + tamanioDelBitMap + tamanioTablaDeArchivos;
	desdeParaBloqueDeDatos = tamanioQueOcupaElHeader + tamanioDelBitMap + tamanioTablaDeArchivos + tamanioQueOcupaLaTablaDeAsignacion;

}

void mock_setearTamanioDelArchivo(int tamanioDelArchivo ){
	tamanioDelArchivoOSADAEnBytes = tamanioDelArchivo ;
}
