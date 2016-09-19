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

 	 	 	 	 //bless disco.bin
*/
int main(void) {
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco-chico.bin";
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

	/*****************BIT MAP*********************/
	fseek(archivo, 0, sizeof(osada_header));

	bitArray = malloc(1000);//para 100k
	int bytesLeidos = 0;
	bytesLeidos=fread(bitArray, OSADA_BLOCK_SIZE, osadaHeaderFile->bitmap_blocks, archivo);
	int sizeOFDelBitMap = osadaHeaderFile->bitmap_blocks * OSADA_BLOCK_SIZE;

	//bitarray = bitarray_create(bitArray, 192);//para 100k

	//REPRESENTA EL MAPA DEL BITMAP
	bitarray = bitarray_create(bitArray, sizeOFDelBitMap);//para 150k
	int i = 0;
	int cantiUno  = 0;
	int cantiCero = 0;

	//LUIS: UN BIT REPRESENTA UN BLOQUE.
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

	len = ftell(archivo);
	printf("Longitud del archivo despuesdelBitmap: %i\n",len);

	/**********Tabla de Archivos**********/
	osada_file osadaFile[2048];
	fseek(archivo, 0, SEEK_CUR);

	bytesLeidos = fread(osadaFile, sizeof(osada_file), 1024, archivo);

	int k=0;
	/*
	 *ESTA BIEN QUE SEA UN ARRAY DE 2048 POSICIONES, SABIENDO QUE HICE UN FREAD DE 1024 DE BLOQUES DE OSADA_FILE?
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
	len = ftell(archivo);
	printf("Longitud del archivo despues de tabla de achivos: %i\n",len);

	/**********TABLA DE ASGINACION**********/
	/*
	 * saco la cantidad de bloques que existe
	 * en el fread leo el entero de la cantidad de bloques
	 *
	 *
	 * */
	int numeroBloques = (osadaHeaderFile->fs_blocks - 1 - osadaHeaderFile->bitmap_blocks - 1024) * 4 /64;
	int *arrayTabla=malloc(1000);
	printf("numeroBloques : %i\n", numeroBloques);
	fseek(archivo,0, SEEK_CUR);
	bytesLeidos = fread(arrayTabla, sizeof(int), numeroBloques, archivo);
	len = ftell(archivo);
	for (k=0; k<numeroBloques; k++){
		//printf("Array tabla asignada: %i\n",arrayTabla[0]);
	}
	printf("Longitud del archivo despues de tabla de achivos: %i\n",len);

	/**********Bloques de datos**********/
	/*
	 * los bloques de datos son de 64? el header ya te da la cantidad bloques, esta bien el fread?
	 *
	 * */
	fseek(archivo,0, SEEK_CUR);
	bytesLeidos=fread(bitArray, OSADA_BLOCK_SIZE, osadaHeaderFile->data_blocks, archivo);
	len = ftell(archivo);

	printf("Longitud del archivo despues de tabla de achivos: %i\n",len);
	return 0;

}
