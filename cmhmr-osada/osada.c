/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <errno.h>

char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile){
	printf("!aca\n");
	int tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	int tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;
	printf("!aca\n");
	int desde = tamanioQueOcupaElHeader + tamanioDelBitMapa + tamanioTablaDeArchivos + tamanioQueOcupaLaTablaDeAsignacion;

	int tamanioQueOcupaElBloqueDeDatos = OSADA_BLOCK_SIZE* osadaHeaderFile->data_blocks;
	unsigned char *bloqueDeDatos = malloc(sizeof(char) * osadaHeaderFile->data_blocks);
	//¿CUANTOS BLOQUE DE ADTOS TENGO QUE LEER?
	//¿NO TENGO QUE HACER 64 del puntero del bloque * cantidadDeBloques?

	//memcpy(bloqueDeDatos, &osada[desde], tamanioQueOcupaElBloqueDeDatos );
	return bloqueDeDatos;
}

void mostarAsignacion(int asignado){
	printf("Array tabla asignada: %i\n",asignado);
}

void mostrarTodosLosAsignados(int *arrayTabla, int numeroBloques){
	int pos = 0;

	for (pos; pos < numeroBloques; pos++){
		mostarAsignacion(arrayTabla[pos]);
	}
}

int *obtenerTablaDeAsignacion(unsigned char *osada, osada_header *osadaHeaderFile){
	int numeroBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 /64;
	int tamanioQueOcupaLaTablaDeAsignacion = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4;

	int *arrayTabla = malloc(sizeof(int) * numeroBloques);
	int tamanioQueOcupaElHeader = OSADA_BLOCK_SIZE;
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int tamanioTablaDeArchivos =  2048 * sizeof(osada_file);
	int desde = tamanioQueOcupaElHeader + tamanioDelBitMapa + tamanioTablaDeArchivos;

	memcpy(arrayTabla, &osada[desde], tamanioQueOcupaLaTablaDeAsignacion );
	//mostrarTodosLosAsignados(arrayTabla, numeroBloques);
	return arrayTabla;
}

void mostrarTablaDeArchivos(osada_file tablaDeArchivo,int pos){
	printf("Empieza: %i****************\n",pos);
	printf("state_%i: %c\n",pos, tablaDeArchivo.state);
	printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
	printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
	printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
	printf("fname_%i: %s\n",pos, &tablaDeArchivo.lastmod);
	printf("file_size_%i: %i\n",pos, tablaDeArchivo.first_block);
	printf("Termina: %i****************\n",pos);
}

osada_file *obtenerTablaDeArchivos(unsigned char *osada, osada_header *osadaHeaderFile){
	osada_file *tablaDeArchivo = malloc(2048*sizeof(osada_file));
	int tamanioDelBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int pos=0;
	int tamanioQueOcupaLaTablaDeArchivos = 2048*sizeof(osada_file);
	int desde = OSADA_BLOCK_SIZE + tamanioDelBitMapa;

	//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
	memcpy(tablaDeArchivo, &osada[desde], tamanioQueOcupaLaTablaDeArchivos);
 	for (pos=0; pos <= 2047; pos++){
		//mostrarTablaDeArchivos(tablaDeArchivo[pos],pos);
	}

 	return tablaDeArchivo;
}

t_bitarray *obtenerBitmap(unsigned char *osada, osada_header *osadaHeaderFile){
	t_bitarray *bitMap;
	unsigned char *unBitMapSinFormato;
	int tamanioQueOcupaElBitMapa = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	int i = 0;
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int desde = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER

	unBitMapSinFormato = malloc(tamanioQueOcupaElBitMapa );
	memcpy(unBitMapSinFormato, &osada[desde], tamanioQueOcupaElBitMapa );
	bitMap = bitarray_create(unBitMapSinFormato, tamanioQueOcupaElBitMapa );//para 150k

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

	return bitMap;

}

