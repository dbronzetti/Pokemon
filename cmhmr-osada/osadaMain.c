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
	t_bitarray *bitMap = obtenerBitmap(osada, osadaHeaderFile);
	osada_file *tablaDeArchivo = obtenerTablaDeArchivos(osada, osadaHeaderFile);
	int *tablaDeAsignacion = obtenerTablaDeAsignacion(osada, osadaHeaderFile);
	char *bloqueDeDatos = obtenerBloqueDeDatos(osada, osadaHeaderFile);

	//mockInicializarOSADA();
	free(bitMap);
	free(osadaHeaderFile);
	free(tablaDeArchivo);
	//free(osada);
	return 0;
}


