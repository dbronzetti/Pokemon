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

	for (pos = 0; pos < TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES; pos++){
		tablaDeAsignacion[pos] = 6;
	}

	guardarEnOsada(osada, DESDE_PARA_TABLA_ASIGNACION , tablaDeAsignacion, TAMANIO_TABLA_DE_ARCHIVOS);

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

	guardarEnOsada(osada, DESDE_PARA_BITMAP, bitMap, TAMANIO_DEL_BITMAP);
}

void mock_Crear_Directorios(unsigned char *osada, osada_header *osadaHeaderFile){
		/*TABLA DE ARCHIVO*/
		printf("creaDirectorios\n");
		char dir[5];
		char dir2[5];
		int k=0;

		osada_file *tablaDeArchivo = malloc(TAMANIO_TABLA_DE_ARCHIVOS);
		printf("tamanio: %i\n",TAMANIO_TABLA_DE_ARCHIVOS);

		//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
		memcpy(tablaDeArchivo, &osada[DESDE_PARA_TABLA_DE_ARCHIVOS ], TAMANIO_TABLA_DE_ARCHIVOS);

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
		printf("tamanio mockOsada: %i\n", TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES);

		//int status = munmap(osada, tamanioDelArchivo);
		guardarEnOsada(osada, DESDE_PARA_TABLA_DE_ARCHIVOS , tablaDeArchivo, TAMANIO_TABLA_DE_ARCHIVOS);


		//printf("Estado del munmap: %i\n", status);
		printf("ERROR: %s\n", strerror(errno));

}

void mock_setearConstantesDePosicionDeOsada(osada_header *osadaHeaderFile){

	TAMANIO_QUE_OCUPA_EL_HEADER = OSADA_BLOCK_SIZE;
	TAMANIO_DEL_BITMAP = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	TAMANIO_TABLA_DE_ARCHIVOS =  2048 * sizeof(osada_file);
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS = OSADA_BLOCK_SIZE* osadaHeaderFile->data_blocks;

	DESDE_PARA_BITMAP = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	DESDE_PARA_TABLA_DE_ARCHIVOS  = OSADA_BLOCK_SIZE + TAMANIO_DEL_BITMAP;
	DESDE_PARA_TABLA_ASIGNACION  = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS;
	DESDE_PARA_BLOQUE_DE_DATOS = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS + TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION;

}

void mock_setearTamanioDelArchivo(int tamanioDelArchivo ){
	TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES = tamanioDelArchivo ;
}
