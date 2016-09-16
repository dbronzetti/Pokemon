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
 	 	 	 	 truncate --size 50k disco.bin
 	 	 	 	 ./osada-format disco.bin
*/
int main(void) {
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco.bin";
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	FILE * archivo= fopen(ruta, "rb");
	t_bitarray *bitarray;
	char *bitArray="0";
	char lala[200];

	int len = ftell(archivo);

	if (archivo != NULL) {
		fread(osadaHeaderFile, sizeof(osada_header), 1, archivo);
		//fclose(archivo);
	}
	len = ftell(archivo);


	printf("magic_number: %i\n", osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %i\n", osadaHeaderFile->padding);

	/**************************************/


	printf("Longitud del archivo 0: %i\n",len);

	fseek(archivo, 0, SEEK_CUR);

	bitArray = malloc(1000);//para 100k
	int bytesLeidos=fread(bitArray, OSADA_BLOCK_SIZE, osadaHeaderFile->bitmap_blocks, archivo);
	int sizeOFDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;

	//bitarray = bitarray_create(bitArray, 192);//para 100k
	bitarray = bitarray_create(bitArray, sizeOFDelBitMap);//para 150k
	int i = 0;
	int cantiUno  = 0;
	int cantiCero = 0;
	for (i=0; i<osadaHeaderFile->fs_blocks; i++){//para 150k

		if(bitarray_test_bit(bitarray, i) == 0){
			cantiCero++;
		}
		if(bitarray_test_bit(bitarray, i) == 1){
			cantiUno++;
		}

	}
	printf("cantiUno: %i\n",cantiUno);
	printf("cantiCero: %i\n",cantiCero);


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
