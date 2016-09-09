/*
 * osadaMain.c
 *
 *  Created on: 5/9/2016
 *      Author: utnso
 */
#include <stdint.h>
#include "osada.h"
#include <stdlib.h>
#include <stdio.h>
/*
Antes hay que hacer esto
 truncate --size 100M disco.bin
 ./osada-format disco.bin
*/
  
void leerArchivo2(){
	osada_file *object2=malloc(sizeof( osada_file));
	FILE * file= fopen("disco.bin", "rb");
	if (file != NULL) {
		fread(object2, sizeof(osada_file), 1, file);
		fclose(file);
	}
	printf("%i\n",object2->file_size);
}
int main(void) {
   leerArchivo2();

	return 0;

}
