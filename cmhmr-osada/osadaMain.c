/*
 * osadaMain.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */
#include "osada.h"


int main(int argc, char *argv[]){

	unsigned char *osada = inicializarOSADA(argv[1]);
	osada_header *osadaHeaderFile = obtenerHeader(osada);
	obtenerBitmap(osada, osadaHeaderFile);
	//mockInicializarOSADA();
	//free(osada);
	return 0;
}


