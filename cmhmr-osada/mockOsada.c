/*
 * mockOsada.c
 *
 *  Created on: 23/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"
#include <errno.h>


osada_file ingresarValoresAOsadaFile(int pd, char *fileName){
	osada_file tablaDeArchivo;

	tablaDeArchivo.state = DIRECTORY;
	tablaDeArchivo.parent_directory = pd;
	memcpy(tablaDeArchivo.fname, fileName, 17);

	return tablaDeArchivo;
}

osada_file updatearTablaDeArchivos(){
	return ingresarValoresAOsadaFile(8,'joel');
}

/*
void mock_guardar(unsigned char *osada, int desde, void *elemento, int tamaniaDelElemento){
	memcpy(&osada[desde], elemento, tamaniaDelElemento );
	int status = munmap(osada, tamaniaDelElemento);

	if (status == -1)
		printf("Estado del munmap: %i\n", status);
}
*/

void mock_Crear_La_Tabla_De_Asignacion_De_Los_Directorios(unsigned char *osada, int *tablaDeAsignacion, int tamanioDelArchivo, osada_header *osadaHeaderFile){
	int numeroBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	int pos = 0;

	int tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	int desde = tamanioQueOcupaElHeader + tamanioDelBitMapa + tamanioTablaDeArchivos;


	for (pos = 0; pos < numeroBloques; pos++){
		tablaDeAsignacion[pos] = 6;
	}

	guardarEnOsada(osada, desde, tablaDeAsignacion, tamanioTablaDeArchivos);

	//printf("Estado del munmap: %i\n", status);

}

void mock_Modificar_Los_Bloques_Del_Bitmap(unsigned char *osada, t_bitarray *bitMap , int tamanioDelArchivo, osada_header *osadaHeaderFile){
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;
	int tamanioQueOcupaElBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int desde = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER

	for (i=0; i < osadaHeaderFile->fs_blocks; i++){

		if(bitarray_test_bit(bitMap, i) == 0){
			//bloquesLibres++;
		}

		if(bitarray_test_bit(bitMap, i) == 1){
			//bloquesOcupados++;
			bitarray_set_bit(bitMap, i);
		}

	}

	guardarEnOsada(osada, desde, bitMap, tamanioQueOcupaElBitMapa);
}

void mock_Crear_Directorios(unsigned char *osada, osada_header *osadaHeaderFile, int tamanioDelArchivo){
		/*TABLA DE ARCHIVO*/
		printf("creaDirectorios\n");
		char dir[5];
		char dir2[5];
		int k=0;
		int sizeOFDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;

		osada_file *tablaDeArchivo = malloc(2048*sizeof(osada_file));

		//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
		memcpy(tablaDeArchivo, &osada[64 + sizeOFDelBitMap], 2048*sizeof(osada_file));


	 	for (k=0; k <= 2047; k++){

	 		strcpy(dir, "dir2");
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
		printf("tamanio: %i\n", tamanioDelArchivo);

		//int status = munmap(osada, tamanioDelArchivo);
		guardarEnOsada(osada, 64 + sizeOFDelBitMap, tablaDeArchivo, 2048 * sizeof(osada_file));


		//printf("Estado del munmap: %i\n", status);
		printf("ERROR: %s\n", strerror(errno));

}


