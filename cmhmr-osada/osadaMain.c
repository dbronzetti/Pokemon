/*
 * osadaMain.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */
#include "osada.h"
#include "mockOsada.h"



int main(int argc, char *argv[]){
	int archivoID = obtenerIDDelArchivo(argv[1]);
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	unsigned char *osada = inicializarOSADA(archivoID);
	osada_header *osadaHeaderFile = obtenerHeader(osada);
	setearConstantesDePosicionDeOsada(osadaHeaderFile);


	//t_bitarray *bitMap = obtenerBitmap();
	obtenerBitmap();

	osada_file *tablaDeArchivo = obtenerTablaDeArchivos(osada, osadaHeaderFile);
	int *tablaDeAsignacion = obtenerTablaDeAsignacion();
	//char *bloqueDeDatos = obtenerBloqueDeDatos(osada, osadaHeaderFile);

	//contarBloques(osada, osadaHeaderFile, bitMap);
	//crearUnArchivo("hola mundo\n", 128);

	//dameTodosLosDirectorios(tablaDeArchivo);
	//dameTodosLosArchivosRegulares(tablaDeArchivo);
	//dameTodosLosBorrados(tablaDeArchivo);
	//dameTodosLosOtrosEstados(tablaDeArchivo);

	//crearArbolAPartirDelPadre(tablaDeArchivo, 65535);

	//t_list unaLista = crearArbolAPartirDelPadre(tablaDeArchivo, 0); //METER TODO EN UNA LISTA, SERIALIZAR E ENVIAR
	//char *altoPaquete = serializeListaBloques(unaLista);
	//sendMessage(&serverData->socketClient, altoPaquete, sizeof(altoPaquete));

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
	//osada_block_pointer posicion = buscarArchivo(tablaDeArchivo, "README.txt");
	//printf("posicion del primer bloque: %i\n", posicion);

	//t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(tablaDeAsignacion, posicion);
	t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(tablaDeAsignacion, 31216);

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


