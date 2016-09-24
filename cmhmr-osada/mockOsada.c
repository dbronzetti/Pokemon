/*
 * mockOsada.c
 *
 *  Created on: 23/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"
#include <errno.h>

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

void mockCrearDirectorios(unsigned char *osada, osada_header *osadaHeaderFile, int tamanioDelArchivo){
		/*TABLA DE ARCHIVO*/
		printf("creaDirectorios\n");

		int sizeOFDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;
		printf("ERROR: %s\n", strerror(errno));

		osada_file *tablaDeArchivo = malloc(2048*sizeof(osada_file));
		int k=0;
		printf("ERROR: %s\n", strerror(errno));

		//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
		memcpy(tablaDeArchivo, &osada[64 + sizeOFDelBitMap], 2048*sizeof(osada_file));
		printf("ERROR: %s\n", strerror(errno));

	 	for (k=0; k <= 2047; k++){
	 		printf("ERROR: %s\n", strerror(errno));


	 		tablaDeArchivo[k].state = DIRECTORY;
	 		tablaDeArchivo[k].parent_directory = k;
	 		memcpy(tablaDeArchivo[k].fname, "puto", 17);

	 		//tablaDeArchivo[k] = updatearTablaDeArchivos();
			//memcpy(tablaDeArchivo[k], updatearTablaDeArchivos(), sizeof(osada_file));
	 		printf("ERROR: %s\n", strerror(errno));

		}

	 	memcpy(&osada[64 + sizeOFDelBitMap], tablaDeArchivo, 2048*sizeof(osada_file));

		/*CERRAR TODO*/
		//printf("archivoID: %i\n", archivoID);
		printf("tamanio: %i\n", tamanioDelArchivo);
	 	int status = munmap(osada, tamanioDelArchivo);
		printf("Estado del munmap: %i\n", status);
		printf("ERROR: %s\n", strerror(errno));

}


