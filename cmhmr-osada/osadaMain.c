/*
 * osadaMain.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"
#include <string.h>



int main(int argc, char *argv[]){
	int i=0;
	int archivoID = obtenerIDDelArchivo(argv[1]);
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	inicializarOSADA(archivoID);
	obtenerHeader();
	setearConstantesDePosicionDeOsada();

	obtenerBitmap();

    obtenerTablaDeArchivos();
	obtenerTablaDeAsignacion();

	//contarBloques(osada, osadaHeaderFile, bitMap);
	int posDelaTablaDeArchivos = escribirEnLaTablaDeArchivos(65535, 0, "/test1", -999, -999);
	crearUnArchivo("hola como andas todo bien que se cuenta nose como estabas tratandonte la vida pero es dififcil.", strlen("hola como andas todo bien que se cuenta nose como estabas tratandonte la vida pero es dififcil."), "/test1", posDelaTablaDeArchivos, 65535);

	//dameTodosLosDirectorios();
	//dameTodosLosArchivosRegulares();
	//dameTodosLosBorrados();
	//dameTodosLosOtrosEstados();

	//crearArbolAPartirDelPadre(65535);


	//encontrarArbolPadre(tablaDeArchivo, 140);
	//printf("bytes libres: %i\n",bytesLibres());

	/*
	t_dictionary *dirDicRoot = dictionary_create();
	t_dictionary *dirDicHijos = dictionary_create();
	dirDicRoot = crearArbolDeDirectoriosDelRoot(tablaDeArchivo);

	dirDicHijos = crearArbolDeDirectoriosHijos(tablaDeArchivo, dirDicRoot);

	 */

	//README.txt
	//021.txt
	//"large.txt" - basic
	//archivo - basic
	//special.mp4 - 138
	osada_block_pointer posicion = buscarArchivo("/test1", 65535);
	//printf("posicion del primer bloque: %i\n", posicion);

	t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
	//t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(31216);

	verContenidoDeArchivo(conjuntoDeBloquesDelArchivo);


	//mock_setearTamanioDelArchivo(tamanioDelArchivo);
	//mock_setearConstantesDePosicionDeOsada(osadaHeaderFile);

	//mock_Crear_Directorios(osada, osadaHeaderFile);
	//mock_Crear_La_Tabla_De_Asignacion_De_Los_Directorios(osada, tablaDeAsignacion, osadaHeaderFile);
	//mock_Modificar_Los_Bloques_Del_Bitmap(osada, bitMap, osadaHeaderFile);


	printf("\n************TERMINO TODO************\n");

	//free(bitMap);
	//free(osadaHeaderFile);
	//free(tablaDeArchivo);

	//free(tablaDeAsignacion);//Error in `./cmhmr-osada': double free or corruption (out): 0x097611b0
	//free(bloqueDeDatos);//Error in `./cmhmr-osada': double free or corruption (out): 0x097611b0

	return 0;
}