osada_header *obtenerHeader(unsigned char *osada){
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	memcpy(osadaHeaderFile, osada, OSADA_BLOCK_SIZE);

	printf("magic_number 2: %s\n",  osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %s\n",   osadaHeaderFile->padding);

	return osadaHeaderFile;

}

unsigned char *inicializarOSADA(char *ruta){
	struct stat buffer;
	unsigned char *osada;
	int archivoID = open(ruta ,O_RDWR,(mode_t)0777);


	fstat(archivoID, &buffer);
	int tamanio = buffer.st_size;

	/************************************************************/
	printf("ruta: %s\n", ruta);
	printf("ERROR: %s\n", strerror(errno));
	printf("archivoID: %i\n", archivoID);
	printf("tamanio: %i\n", tamanio);
	/************************************************************/

	osada = mmap(0, tamanio, PROT_READ|PROT_WRITE,MAP_SHARED, archivoID, 0);
	int statusCerrar = close(archivoID);
	return osada;

}

void mockInicializarOSADA(){

	caddr_t mmap_ptr;
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco-mediano.bin";
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	unsigned char *osada;
	t_bitarray *bitMap;
	unsigned char *unBitMapSinFormato;

	struct stat buf;


	int fid	= open(ruta ,O_RDWR,(mode_t)0777);
	int pagesize = getpagesize();

	fstat(fid, &buf);
	int size = buf.st_size;

	printf("El Archivo pesa: %i\n", size);
	printf("sizeof(osada_header): %i\n",  sizeof(osada_header));

	if(fid<0){
		fprintf(stderr,"Bad Open of file <%s>\n",ruta);
		error("Failed to open mmap file, QUIT!");
	}

	osada = mmap(0, size, PROT_READ|PROT_WRITE,MAP_SHARED, fid, 0);
	printf("ERROR: %s\n", strerror(errno));
	printf("error bit 1: %i\n", osada);
	/* Offset in page frame */
	memcpy(osadaHeaderFile, osada, 64);

	printf("magic_number 2: %s\n",  osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %s\n",   osadaHeaderFile->padding);

	//osadaHeaderFile->version = 6;
	printf("version: %i\n", osadaHeaderFile->version);
	memcpy(osada, osadaHeaderFile, 64);

	/*BIT MAP*/


	//REPRESENTA EL MAPA DEL BITMAP
	int sizeOFDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
	unBitMapSinFormato= malloc(sizeOFDelBitMap);
	memcpy(unBitMapSinFormato, &osada[64], sizeOFDelBitMap);
	bitMap = bitarray_create(unBitMapSinFormato, sizeOFDelBitMap);//para 150k
	int i = 0;
	int cantiUno  = 0;
	int cantiCero = 0;

	for (i=0; i<osadaHeaderFile->fs_blocks; i++){//para 150k

		if(bitarray_test_bit(bitMap, i) == 0){
			cantiCero++;
		}

		if(bitarray_test_bit(bitMap, i) == 1){
			cantiUno++;
		}

	}
	printf("cantiUno: %i\n",cantiUno);
	printf("cantiCero: %i\n",cantiCero);
	/*TABLA DE ARCHIVO*/
	osada_file *tablaDeArchivo = malloc(2048*sizeof(osada_file));
	int k=0;
	printf("Test: %i\n", 2048*sizeof(osada_file));
	//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
	memcpy(tablaDeArchivo, &osada[64 + sizeOFDelBitMap], 2048*sizeof(osada_file));
	printf("Test: %i\n", sizeof(osada_file));
 	for (k=0; k<=2047; k++){
 /*		tablaDeArchivo[k].state = DIRECTORY;
 		tablaDeArchivo[k].parent_directory = 9;

 		memcpy(tablaDeArchivo[k].fname, "put", 17);
*/
 		printf("%i**********************************************\n",k);
		printf("state_%i: %c\n",k, tablaDeArchivo[k].state);
		printf("joel parent_directory_%i: %i\n",k, tablaDeArchivo[k].parent_directory);
	printf("fname_%i: %s\n",k, &tablaDeArchivo[k].fname);
	/*	printf("file_size_%i: %i\n",k, tablaDeArchivo[k].file_size);
		printf("fname_%i: %s\n",k, &tablaDeArchivo[k].lastmod);
		printf("file_size_%i: %i\n",k, tablaDeArchivo[k].first_block);
*/
		//msync(osada, pagesize, MS_SYNC);
	}
	memcpy(&osada[64 + sizeOFDelBitMap], tablaDeArchivo, 2048*sizeof(osada_file));

	/*CERRAR TODO*/
	int status = munmap(osada, size);
	printf("Estado del munmap: %i", status);
	int statusCerrar = close(fid);
	free(tablaDeArchivo);
}
