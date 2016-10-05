/*
 * osadaMain.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"
#include <commons/collections/list.h>

int main(int argc, char *argv[]){
	int archivoID = obtenerIDDelArchivo(argv[1]);
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	unsigned char *osada = inicializarOSADA(archivoID);
	osada_header *osadaHeaderFile = obtenerHeader(osada);
	setearConstantesDePosicionDeOsada(osadaHeaderFile);


	t_bitarray *bitMap = obtenerBitmap(osada, osadaHeaderFile);
	osada_file *tablaDeArchivo = obtenerTablaDeArchivos(osada, osadaHeaderFile);
	int *tablaDeAsignacion = obtenerTablaDeAsignacion(osada, osadaHeaderFile);
	//char *bloqueDeDatos = obtenerBloqueDeDatos(osada, osadaHeaderFile);

	//dameTodosLosDirectorios(tablaDeArchivo);
	//dameTodosLosArchivosRegulares(tablaDeArchivo);
	//dameTodosLosBorrados(tablaDeArchivo);

	crearArbolDeDirectorios(tablaDeArchivo);

	//README.txt
	//021.txt
	//"large.txt" - basic
	//archivo - basic
	//osada_block_pointer posicion = buscarArchivo(tablaDeArchivo, "special.txt");
	//printf("posicion del primer bloque: %i\n", posicion);

	//t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(tablaDeAsignacion, posicion);
	//verContenidoDeArchivo(conjuntoDeBloquesDelArchivo, osada);


	//mock_setearTamanioDelArchivo(tamanioDelArchivo);
	//mock_setearConstantesDePosicionDeOsada(osadaHeaderFile);

	//mock_Crear_Directorios(osada, osadaHeaderFile);
	//mock_Crear_La_Tabla_De_Asignacion_De_Los_Directorios(osada, tablaDeAsignacion, osadaHeaderFile);
	//mock_Modificar_Los_Bloques_Del_Bitmap(osada, bitMap, osadaHeaderFile);


	printf("\n************TERMINO TODO************\n");

	free(bitMap);
	//free(osadaHeaderFile);
	//free(tablaDeArchivo);

	//free(tablaDeAsignacion);//Error in `./cmhmr-osada': double free or corruption (out): 0x097611b0
	//free(bloqueDeDatos);//Error in `./cmhmr-osada': double free or corruption (out): 0x097611b0

	return 0;
}


