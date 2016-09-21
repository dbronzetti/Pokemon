/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"


void inicializarOSADA(){

	caddr_t mmap_ptr;
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco-chico.bin";
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	unsigned char *osada;
	t_bitarray *bitMap;
	unsigned char *unBitMapSinFormato;

	int fid	= open(ruta ,O_RDWR,(mode_t)0777);
	int pagesize = getpagesize();
	printf("pagesize: %i\n", pagesize);
	printf("sizeof(osada_header): %i\n",  sizeof(osada_header));

	if(fid<0){
		fprintf(stderr,"Bad Open of file <%s>\n",ruta);
		error("Failed to open mmap file, QUIT!");
	}

	osada = mmap(NULL, pagesize, PROT_READ|PROT_WRITE,MAP_SHARED, fid, 0);
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

	osadaHeaderFile->version = 2;
	printf("version: %i\n", osadaHeaderFile->version);

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
	osada_file *tablaDeArchivo = malloc(1024*sizeof(osada_file));
	int k=0;
	printf("Test: %i\n", 1024*sizeof(osada_file));

	memcpy(tablaDeArchivo, &osada[64 + sizeOFDelBitMap], 1024*sizeof(osada_file));
	printf("Test: %i\n", 1);
 	for (k=0; k<=2048; k++){
 		//(tablaDeArchivo[k].state) = DIRECTORY;
 		//tablaDeArchivo[k].parent_directory = 6;
		printf("%i**********************************************\n",k);
		printf("state_%i: %s\n",k, &(tablaDeArchivo[k]).state);
		printf("parent_directory_%i: %i\n",k, tablaDeArchivo[k].parent_directory);
		printf("fname_%i: %s\n",k, &tablaDeArchivo[k].fname);
		printf("file_size_%i: %i\n",k, tablaDeArchivo[k].file_size);
		printf("fname_%i: %s\n",k, &tablaDeArchivo[k].lastmod);
		printf("file_size_%i: %i\n",k, tablaDeArchivo[k].first_block);
		//msync(osada, pagesize, MS_SYNC);
	}

	/*CERRAR TODO*/
	int status = munmap(tablaDeArchivo, pagesize);

	int statusCerrar = close(fid);
	free(tablaDeArchivo);
}


osada_header *leerArchivoParaHeader(char *ruta){
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	FILE * archivo= fopen(ruta, "rb");

	if (archivo != NULL) {
		fread(osadaHeaderFile, sizeof(osada_header), 1, archivo);
		fclose(archivo);
	}
	return osadaHeaderFile;
}



unsigned char devolverMagicNumbre(osada_header *osadaHeader){
	return osadaHeader->magic_number[7];
}

uint8_t devolverVersion(osada_header *osadaHeader){
	return osadaHeader->version;
}

int devolverFSBloques(osada_header *osadaHeader){
	return osadaHeader->fs_blocks;
}

uint32_t devolverBitMapBlocks(osada_header *osadaHeader){
	return osadaHeader->bitmap_blocks; // bitmap size in blocks
}

uint32_t devolverAllocations_table_offset(osada_header *osadaHeader){
	return  osadaHeader->allocations_table_offset; // allocations table's first block number

}

uint32_t devolverDataBlocks(osada_header *osadaHeader){
	return osadaHeader->data_blocks; // amount of data blocks
}


unsigned char devolverPadding(osada_header *osadaHeader){
	return	osadaHeader->padding[40]; // useless bytes just to complete the block size
}



/*FILE SYSTEM */
osada_file *leerArchivoParaOsadaFile(char *ruta){
	osada_file *osadaFile = malloc(sizeof(osada_file));
	FILE * archivo= fopen(ruta, "rb");

	if (archivo != NULL) {
		fread(osadaFile, sizeof(osada_file), 1, archivo);
		fclose(archivo);
	}
	return osadaFile;
}

int devolverTamanio(osada_file *osadaFile){
	return osadaFile->file_size;
}

osada_file_state devolverEstado(osada_file *osadaFile){
	return osadaFile->state;
}

unsigned char devolverNombreDelArchivo(osada_file *osadaFile){
	return osadaFile->fname[OSADA_FILENAME_LENGTH];
}

uint16_t devolverDirectorioPadre(osada_file *osadaFile){
	return osadaFile->parent_directory;
}

uint32_t devolverUltimaMod(osada_file *osadaFile){
	return osadaFile->lastmod;
}

osada_block_pointer devolverPrimerBloque(osada_file *osadaFile){
	return osadaFile->first_block;
}
