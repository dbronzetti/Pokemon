/*
 * osadaMain.c
 *
 *  Created on: 5/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <commons/bitarray.h>
#include <stdio.h>

/*
 	 	 	 	 Antes hay que hacer esto:
 	 	 	 	 truncate --size 100M disco.bin
 	 	 	 	 ./osada-format disco.bin
*/
int main(void) {
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco-chico.bin";
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	FILE * archivo= fopen(ruta, "rb");
	t_bitarray *bitarray;
	char *bitArray="0";
	char lala[200];

	if (archivo != NULL) {
		fread(osadaHeaderFile, sizeof(osada_header), 1, archivo);
		//fclose(archivo);
	}

	printf("magic_number: %i\n", osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %i\n", osadaHeaderFile->padding);

	/**************************************/
	//bitarray = bitarray_create(data, sizeof(data));

	fseek(archivo, sizeof(osada_header)+1, SEEK_CUR);
	bitArray = malloc(osadaHeaderFile->bitmap_blocks*64);
	fread(bitArray, osadaHeaderFile->bitmap_blocks*64, 1, archivo);
	bitarray = bitarray_create(bitArray, osadaHeaderFile->bitmap_blocks*64);
	printf("que onda: %i",bitarray_test_bit(bitarray, 1025));

	/**************************************/


/*
 * https://github.com/sisoputnfrba/foro/issues/445
	Segun lo que entendia el header era lo primero que tenia el archivo, use esto, y me trajo bien todo con la estructura que esperaba

	    fread(header,sizeof(osada_header),1,file);

	y para leer la tabla de archivos tenes que pasar el bitmap, una vez que leiste el header, tu puntero en el FILE queda al final del header (principio de bitmap). El tamaño del bitmap lo tenes en el header(pero en bloques).
*/

	//TENGO Q LEER EL BITMAP!!!!
	/*
	osada_file osadaFile[2048];
	fseek(archivo, osadaHeaderFile->bitmap_blocks*64, SEEK_CUR);
	fread(osadaFile, sizeof(osada_file), 2048, archivo);
	int k=0;
	for (k=0; k<2048; k++){
		printf("**********************************************\n");
		printf("state_%i: %s\n",k, osadaFile[k].state);
		printf("parent_directory_%i: %i\n",k, osadaFile[k].parent_directory);
		printf("fname_%i: %s\n",k, osadaFile[k].fname);
		printf("file_size_%i: %i\n",k, osadaFile[k].file_size);
		printf("fname_%i: %s\n",k, osadaFile[k].lastmod);
		printf("file_size_%i: %i\n",k, osadaFile[k].first_block);
	}
	*/
	return 0;


}
