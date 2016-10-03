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
	dameTodosLosArchivosRegulares(tablaDeArchivo);
	//dameTodosLosBorrados(tablaDeArchivo);

	//crearArbolDeDirectorios(tablaDeArchivo);

	//caso para el archivo 114.txt, array 171
	printf("nombre del archivo: %s\n", tablaDeArchivo[171].fname);
	osada_block_pointer posicion = buscarArchivo(tablaDeArchivo, "114.txt");
	printf("posicion del primer bloque: %i\n", posicion);
	//t_list *proximo2 = crearPosicionesDeBloquesParaUnArchivo(tablaDeAsignacion, 19685);

	/*
	mostrarLosBloquesAsignados(tablaDeAsignacion, 19685);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19686);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19687);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19688);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19689);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19690);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19691);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19692);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19693);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19694);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19695);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19696);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19697);

	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19698);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19699);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19700);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19701);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19702);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19703);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19704);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19705);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19706);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19707);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19708);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19709);

	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19710);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19711);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19712);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19713);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19714);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19715);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19716);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19717);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19718);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19719);
	 mostrarLosBloquesAsignados(tablaDeAsignacion, 19720);
	 */
	 //el fin es menos -1

	 //LEEO BLOQUES DE DATOS
	 /*
	 printf("%c",bloqueDeDatos[19685]);
	 printf("%c",bloqueDeDatos[19686]);
	 printf("%c",bloqueDeDatos[19687]);
	 printf("%c",bloqueDeDatos[19688]);
	 printf("%c",bloqueDeDatos[19689]);
	 printf("%c",bloqueDeDatos[19690]);
	 printf("%c",bloqueDeDatos[19691]);
	 printf("%c",bloqueDeDatos[19692]);
	 printf("%c",bloqueDeDatos[19693]);
	 printf("%c",bloqueDeDatos[19694]);
	 printf("%c",bloqueDeDatos[19695]);
	 printf("%c",bloqueDeDatos[19696]);
	 printf("%c",bloqueDeDatos[19697]);

	 printf("%c",bloqueDeDatos[19698]);
	 printf("%c",bloqueDeDatos[19699]);
	 printf("%c",bloqueDeDatos[19700]);
	 printf("%c",bloqueDeDatos[19701]);
	 printf("%c",bloqueDeDatos[19702]);
	 printf("%c",bloqueDeDatos[19703]);
	 printf("%c",bloqueDeDatos[19704]);
	 printf("%c",bloqueDeDatos[19705]);
	 printf("%c",bloqueDeDatos[19706]);
	 printf("%c",bloqueDeDatos[19707]);
	 printf("%c",bloqueDeDatos[19708]);
	 printf("%c",bloqueDeDatos[19709]);

	 printf("%c",bloqueDeDatos[19710]);
	 printf("%c",bloqueDeDatos[19711]);
	 printf("%c",bloqueDeDatos[19712]);
	 printf("%c", bloqueDeDatos[19713]);
	 printf("%c",bloqueDeDatos[19714]);
	 printf("%c",bloqueDeDatos[19715]);
	 printf("%c",bloqueDeDatos[19716]);
	 printf("%c",bloqueDeDatos[19717]);
	 printf("%c",bloqueDeDatos[19718]);
	 printf("%c",bloqueDeDatos[19719]);
	 printf("%c",bloqueDeDatos[19720]);
	*/

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


