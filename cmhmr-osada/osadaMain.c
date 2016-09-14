/*
 * osadaMain.c
 *
 *  Created on: 5/9/2016
 *      Author: utnso
 */

#include "osada.h"

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

	if (archivo != NULL) {
		fread(osadaHeaderFile, sizeof(osada_header), 1, archivo);
		//fclose(archivo);
	}
	//osada_header *osadaHeader = leerArchivoParaHeader(ruta);
	/*
	char *ruta = "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/disco.bin";
	osada_file osadaFile[2048] = leerArchivoParaOsadaFile(ruta);


	printf("Tamanio: %i\n", devolverTamanio(osadaFile));
	printf("Nombre: %d\n", (unsigned char)devolverNombreDelArchivo(osadaFile));
	printf("Directorio Padre: %i\n", devolverDirectorioPadre(osadaFile));
	printf("Ultima Mod: %d\n", devolverUltimaMod(osadaFile));
	printf("Primer Bloque: %i\n", devolverPrimerBloque(osadaFile));
	free(osadaFile);
*/

/*
	printf("magic_number: %i\n", devolverMagicNumbre(osadaHeader));
	printf("version: %i\n", devolverVersion(osadaHeader));
	printf("fs_blocks: %i\n", devolverFSBloques(osadaHeader));
	printf("bitmap_blocks: %i\n", devolverBitMapBlocks(osadaHeader));
	printf("allocations_table_offset: %i\n", devolverAllocations_table_offset(osadaHeader));
	printf("fs_blocks: %i\n", devolverFSBloques(osadaHeader));
	printf("data_blocks: %i\n", devolverDataBlocks(osadaHeader));
	printf("padding: %i\n", devolverPadding(osadaHeader));
*/

	//free(osadaHeader);

/*
 * https://github.com/sisoputnfrba/foro/issues/445
	Segun lo que entendia el header era lo primero que tenia el archivo, use esto, y me trajo bien todo con la estructura que esperaba

	    fread(header,sizeof(osada_header),1,file);

	y para leer la tabla de archivos tenes que pasar el bitmap, una vez que leiste el header, tu puntero en el FILE queda al final del header (principio de bitmap). El tamaño del bitmap lo tenes en el header(pero en bloques).
*/


	osada_file osadaFile[2048];
	fseek(archivo, osadaHeaderFile->bitmap_blocks*64, SEEK_CUR);
	fread(osadaFile, sizeof(osada_file), 2048, archivo);
	int k=0;
	for (k=0; k<2048; k++){
		printf("It is %i\n",osadaFile[k].parent_directory);
	}
	return 0;


}
