/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <errno.h>
#include <commons/string.h>

void initMutexOsada(){
    pthread_mutex_init(&OSADAmutex, NULL);
    pthread_mutex_init(&HEADERmutex, NULL);
    pthread_mutex_init(&BITMAPmutex, NULL);
    pthread_mutex_init(&DATA_BLOCKSmutex, NULL);
    pthread_mutex_init(&ARRAY_TABLA_ASIGNACIONmutex, NULL);
    pthread_mutex_init(&TABLA_DE_ARCHIVOSmutex, NULL);
}

void destroyMutexOsada(){
	pthread_mutex_destroy(&OSADAmutex);
	pthread_mutex_destroy(&HEADERmutex);
	pthread_mutex_destroy(&BITMAPmutex);
	pthread_mutex_destroy(&DATA_BLOCKSmutex);
	pthread_mutex_destroy(&ARRAY_TABLA_ASIGNACIONmutex);
	pthread_mutex_destroy(&TABLA_DE_ARCHIVOSmutex);
}

void guardarEnOsada(int desde, void *elemento, int tamaniaDelElemento){
	pthread_mutex_lock(&OSADAmutex);
	memcpy(&OSADA[desde], elemento, tamaniaDelElemento);
	pthread_mutex_unlock(&OSADAmutex);
}




char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile){
	//unsigned char *bloqueDeDatos = malloc(sizeof(char) * osadaHeaderFile->data_blocks);OLD
	pthread_mutex_lock(&HEADERmutex);
	unsigned char *bloqueDeDatos = malloc(sizeof(char) * OSADA_BLOCK_SIZE * osadaHeaderFile->data_blocks);
	pthread_mutex_unlock(&HEADERmutex);

	pthread_mutex_lock(&OSADAmutex);
	memcpy(bloqueDeDatos, &osada[DESDE_PARA_BLOQUE_DE_DATOS], TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS );
	pthread_mutex_unlock(&OSADAmutex);
	return bloqueDeDatos;
}

void mostarAsignacion(int asignado){
	log_info(logPokeDexServer, "Array tabla asignada: %i",asignado);
}

void mostrarTodosLosAsignados(int *arrayTabla, int numeroBloques){
	int pos = 0;

	for (pos = 0; pos < numeroBloques; pos++){
		mostarAsignacion(arrayTabla[pos]);
	}
}

int *obtenerTablaDeAsignacion(){
	int *arrayTabla = malloc(TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);

	pthread_mutex_lock(&OSADAmutex);
	memcpy(arrayTabla, &OSADA[DESDE_PARA_TABLA_ASIGNACION], TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION );
	pthread_mutex_unlock(&OSADAmutex);

	//mostrarTodosLosAsignados(arrayTabla, numeroBloques);
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	ARRAY_TABLA_ASIGNACION = arrayTabla;
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
	return arrayTabla;
}

void mostrarTodaLaTablaDeArchivos(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarStructDeArchivos(tablaDeArchivo[pos], pos);
	}
}

void mostrarStructDeArchivos(osada_file tablaDeArchivo, int pos){
	log_info(logPokeDexServer, "Empieza: %i****************",pos);
	log_info(logPokeDexServer, "state_%i: %c",pos, tablaDeArchivo.state);
	log_info(logPokeDexServer, "parent_directory_%i: %i",pos, tablaDeArchivo.parent_directory);
	log_info(logPokeDexServer, "fname_%i: %s",pos, &tablaDeArchivo.fname);
	log_info(logPokeDexServer, "file_size_%i: %i",pos, tablaDeArchivo.file_size);
	log_info(logPokeDexServer, "lastmod_%i: %i",pos, tablaDeArchivo.lastmod);
	log_info(logPokeDexServer, "first_block_%i: %i",pos, tablaDeArchivo.first_block);
	log_info(logPokeDexServer, "Termina: %i****************",pos);
}

osada_file *obtenerTablaDeArchivos(){
	osada_file *tablaDeArchivo = malloc(TAMANIO_TABLA_DE_ARCHIVOS);

	//2048*sizeof(osada_file) = 1024 bloques * 64 bytes ptr
	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	memcpy(tablaDeArchivo, &OSADA[DESDE_PARA_TABLA_DE_ARCHIVOS ], TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	pthread_mutex_unlock(&OSADAmutex);

	//mostrarTodaLaTablaDeArchivos(tablaDeArchivo);
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	TABLA_DE_ARCHIVOS = tablaDeArchivo;
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	return tablaDeArchivo;
}

void contarBloques(){
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (i=0; i < fs_blocks; i++){//para 150k

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i) == 0){
			bloquesLibres++;
			//log_info(logPokeDexServer, "Bloque - %i - LIBRE",i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i) == 1){
			bloquesOcupados++;
			//log_info(logPokeDexServer, "Bloque - %i - OCUPADO",i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

	}
	//log_info(logPokeDexServer, "Bloques Ocupados: %i",bloquesOcupados);
	//log_info(logPokeDexServer, "Bloques Libres: %i",bloquesLibres);
	//log_info(logPokeDexServer, "bytes libres: %i",bloquesLibres*64);
	BYTES_LIBRES = bloquesLibres*64;
	BYTES_OCUPADOS = bloquesOcupados*64;
	log_info(logPokeDexServer, "kb libres: %d", (bloquesLibres*64)/1024);

}

int bytesLibres(){
	return BYTES_LIBRES;
}

int bytesOcupados(){
	return BYTES_OCUPADOS;
}

t_bitarray *obtenerBitmap(){
	t_bitarray *bitMap;
	unsigned char *unBitMapSinFormato;

	unBitMapSinFormato = malloc(TAMANIO_DEL_BITMAP );
	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&BITMAPmutex);
	memcpy(unBitMapSinFormato, &OSADA[DESDE_PARA_BITMAP], TAMANIO_DEL_BITMAP );
	pthread_mutex_unlock(&BITMAPmutex);
	pthread_mutex_unlock(&OSADAmutex);

	pthread_mutex_lock(&BITMAPmutex);
	bitMap = bitarray_create(unBitMapSinFormato, TAMANIO_DEL_BITMAP );
	BITMAP = bitMap;
	pthread_mutex_unlock(&BITMAPmutex);
	contarBloques();

	return bitMap;

}



void mostrarHeader(osada_header *osadaHeaderFile){
	pthread_mutex_lock(&HEADERmutex);
	log_info(logPokeDexServer, "magic_number 2: %s",  osadaHeaderFile->magic_number);
	log_info(logPokeDexServer, "version: %i", osadaHeaderFile->version);
	log_info(logPokeDexServer, "fs_blocks: %i", osadaHeaderFile->fs_blocks);
	log_info(logPokeDexServer, "bitmap_blocks: %i", osadaHeaderFile->bitmap_blocks);
	log_info(logPokeDexServer, "allocations_table_offset: %i", osadaHeaderFile->allocations_table_offset);
	log_info(logPokeDexServer, "data_blocks: %i", osadaHeaderFile->data_blocks);
	log_info(logPokeDexServer, "padding: %s",   osadaHeaderFile->padding);
	pthread_mutex_unlock(&HEADERmutex);
}

osada_header *obtenerHeader(){
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	pthread_mutex_lock(&OSADAmutex);
	memcpy(osadaHeaderFile, OSADA, OSADA_BLOCK_SIZE);
	pthread_mutex_unlock(&OSADAmutex);

	mostrarHeader(osadaHeaderFile);
	pthread_mutex_lock(&HEADERmutex);
	HEADER = osadaHeaderFile;
	pthread_mutex_unlock(&HEADERmutex);
	return osadaHeaderFile;

}

/*SETEA Y GUARDA EN LA VARIABLE GLOBAL, ADEMAS SI SE NECESITA SE DEVUELVE EL TAMAÑO PARA USARLOS EN MOCKS*/
int setearTamanioDelArchivo(int archivoID){
	struct stat buffer;
	fstat(archivoID, &buffer);
	TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES = buffer.st_size;
	return TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES;
}

int obtenerIDDelArchivo(char *ruta){
	log_info(logPokeDexServer, "ruta: %s", ruta);
	return open(ruta, O_RDWR, (mode_t)0777);
}
void setearConstantesDePosicionDeOsada(){
	TAMANIO_QUE_OCUPA_EL_HEADER = OSADA_BLOCK_SIZE;
	pthread_mutex_lock(&HEADERmutex);
	TAMANIO_DEL_BITMAP = HEADER->bitmap_blocks * OSADA_BLOCK_SIZE;
	pthread_mutex_unlock(&HEADERmutex);
	TAMANIO_TABLA_DE_ARCHIVOS =  2048 * sizeof(osada_file);
	pthread_mutex_lock(&HEADERmutex);
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION = (HEADER->fs_blocks - 1 - HEADER->bitmap_blocks - 1024) * 4;
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES = (HEADER->fs_blocks - 1 - HEADER->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS = OSADA_BLOCK_SIZE* HEADER->data_blocks;
	pthread_mutex_lock(&DATA_BLOCKSmutex);
	DATA_BLOCKS= (HEADER->fs_blocks - HEADER->data_blocks)*64;
	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	//dataBlocks=  osadaHeaderFile->allocations_table_offset + tamanioQueOcupaLaTablaDeAsignacionEnBloques;

	log_info(logPokeDexServer, "HEADER->fs_blocks - HEADER->data_blocks: %i",HEADER->fs_blocks - HEADER->data_blocks);
	pthread_mutex_unlock(&HEADERmutex);
	log_info(logPokeDexServer, "dataBlocks: %i",DATA_BLOCKS);

	DESDE_PARA_BITMAP = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	DESDE_PARA_TABLA_DE_ARCHIVOS  = OSADA_BLOCK_SIZE + TAMANIO_DEL_BITMAP;
	DESDE_PARA_TABLA_ASIGNACION  = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS;
	DESDE_PARA_BLOQUE_DE_DATOS = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS + TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION;
	log_info(logPokeDexServer, "desdeParaTablaAsigancion: %i",DESDE_PARA_TABLA_ASIGNACION );
	log_info(logPokeDexServer, "desdeParaBloqueDeDatos: %i",DESDE_PARA_BLOQUE_DE_DATOS);
}


unsigned char *inicializarOSADA(int archivoID){
	unsigned char *osada;
	setlocale(LC_ALL, "es_ES.UTF-8");
	/************************************************************/
	log_info(logPokeDexServer, "Locale is: %s", setlocale(LC_ALL, "es_ES.UTF-8"));
	log_info(logPokeDexServer, "Que paso?: %s", strerror(errno));
	log_info(logPokeDexServer, "archivoID: %i", archivoID);
	log_info(logPokeDexServer, "tamanio: %i", TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES);
	/************************************************************/


	osada = mmap(0, TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES, PROT_READ|PROT_WRITE,MAP_SHARED, archivoID, 0);
	int statusCerrar = close(archivoID);
	pthread_mutex_lock(&OSADAmutex);
	OSADA = osada;
	pthread_mutex_unlock(&OSADAmutex);
	return osada;

}
/********************************************ARCHIVOS**************************************/

void _iterarParaVerContenido(int bloque){

	char *bloqueDeDatos = malloc(OSADA_BLOCK_SIZE + 1);
	int bloque2 = bloque *64;
	int i;
	//tamanioQueOcupaElBloqueDeDatos ir de atras con los bloques.
	//log_info(logPokeDexServer, "%i", dataBlocks);
	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&DATA_BLOCKSmutex);
	memcpy(bloqueDeDatos, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );
	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	pthread_mutex_unlock(&OSADAmutex);

//	for(i=1; i<=64; i++){
	bloqueDeDatos[OSADA_BLOCK_SIZE + 1] = '\0';
		log_info(logPokeDexServer, "%s", bloqueDeDatos);
	//}
	//log_info(logPokeDexServer, "\nTERMINO");
	free(bloqueDeDatos);



}

void verContenidoDeArchivo(t_list *conjuntoDeBloques){
	list_iterate(conjuntoDeBloques, (void*) _iterarParaVerContenido);
}

void _iterarBloques(int bloque){
	log_info(logPokeDexServer, "_iterarBloques el proximo: %i", bloque);
}



t_list *obtenerElListadoDeBloquesCorrespondientesAlArchivo(int bloqueActual){
	int elProximo = 0;
	t_list *listaDeBloques = list_create();

	if ( bloqueActual!=-999){
		list_add(listaDeBloques, bloqueActual);

		pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
		elProximo = ARRAY_TABLA_ASIGNACION[bloqueActual];
		pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

		while (elProximo != -1){
			list_add(listaDeBloques, elProximo);
			bloqueActual = elProximo;
			pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
			elProximo = ARRAY_TABLA_ASIGNACION[bloqueActual];
			pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
		}
	}

	//list_iterate(listaDeBloques, (void*) _iterarBloques);

	return listaDeBloques;
}

osada_block_pointer comprobarElNombreDelArchivo(osada_file tablaDeArchivo, uint16_t parent_directory, char *nombre){
	char *tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim;
	char *n;
	tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim = string_duplicate(&tablaDeArchivo.fname);
	n = string_duplicate(nombre);
	string_trim(&tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	string_trim(&n);
	/*
	log_info(logPokeDexServer, "tablaDeArchivo.parent_directory: %i", tablaDeArchivo.parent_directory);
	log_info(logPokeDexServer, "parent_directory_: %i", parent_directory);
	log_info(logPokeDexServer, "nac: %s",tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	log_info(logPokeDexServer, "n: %s",n);
	*/
	if (tablaDeArchivo.parent_directory == parent_directory && tablaDeArchivo.state == REGULAR && strcmp(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim, n) == 0){
		free(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
		free(n);
		/*
		log_info(logPokeDexServer, "state_: %c", tablaDeArchivo.state);
		log_info(logPokeDexServer, "parent_directory_: %i", tablaDeArchivo.parent_directory);
		log_info(logPokeDexServer, "fname_: %s",&tablaDeArchivo.fname);
		log_info(logPokeDexServer, "file_size_: %i",tablaDeArchivo.file_size);
		log_info(logPokeDexServer, "lastmod_: %i", tablaDeArchivo.lastmod);
		log_info(logPokeDexServer, "first_block_: %i",tablaDeArchivo.first_block);
		*/
		return tablaDeArchivo.first_block;
	}

	free(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	free(n);
	return -666;
}



osada_block_pointer devolverOsadaBlockPointer(char *nombre, uint16_t parent_directory){
	log_info(logPokeDexServer, "******************************** ENTRO EN EL devolverOsadaBlockPointer ******************************** ");
	int pos=0;
	osada_block_pointer posicion = -999;
	char *file_name = strrchr (nombre, '/') + 1;
	log_info(logPokeDexServer, "file_name: %s", file_name);
	bool found = false;

	log_info(logPokeDexServer, "devolverOsadaBlockPointer - parent_directory: %i", parent_directory);

	for (pos=0; pos <= 2047; pos++){

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if ((posicion = comprobarElNombreDelArchivo(TABLA_DE_ARCHIVOS[pos], parent_directory,  file_name)) != -666){
			log_info(logPokeDexServer, "devolverOsadaBlockPointer - Encontro archivo - pos:  %i", pos);
			found = true;
		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

		if (found){
			return posicion;
		}
	}

	log_info(logPokeDexServer, "******************************* devolverOsadaBlockPointer- NO LO ENCONTRO! ******************************* ");
	return posicion;
}

int buscarElArchivoYDevolverPosicion(char *nombre, uint16_t parent_directory){
	log_info(logPokeDexServer, "******************************** ENTRO EN buscarElArchivo  ******************************** ");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	//log_info(logPokeDexServer, "file_name: %s", file_name);
	bool found = false;
	for (pos=0; pos <= 2047; pos++){
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		char *nac;
		char *n;
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);
		//log_info(logPokeDexServer, "nac: %s", &TABLA_DE_ARCHIVOS[pos].fname);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			log_info(logPokeDexServer, "******************************* buscarElArchivoYDevolverPosicion - LO ENCONTRO! ******************************* ");
			found = true;
		}else if(TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) != 0 && pos == 2047){
			pos =-666;
			log_info(logPokeDexServer, "******************************* buscarElArchivoYDevolverPosicion - NO LO ENCONTRO! ******************************* ");

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		free(nac);
		free(n);

		if (found){
			break;
		}
	}

	return pos;
}

osada_file buscarElArchivoYDevolverOsadaFile(char *nombre, uint16_t parent_directory, int* posicionTablaDeArchivo){


	log_info(logPokeDexServer, "******************************** ENTRO EN buscarElArchivoYDevolverOsadaFile  ******************************** ");
	log_info(logPokeDexServer, "DATOS INICIALES: nombre: %s - parent_directory: %d - posicionTablaDeArchivo %d",nombre,parent_directory,posicionTablaDeArchivo);
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;

	//log_info(logPokeDexServer, "file_name: %s", file_name);
	bool found = false;
	for (pos=0; pos <= 2047; pos++){
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory ){
			char *nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
			char *n   = string_duplicate(file_name);
			string_trim(&nac);
			string_trim(&n);
			//log_info(logPokeDexServer, "nac: %s", &TABLA_DE_ARCHIVOS[pos].fname);
			if (strcmp(nac, n) == 0){
				log_info(logPokeDexServer, "*******************************buscarElArchivoYDevolverOsadaFile - LO ENCONTRO! ******************************* ");
				found = true;
			}
			free(nac);
			free(n);
		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

		if(found){
			break;
		}
	}
	if(!found){
		*posicionTablaDeArchivo=-666;
		log_info(logPokeDexServer, "******************************* buscarElArchivoYDevolverOsadaFile -NO LO ENCONTRO! ******************************* ");
	}else{
		*posicionTablaDeArchivo=pos;
	}

	return TABLA_DE_ARCHIVOS[pos];
}

void borrarBloqueDelBitmap(int bloque){

	pthread_mutex_lock(&BITMAPmutex);
	if(bitarray_test_bit(BITMAP, bloque) == 1){
		bitarray_clean_bit(BITMAP, bloque);
	}
	pthread_mutex_unlock(&BITMAPmutex);

}

void borrarBloquesDelBitmap(t_list *listado){
	list_iterate(listado, (void*)borrarBloqueDelBitmap);
	pthread_mutex_lock(&BITMAPmutex);
	guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
	pthread_mutex_unlock(&BITMAPmutex);

}

int ingresarElUTIMENS(char *nombre, uint16_t parent_directory, int tv_nsec){
	log_info(logPokeDexServer, "******************************** ENTRO EN borrarUnArchivo  ******************************** ");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	log_info(logPokeDexServer, "file_name: %s", file_name);

	for (pos=0; pos <= 2047; pos++){
		char *nac;
		char *n;
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			log_info(logPokeDexServer, "******************************* LO ENCONTRO! ******************************* ");
			TABLA_DE_ARCHIVOS[pos].lastmod = tv_nsec;
			posicion = pos;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	}

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

	return posicion;
}

int borrarUnArchivo(char *nombre, uint16_t parent_directory){
	log_info(logPokeDexServer, "******************************** ENTRO EN borrarUnArchivo  ******************************** ");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	log_info(logPokeDexServer, "file_name: %s", file_name);

	for (pos=0; pos <= 2047; pos++){
		char *nac;
		char *n;
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			log_info(logPokeDexServer, "******************************* LO ENCONTRO! ******************************* ");
			TABLA_DE_ARCHIVOS[pos].state = DELETED;
			posicion = pos;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	}

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

	return posicion;
}

int sobreescribirNombre(char *nombre, char *nuevoNombre, uint16_t parent_directory){
	log_info(logPokeDexServer, "******************************** ENTRO EN sobreescribirNombre  ******************************** ");
	int pos=0;
	osada_block_pointer posicion = 0;
	log_info(logPokeDexServer, "antes file_name: %s", nombre);
	log_info(logPokeDexServer, "antes nuevoNombre: %s", nuevoNombre);
	char *file_name = strrchr (nombre, '/') + 1;
	log_info(logPokeDexServer, "file_name: %s", file_name);

	char *nuevo_Nombre = strrchr (nuevoNombre, '/') + 1;
	log_info(logPokeDexServer, "nuevo_Nombre: %s", nuevo_Nombre);
	bool found = false;
	for (pos=0; pos <= 2047; pos++){
		char *nac;
		char *n;
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);

		//log_info(logPokeDexServer, "nac: %s", &TABLA_DE_ARCHIVOS[pos].fname);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			log_info(logPokeDexServer, "******************************* LO ENCONTRO - sobreescribirNombre! ******************************* ");

			strcpy(TABLA_DE_ARCHIVOS[pos].fname, "\0");
			strcat(TABLA_DE_ARCHIVOS[pos].fname, nuevo_Nombre);
			posicion = pos;
			found = true;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		free(nac);
		free(n);

		if (found){
			break;
		}
	}

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	if (!found){
		//NO LO ENCONTRO
		return -999;
	}

	return posicion;
}

char**  armar_vector_path(const char* text)
{
    int length_value = strlen(text) - 1;
    char* value_without_brackets = string_substring(text, 1, length_value);
    char **array_values = string_split(value_without_brackets, "/");

    free(value_without_brackets);

    return array_values;
}


int obtener_bloque_archivo(const char* path)
{
   char** vector_path = armar_vector_path(path);

   int parent_dir = 65535;
   int pos_archivo = -666;

   int j;
   int i=0;
   bool found = false;
   while (vector_path[i] != NULL)
   {
	   found = false;
	   for (j=0;j<2047;j++)
	   {
		   pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		   if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir) && (TABLA_DE_ARCHIVOS[j].state != DELETED))
		   {
			   parent_dir = j;
			   pos_archivo = j;
			   found = true;
		   }
		   pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		   if (found){
			   break;
		   }
	   }
	   i++;
   }

   if (j == 2047)
   {
	   return -666;
   }
   return pos_archivo;


}

int obtener_Nuevo_padre (const char* path)
{
	int sizeVectorPath = 0;
	char** vector_path = armar_vector_path(path);
	char *file_name = strrchr (path, '/') + 1;

	while (vector_path[sizeVectorPath] != NULL){
		sizeVectorPath++;
	}
	log_info(logPokeDexServer,"BUSCAMOS ESTO %s", path);
	log_info(logPokeDexServer,"sizeVectorPath= %d", sizeVectorPath);
	int parent_dir = 65535;

	if ( strcmp (file_name, strrchr(path, '/')) !=0 )
	{
		int i = 0;
		while (vector_path[i] != NULL){
			int j;
			for (j = 0; j <= 2047; j++){
				pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);

				if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir)){

					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - TABLA_DE_ARCHIVOS[j].fname: %s ",TABLA_DE_ARCHIVOS[j].fname);
					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i ",TABLA_DE_ARCHIVOS[j].parent_directory);

					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - vector_path[i]: %s ",vector_path[i]);
					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - parent_dir: %i ", parent_dir);
					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - Bloque Tabla j: %i ", j);
					log_info(logPokeDexServer, "-------------------------------------------------------------------------------------");
					if(sizeVectorPath != i){
						parent_dir = j;
					}

				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			i++;
		}
	}
	log_info(logPokeDexServer, "****************obtener_Nuevo_padre - RETORNADO: parent_dir: %i ", parent_dir);
	return parent_dir;
}


int obtener_bloque_padre (const char* path)
{
	int sizeVectorPath = 0;
	char** vector_path = armar_vector_path(path);
	char *file_name = strrchr (path, '/') + 1;

	while (vector_path[sizeVectorPath] != NULL){
		sizeVectorPath++;
	}

	int parent_dir = 65535;

	if ( strcmp (file_name, strrchr(path, '/')) !=0 )
	{
		int i = 0;
		while (vector_path[i] != NULL){
			int j;
			for (j = 0; j <= 2047; j++){
				pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
				if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir)){

					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].fname: %s ",TABLA_DE_ARCHIVOS[j].fname);
					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i ",TABLA_DE_ARCHIVOS[j].parent_directory);

					log_info(logPokeDexServer, "****************obtener_bloque_padre - vector_path[i]: %s ",vector_path[i]);
					log_info(logPokeDexServer, "****************obtener_bloque_padre - parent_dir: %i ", parent_dir);
					log_info(logPokeDexServer, "****************obtener_bloque_padre - Bloque Tabla j: %i ", j);
					log_info(logPokeDexServer, "-------------------------------------------------------------------------------------");

					if(sizeVectorPath>i+1){
						parent_dir = j;
					}


				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			i++;
		}
	}
	log_info(logPokeDexServer, "****************obtener_bloque_padre - RETORNADO parent_dir: %i ", parent_dir);
	return parent_dir;
}


int noEsVacio(int tamanio){
	return tamanio !=0;
}

int elTamanioDelArchivoEntraEnElOsada(int tamanio){
	log_info(logPokeDexServer, "BYTES_LIBRES: %i",BYTES_LIBRES);
	return tamanio<=BYTES_LIBRES;
}

void modificarEnLaTablaDeArchivos(int file_size, int posDelaTablaDeArchivos, int first_block){
	log_info(logPokeDexServer, "modificarEnLaTablaDeArchivos - posDelaTablaDeArchivos: %i ", posDelaTablaDeArchivos);
	log_info(logPokeDexServer, "modificarEnLaTablaDeArchivos - file_size: %i ", file_size);
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].file_size = file_size;
	if(TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].first_block==-999){
		TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].first_block = first_block;
	}
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	//TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].lastmod = 1;
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	log_info(logPokeDexServer, "modificarEnLaTablaDeArchivos - guarda osada fuera");

}
int escribirEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos){
	log_info(logPokeDexServer, "****** escribirEnLaTablaDeArchivos");
	int pos=0;
	int encontroLugar = 0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	log_info(logPokeDexServer, "file_name: %s", file_name);
	bool found = false;
    if (posDelaTablaDeArchivos == -999){//SI SE CREA EL ARCHIVO POR PRIMERA VEZ

		for (pos=0; pos <= 2047; pos++){
			//log_info(logPokeDexServer, "EN EL FOR");
			pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
			if (TABLA_DE_ARCHIVOS[pos].state == DELETED && TABLA_DE_ARCHIVOS[pos].state != REGULAR && TABLA_DE_ARCHIVOS[pos].state !=DIRECTORY){
				//log_info(logPokeDexServer, "EN EL if");
				TABLA_DE_ARCHIVOS[pos].state = REGULAR;
				//log_info(logPokeDexServer, "state");

				TABLA_DE_ARCHIVOS[pos].parent_directory = parent_directory;
				//log_info(logPokeDexServer, "parent_directory: %i",parent_directory);

				//log_info(logPokeDexServer, "fname: %s", fname);
				//log_info(logPokeDexServer, "sizeof(fname): %i", strlen(file_name));
				strcpy(TABLA_DE_ARCHIVOS[pos].fname, "\0");
				strcat(TABLA_DE_ARCHIVOS[pos].fname, file_name);

				//log_info(logPokeDexServer, "fname: %s", file_name);
				TABLA_DE_ARCHIVOS[pos].file_size = file_size;
				//log_info(logPokeDexServer, "file_size: %i",file_size);
				TABLA_DE_ARCHIVOS[pos].lastmod = 0;
				//log_info(logPokeDexServer, "lastmod");

				TABLA_DE_ARCHIVOS[pos].first_block= first_block;
				//log_info(logPokeDexServer, "first_block: %i",first_block);

				found = true;


			}
			pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

			if (found){
				break;
			}
		}//for (k=0; k <= 2047; k++)

		if(!found){
			return -1;
		}
	}
    else
	{
    	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
    	TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].file_size = file_size;
    	TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].first_block= first_block;
    	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		pos=posDelaTablaDeArchivos;
	}

	//log_info(logPokeDexServer, "k: %i", k);
	//log_info(logPokeDexServer, "tablaDeArchivo[k].fname: %s", TABLA_DE_ARCHIVOS[k].fname);
	//log_info(logPokeDexServer, "tablaDeArchivo[k].first_block: %i", TABLA_DE_ARCHIVOS[k].first_block);
    pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	log_info(logPokeDexServer, "guarda osada 2 fuera");
	return pos;

}

t_list* obtenerLosIndicesDeLosBloquesDisponiblesYGuardar(int cantBloquesDeseados){
	t_list *listDeBloques = list_create();

	int bloquesObtenidos = 0;
	int i = 0;

	log_info(logPokeDexServer, "HEADER->fs_blocks:  %i",HEADER->fs_blocks);
	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (i=0; i < fs_blocks; i++){

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i) == 0){
			list_add(listDeBloques, i);
			bloquesObtenidos++;
//			log_info(logPokeDexServer, "Bloque - %i - LIBRE",i);
			bitarray_set_bit(BITMAP, i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

		if (cantBloquesDeseados == bloquesObtenidos)
			break;

	}

	// Quitamos los nuevos bloques asignados.
	// TODO : Poner Mutex
	BYTES_LIBRES -=  bloquesObtenidos * OSADA_BLOCK_SIZE;
	BYTES_OCUPADOS +=  bloquesObtenidos * OSADA_BLOCK_SIZE;

	pthread_mutex_lock(&BITMAPmutex);
	guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
	pthread_mutex_unlock(&BITMAPmutex);

	return listDeBloques;
}


void escribirTablaDeAsignacion(int pos, int bloqueSiguiente){
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	ARRAY_TABLA_ASIGNACION[pos] = bloqueSiguiente;
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
}


void _interarBloquesQueSeranAsignados(int bloque,int hola){
	log_info(logPokeDexServer, "el proximo: %i", bloque);
}

void _prepararLaVariableGlobalParaGuadar(char* bloquePos, int bloqueSig) {

//	log_info(logPokeDexServer, "Bloque Pos: %i", atoi(bloquePos));
//	log_info(logPokeDexServer, "Bloque Sig: %i", bloqueSig);
	escribirTablaDeAsignacion(atoi(bloquePos), bloqueSig);

	//free(archivo);

}

//JOEL: NO DEBERIA USARSE MAS
void _guardarEnTablaDeDatos(char* bloquePos, unsigned char* contenido){
	//log_info(logPokeDexServer, "_guardarEnTablaDeDatos - Bloque Pos: %i", atoi(bloquePos));
	//log_info(logPokeDexServer, "_guardarEnTablaDeDatos - contenido: %s",contenido);
	int bloquePosInt = 0;
	bloquePosInt = atoi(bloquePos);
	int tamanioDelBloque = bloquePosInt *64;

	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&DATA_BLOCKSmutex);

	memcpy(&OSADA[DATA_BLOCKS+tamanioDelBloque], contenido, OSADA_BLOCK_SIZE );
	log_info(logPokeDexServer, "_guardarEnTablaDeDatos - bloqueDeDatos: %s" ,contenido);

	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	pthread_mutex_unlock(&OSADAmutex);


}



void guardarBloqueDeDatos(t_list* listado, unsigned char *contenido){
	//log_info(logPokeDexServer, "********** INICIO guardarBloqueDeDatos");

	int cantidadDeBloques = list_size(listado);
	int bloquePos;
	int i=0;

	for(i = 0; i < cantidadDeBloques; i++){

		bloquePos = list_get(listado, i);
		int tamanioDelBloque = bloquePos * OSADA_BLOCK_SIZE;

		pthread_mutex_lock(&OSADAmutex);
		memcpy(&OSADA[DATA_BLOCKS + tamanioDelBloque], &contenido[i * OSADA_BLOCK_SIZE ], OSADA_BLOCK_SIZE );
		pthread_mutex_unlock(&OSADAmutex);

	}
	//log_info(logPokeDexServer, "**********FIN guardarBloqueDeDatos");
}

int calcularCantidadDeBloquesParaGrabar(int tamanio){
	int cantidadDeBloquesParaGrabar = 0;

	if(tamanio > 0 && tamanio < OSADA_BLOCK_SIZE){
		return 1;
	}

	if(tamanio > OSADA_BLOCK_SIZE){

		cantidadDeBloquesParaGrabar = tamanio / OSADA_BLOCK_SIZE;
		int moduloTamanio=0;
		moduloTamanio = tamanio % OSADA_BLOCK_SIZE;

		if (moduloTamanio>0){
			return cantidadDeBloquesParaGrabar + 1;
		}

		if (moduloTamanio == 0){
			return cantidadDeBloquesParaGrabar;
		}

	}

	return 0;
}
t_dictionary *armarDicDeTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles){
	char *bloquePosStr=malloc(10);
	int cantidadDeElemento = 0;
	int bloquePos;
	int bloqueSig;
	int i;
	t_dictionary *dictionary = dictionary_create();
	cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);

	log_info(logPokeDexServer, "cantidadDeElemento: %i", cantidadDeElemento);
	for(i=0;i<cantidadDeElemento;i++){
		bloquePosStr = string_repeat("\0", 10);
		bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i+1);
		//log_info(logPokeDexServer, "bloquePos: %i", bloquePos);

		if(bloqueSig==0){
			bloqueSig =-1;
		}

		sprintf(bloquePosStr, "%d", bloquePos);

		dictionary_put(dictionary, bloquePosStr, bloqueSig);

		//log_info(logPokeDexServer, "bloqueSig: %i",bloqueSig);

	}
	free(bloquePosStr);
	return dictionary;
}

void modificarAgregandoBloquesEnLaTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles, int ultimoPuntero){
	bool flag = true;
	int cantidadDeElemento = 0;
	int bloquePos;
	int bloqueSig;
	int i;

	t_dictionary *dictionary = dictionary_create();
	cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);

	log_info(logPokeDexServer, "cantidadDeElemento: %i", cantidadDeElemento);
	for(i = 0; i < cantidadDeElemento; i++){


		if (i==0){
			if(ultimoPuntero!=-999){ //El ultimo puntero de un archivo nuevo se crea con -999
				bloquePos = ultimoPuntero;
				bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
			}else{
				 flag = false;
			}
		}else{
			flag = true;
			bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i-1);
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		}

		if(flag){
			char *bloquePosStr = string_new();
			string_append(&bloquePosStr,string_itoa(bloquePos));
			dictionary_put(dictionary, bloquePosStr, bloqueSig);
		}

	}

	// Al ultimo bloque utilizado le asigno el final de Archivo.
	char *bloquePosStr = string_new();
	string_append(&bloquePosStr,string_itoa(bloqueSig));
	bloqueSig =-1;
	dictionary_put(dictionary, bloquePosStr, bloqueSig);

	dictionary_iterator(dictionary, (void*) _prepararLaVariableGlobalParaGuadar);
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
}

void guardarEnLaTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles){
	modificarAgregandoBloquesEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles,0); //TODO: ESTO ESTA PARA EL CULO
}

int diferenciaEntreTamanioViejoYNuevo(int tamanioViejo, int tamanioNuevo){
	return tamanioNuevo - tamanioViejo;
}

int hayNuevosDatosParaAgregar(int tamanioViejo, int tamanioNuevo){
	return diferenciaEntreTamanioViejoYNuevo(tamanioViejo, tamanioNuevo) > 0;
}

void guardarLaMismaCantidadDeBloques(int cantidadDeBloquesParaGrabar,
		uint16_t parent_directory, int tamanioNuevo, int posDelaTablaDeArchivos,
		t_list* conjuntoDeBloquesDelArchivo, char* contenido, char* fname) {
	if (conjuntoDeBloquesDelArchivo->elements_count
			== cantidadDeBloquesParaGrabar) {
		//SI ES LA MISMA CANTIDAD DE BLOQUES, ENTONCES SOBREESCRIBO LOS BLOQUES CON EL NUEVO CONTENIDO
		log_info(logPokeDexServer, "*SI ES LA MISMA CANTIDAD DE BLOQUES, ENTONCES SOBREESCRIBO LOS BLOQUES CON EL NUEVO CONTENIDO");
		guardarBloqueDeDatos(conjuntoDeBloquesDelArchivo, contenido);
		modificarEnLaTablaDeArchivos(tamanioNuevo,posDelaTablaDeArchivos, list_get(conjuntoDeBloquesDelArchivo, 0));
	}
}


int agregarMasDatosAlArchivos(osada_file elArchivo,t_list *conjuntoDeBloquesDelArchivo,int posDelaTablaDeArchivos,char *contenido, int tamanioNuevo, char* fname,  uint16_t parent_directory){
	t_list* listadoLosIndicesDeLosBloquesDisponibles;

	int ultimoPuntero = elArchivo.first_block; //Se asigna el primer bloque, para archivos nuevos que contienen -999
	if(list_size(conjuntoDeBloquesDelArchivo)!=0){
		ultimoPuntero = list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1);
	}

	log_info(logPokeDexServer, "*********** agregarMasDatosAlArchivos - ultimoPuntero: %i", ultimoPuntero);

	int cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);
	int nuevoSize = elArchivo.file_size + tamanioNuevo;

	//log_info(logPokeDexServer, "agregarMasDatosAlArchivos_archivosGrandes - SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO");
	listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadNuevaDeBloquesParaGrabar);
	modificarAgregandoBloquesEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles, ultimoPuntero);

	guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
	modificarEnLaTablaDeArchivos(nuevoSize, posDelaTablaDeArchivos, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0));

	return list_get(listadoLosIndicesDeLosBloquesDisponibles, listadoLosIndicesDeLosBloquesDisponibles->elements_count-1);
}

void modificarUnArchivo(char *contenido, int tamanioNuevo, char* fname,  uint16_t parent_directory){
//	int cantidadNuevaDeBloquesParaGrabar = 0;
//	int posDelaTablaDeArchivos = 0;
//	t_list* listadoLosIndicesDeLosBloquesDisponibles;
//	int restaEntreNuevoTamanioYViejo = 0;
//
//	int i=0;
//
//	if(elTamanioDelArchivoEntraEnElOsada(tamanioNuevo) && noEsVacio(tamanioNuevo)){
//		osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory,&posDelaTablaDeArchivos);
//		log_info(logPokeDexServer, "El archivo size: %i", elArchivo.file_size);
//
//		osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
//
//		if (posicion == -999){
//			escribirUnArchivo(contenido, tamanioNuevo, fname, parent_directory);
//			return;
//		}
//
//		t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(posicion);
//
//		if (hayNuevosDatosParaAgregar(elArchivo.file_size, tamanioNuevo)){
//			log_info(logPokeDexServer, "************ HAY MAS DATOS PARA AGREGAR");
//			cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);
//
//			guardarLaMismaCantidadDeBloques(cantidadNuevaDeBloquesParaGrabar,
//					parent_directory, tamanioNuevo, posDelaTablaDeArchivos,
//					conjuntoDeBloquesDelArchivo, contenido, fname);
//
//			if (conjuntoDeBloquesDelArchivo->elements_count <  cantidadNuevaDeBloquesParaGrabar){
//				//*SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO\n
//				char *nuevoContenido = string_new();
//				log_info(logPokeDexServer, "*SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO");
//				listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadNuevaDeBloquesParaGrabar);
//				modificarAgregandoBloquesEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles, conjuntoDeBloquesDelArchivo);
//				nuevoContenido = string_substring(contenido, elArchivo.file_size, tamanioNuevo - elArchivo.file_size);
//				log_info(logPokeDexServer, "nuevoContenido: %s", nuevoContenido);
//				guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, nuevoContenido);
//				modificarEnLaTablaDeArchivos(tamanioNuevo, posDelaTablaDeArchivos, list_get(conjuntoDeBloquesDelArchivo, 0));
//			}
///*
//			Viejo: 65 -2bloques
//			Nuevo: 129 -3bloques
//			Desde: 129-65=64
//			*/
//
//		}else{
//			log_info(logPokeDexServer, "************ HAY MENOS DATOS PARA AGREGAR");
//			cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);
//
//			guardarLaMismaCantidadDeBloques(cantidadNuevaDeBloquesParaGrabar,
//					parent_directory, tamanioNuevo, posDelaTablaDeArchivos,
//					conjuntoDeBloquesDelArchivo, contenido, fname);
//
//			if (conjuntoDeBloquesDelArchivo->elements_count > cantidadNuevaDeBloquesParaGrabar){
//				//*SI ES MENOR LA CANTIDAD DE BLOQUES, ENTONCES SACO  LOS VIEJO BLOQUES CON EL NUEVO CONTENIDO
//				char *nuevoContenido = string_new();
//				log_info(logPokeDexServer, "*SI ES MENOR LA CANTIDAD DE BLOQUES, ENTONCES SACO  LOS VIEJO BLOQUES CON EL NUEVO CONTENIDO");
//				 t_list* sublist = list_take(conjuntoDeBloquesDelArchivo, cantidadNuevaDeBloquesParaGrabar);
//				borrarBloquesDelBitmap(conjuntoDeBloquesDelArchivo);
//				list_add_in_index(sublist, cantidadNuevaDeBloquesParaGrabar, 0);
//
//				/*
//				for(i=0; i <conjuntoDeBloquesDelArchivo->elements_count; i++ ){
//					log_info(logPokeDexServer, "conjuntoDeBloquesDelArchivo - pos: %i", list_get(conjuntoDeBloquesDelArchivo, i));
//				}
//
//				for(i=0; i < sublist->elements_count; i++ ){
//					log_info(logPokeDexServer, "sublist - pos: %i", list_get(sublist, i));
//				}
//
//
//				int posicionDondeTieneQueHaberUnCero = conjuntoDeBloquesDelArchivo->elements_count - cantidadNuevaDeBloquesParaGrabar;
//					for(i=0; i < sublist->elements_count; i++ ){
//					log_info(logPokeDexServer, "sublist - pos: %i", list_get(sublist, i));
//				}
//				*/
//
//				guardarEnLaTablaDeAsignacion(sublist);
//				//nuevoContenido = string_substring(contenido, elArchivo.file_size, tamanioNuevo);
//				//log_info(logPokeDexServer, "nuevoContenido: %s", nuevoContenido);
//				//guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, nuevoContenido);
//				modificarEnLaTablaDeArchivos(tamanioNuevo, posDelaTablaDeArchivos, list_get(conjuntoDeBloquesDelArchivo, 0));
//			}
//		}
//
//	}
//
//
//	log_info(logPokeDexServer, "************************ FIN MODIFICAR UN ARCHIVO ************************");
}

unsigned char *creoContenidoBinario(int tamanio){
	unsigned char *contenido = malloc(tamanio);
	memset(contenido, 0, tamanio);
	return contenido;

}
void bajarLosBytesDelArchivo(char *contenido, int tamanioNuevo, char* fname,  uint16_t parent_directory){
//	int cantidadNuevaDeBloquesParaGrabar = 0;
//	t_list* listadoLosIndicesDeLosBloquesDisponibles;
//	int restaEntreNuevoTamanioYViejo = 0;
//	int posDelaTablaDeArchivos = 0;
//	int i=0;
//
//	if(elTamanioDelArchivoEntraEnElOsada(tamanioNuevo) && noEsVacio(tamanioNuevo)){
//
//		osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory,&posDelaTablaDeArchivos);
//		log_info(logPokeDexServer, "bajarLosBytesDelArchivo - tamanioNuevo: %i", tamanioNuevo);
//		log_info(logPokeDexServer, "bajarLosBytesDelArchivo - El archivo size: %i", elArchivo.file_size);
//
//		osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
//		t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(posicion);
//		borrarBloquesDelBitmap(conjuntoDeBloquesDelArchivo);
//
//		log_info(logPokeDexServer, "************ bajarLosBytesDelArchivo - HAY MENOS DATOS PARA AGREGAR");
//		cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);
//		log_info(logPokeDexServer, "bajarLosBytesDelArchivo - cantidadDeBloquesParaGrabar: %i", cantidadNuevaDeBloquesParaGrabar);
//
//
//		listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar(cantidadNuevaDeBloquesParaGrabar);
//
//		guardarEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles);
//		guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
//		escribirEnLaTablaDeArchivos(parent_directory, tamanioNuevo, fname, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0), posDelaTablaDeArchivos);
//
//
//	}
//
//
//	log_info(logPokeDexServer, "************************ FIN MODIFICAR UN ARCHIVO ************************");

};

int hacerElTruncate(int tamanio, char* fname, int posDelaTablaDeArchivos, uint16_t parent_directory){
//	int cantidadDeBloquesParaGrabar = 0;
//	t_list* listadoLosIndicesDeLosBloquesDisponibles;
//	unsigned char *contenido = creoContenidoBinario(tamanio);
//
//	log_info(logPokeDexServer, "************************ FUNCION: hacerElTruncate ************************");
//
//
//	osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory,&posDelaTablaDeArchivos);
//	osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
//
//
//
//	if (posicion != -999 && elArchivo.file_size != 0){
//		log_info(logPokeDexServer, "hacerElTruncate - El archivo size: %i", elArchivo.file_size);
//		if (elArchivo.file_size  > tamanio){
//			log_info(logPokeDexServer, "hacerElTruncate - entra para truncate - elArchivo.file_size: %i, tamanio: %i", elArchivo.file_size,tamanio);
//			bajarLosBytesDelArchivo(contenido, tamanio,fname, parent_directory);
//			return 1;
//		}
//			else
//		{
//			log_info(logPokeDexServer, "hacerElTruncate -ES UN ARHIVO CON MUCHOS BLOQUES- elArchivo.file_size: %i, tamanio: %i", elArchivo.file_size,tamanio);
//			t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(posicion);
//			return agregarMasDatosAlArchivos_archivosGrandes(elArchivo,conjuntoDeBloquesDelArchivo,posDelaTablaDeArchivos, contenido, tamanio, fname,  parent_directory);
//		}
//	}
//
//
//
//	/********************************************************/
//
//	if(elTamanioDelArchivoEntraEnElOsada(tamanio) && noEsVacio(tamanio)){
//		log_info(logPokeDexServer, "hacerElTruncate - tamanio: %i", tamanio);
//
//		cantidadDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanio);
//		log_info(logPokeDexServer, "hacerElTruncate - cantidadDeBloquesParaGrabar: %i", cantidadDeBloquesParaGrabar);
//
//
//		listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadDeBloquesParaGrabar);
//
//		guardarEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles);
//		guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
//		escribirEnLaTablaDeArchivos(parent_directory, tamanio, fname, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0), posDelaTablaDeArchivos);
//
//	}
//	return list_get(listadoLosIndicesDeLosBloquesDisponibles, listadoLosIndicesDeLosBloquesDisponibles->elements_count-1);
//	log_info(logPokeDexServer, "************************ FIN hacerElTruncate ************************");
}

int escribirUnArchivo(unsigned char *contenido, int tamanio, char* fname, uint16_t parent_directory){
	int ultimoPuntero = -999;
	int posDelaTablaDeArchivos = 0;

	osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory,&posDelaTablaDeArchivos);

	log_info(logPokeDexServer, "escribirUnArchivo - El archivo size: %i", elArchivo.file_size);

	// Agrandar Archivos
	if (elArchivo.state == REGULAR){
			log_info(logPokeDexServer, "escribirUnArchivo -ES UN ARHIVO CON MUCHOS BLOQUES- elArchivo.file_size: %i, tamanio: %i", elArchivo.file_size,tamanio);
			t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(elArchivo.first_block);
			ultimoPuntero = agregarMasDatosAlArchivos(elArchivo,conjuntoDeBloquesDelArchivo,posDelaTablaDeArchivos, contenido, tamanio, fname,  parent_directory);
	}

	log_info(logPokeDexServer, "************************ FIN CREAR UN ARCHIVO ************************");
	return ultimoPuntero;
}
/************************FIN ARCHIVO************************************************/

/**************************INICIO DIRECTORIOS**************************************/
/****************LISTAR TODO *************************************************/
void mostrarLosDirectorios(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DIRECTORY){
		log_info(logPokeDexServer, "Empieza: %i****************",pos);
		log_info(logPokeDexServer, "state_%i: %c",pos, tablaDeArchivo.state);
		log_info(logPokeDexServer, "parent_directory_%i: %i",pos, tablaDeArchivo.parent_directory);
		log_info(logPokeDexServer, "fname_%i: %s",pos, &tablaDeArchivo.fname);
		log_info(logPokeDexServer, "file_size_%i: %i",pos, tablaDeArchivo.file_size);
		log_info(logPokeDexServer, "lastmod_%i: %i",pos, tablaDeArchivo.lastmod);
		log_info(logPokeDexServer, "first_block_%i: %i",pos, tablaDeArchivo.first_block);
		log_info(logPokeDexServer, "Termina: %i****************",pos);
	}
}

void mostrarLosRegulares(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == REGULAR){
		log_info(logPokeDexServer, "Empieza: %i****************",pos);
		log_info(logPokeDexServer, "state_%i: %c",pos, tablaDeArchivo.state);
		log_info(logPokeDexServer, "parent_directory_%i: %i",pos, tablaDeArchivo.parent_directory);
		log_info(logPokeDexServer, "fname_%i: %s",pos, &tablaDeArchivo.fname);
		log_info(logPokeDexServer, "file_size_%i: %i",pos, tablaDeArchivo.file_size);
		log_info(logPokeDexServer, "lastmod_%i: %i",pos, tablaDeArchivo.lastmod);
		log_info(logPokeDexServer, "first_block_%i: %i",pos, tablaDeArchivo.first_block);
		log_info(logPokeDexServer, "Termina: %i****************",pos);
	}
}

void mostrarLosBorrados(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DELETED){
		log_info(logPokeDexServer, "Empieza: %i****************",pos);
		log_info(logPokeDexServer, "state_%i: %c",pos, tablaDeArchivo.state);
		log_info(logPokeDexServer, "parent_directory_%i: %i",pos, tablaDeArchivo.parent_directory);
		log_info(logPokeDexServer, "fname_%i: %s",pos, &tablaDeArchivo.fname);
		log_info(logPokeDexServer, "file_size_%i: %i",pos, tablaDeArchivo.file_size);
		log_info(logPokeDexServer, "lastmod_%i: %i",pos, tablaDeArchivo.lastmod);
		log_info(logPokeDexServer, "first_block_%i: %i",pos, tablaDeArchivo.first_block);
		log_info(logPokeDexServer, "Termina: %i****************",pos);
	}
}

void mostrarOtrosEstados(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state != DELETED && tablaDeArchivo.state != REGULAR && tablaDeArchivo.state !=DIRECTORY){
		log_info(logPokeDexServer, "Empieza: %i****************",pos);
		log_info(logPokeDexServer, "state_%i: %c",pos, tablaDeArchivo.state);
		log_info(logPokeDexServer, "parent_directory_%i: %i",pos, tablaDeArchivo.parent_directory);
		log_info(logPokeDexServer, "fname_%i: %s",pos, &tablaDeArchivo.fname);
		log_info(logPokeDexServer, "file_size_%i: %i",pos, tablaDeArchivo.file_size);
		log_info(logPokeDexServer, "lastmod_%i: %i",pos, tablaDeArchivo.lastmod);
		log_info(logPokeDexServer, "first_block_%i: %i",pos, tablaDeArchivo.first_block);
		log_info(logPokeDexServer, "Termina: %i****************",pos);
	}
}



void dameTodosLosDirectorios(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarLosDirectorios(tablaDeArchivo[pos], pos);
	}
}

void dameTodosLosArchivosRegulares(){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		mostrarLosRegulares(TABLA_DE_ARCHIVOS[pos], pos);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	}
}

void dameTodosLosBorrados(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarLosBorrados(tablaDeArchivo[pos], pos);
	}
}

void dameTodosLosOtrosEstados(osada_file *tablaDeArchivo){
	int pos=0;
	for (pos=0; pos <= 2047; pos++){
		mostrarOtrosEstados(tablaDeArchivo[pos], pos);
	}
}
/****************FIN LISTAR TODO *************************************************/
//UN DICCIONARIO CON LISTAS PARA REPRESENTAR JERARQUIAS.

void _recorrerDirectoriosPadres(char* key,t_list *datos) {

	osada_file *archivo =  malloc(sizeof(osada_file));
	archivo = list_get(datos, 0);
	log_info(logPokeDexServer, "Diccionario - Carpeta en el root: %s", key);
	log_info(logPokeDexServer, "Un elemento de la lista list, que seria el primer hijo del root: %s", archivo->fname);
	log_info(logPokeDexServer, "archivo->parent_directory: %i", archivo->parent_directory);

	//free(archivo);

}

void reconocerDirectorio(osada_file *archivo, int pos, t_dictionary *dictionary){

	t_list *list = list_create();

	if (archivo->state == DIRECTORY  && archivo->parent_directory == 65535){
		char str[10];
		sprintf(str, "%d", pos);
		list_add(list, archivo);

		//log_info(logPokeDexServer, "pos: %s", str);

		//dictionary_put(dictionary, (char *)archivo->fname , list);
		dictionary_put(dictionary, str , list);

	}

}

t_dictionary *crearArbolDeDirectoriosDelRoot(osada_file *tablaDeArchivo){
	t_dictionary *dictionaryDirRoot = dictionary_create();
	int pos=0;

	for (pos=0; pos <= 2047; pos++){
		reconocerDirectorio(&tablaDeArchivo[pos], pos, dictionaryDirRoot);
	}

	dictionary_iterator(dictionaryDirRoot, (void*) _recorrerDirectoriosPadres);

	return dictionaryDirRoot;
}

/*****************************************************/

void reconocerDirectorioHijos(osada_file *archivo, int pos, t_dictionary *dictionaryDirRoot){


	if (archivo->state == DIRECTORY && archivo->parent_directory != 65535){
		//log_info(logPokeDexServer, "error!");
		char str[10];
		sprintf(str, "%d", archivo->parent_directory);
		//list_add(list, archivo);

		//log_info(logPokeDexServer, "pos: %i",  archivo->parent_directory);

		//dictionary_put(dictionary, (char *)archivo->fname , list);
		t_list *list = dictionary_get(dictionaryDirRoot, str);
		//log_info(logPokeDexServer, "list: %i",  list);

		if (list != 0){
			list_add(list, archivo);
			dictionary_put(dictionaryDirRoot, str , list);
		}else{

			log_info(logPokeDexServer, "Directorios sin padres: %i",  archivo->parent_directory);
			log_info(logPokeDexServer, "Sin Nombre: %s", archivo->fname);
		}


	}

}

t_dictionary *crearArbolDeDirectoriosHijos(osada_file *tablaDeArchivo, t_dictionary *dictionaryDirRoot){
	int pos=0;

	for (pos=0; pos <= 2047; pos++){
		reconocerDirectorioHijos(&tablaDeArchivo[pos], pos, dictionaryDirRoot);
	}

	//dictionary_iterator(dictionaryDirRoot, (void*) _recorrerDirectoriosPadres);

	return dictionaryDirRoot;
}
/*********************************************************************************************************/
void reconocerArchivosParaArbol(osada_file *archivo, int pos, int padre, t_list* lista){



	if (archivo->parent_directory == padre && (archivo->state == DIRECTORY || archivo->state == REGULAR)){
		log_info(logPokeDexServer, "EMPIEZA reconocerArchivosParaArbol %i: ****************", pos);
		log_info(logPokeDexServer, "state_: %c", archivo->state);
		log_info(logPokeDexServer, "parent_directory_: %i", archivo->parent_directory);
		log_info(logPokeDexServer, "fname_: %s", &archivo->fname);
		log_info(logPokeDexServer, "file_size_: %i", archivo->file_size);
		log_info(logPokeDexServer, "lastmod_: %i", archivo->lastmod);
		log_info(logPokeDexServer, "first_block_: %i", archivo->first_block);
		log_info(logPokeDexServer, "Termina reconocerArchivosParaArbol %i ****************", pos);
		list_add(lista, archivo);
	}

}

void reconocerDirectorioPadre(osada_file *archivo, int pos, int padre){
	if (archivo[padre].state == DIRECTORY  ){
		log_info(logPokeDexServer, "EMPIEZA reconocerDirectorioPadre %i: ****************", pos);
		log_info(logPokeDexServer, "state_: %c", archivo->state);
		log_info(logPokeDexServer, "parent_directory_: %i", archivo->parent_directory);
		log_info(logPokeDexServer, "fname_: %s", &archivo->fname);
		log_info(logPokeDexServer, "file_size_: %i", archivo->file_size);
		log_info(logPokeDexServer, "lastmod_: %i", archivo->lastmod);
		log_info(logPokeDexServer, "first_block_: %i", archivo->first_block);
		log_info(logPokeDexServer, "Termina reconocerDirectorioPadre %i: ****************", pos);
	}
}

t_list* crearArbolAPartirDelPadre(int padre){
	int pos=0;
	t_list* lista = list_create();

	for (pos=0; pos <= 2047; pos++){
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		reconocerArchivosParaArbol(&TABLA_DE_ARCHIVOS[pos], pos, padre, lista);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	}

	return lista;

}

void encontrarArbolPadre(int padre){
	int pos=0;

	//for (pos=0; pos <= 2047; pos++){
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	reconocerDirectorioPadre(&TABLA_DE_ARCHIVOS[pos], pos, padre);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	//}

}

/*****************************************************/
int crearUnDirectorio(char *fname){
	int k=0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	log_info(logPokeDexServer, "**********************************crearUnDirectorio - file: %s", fname);
	log_info(logPokeDexServer, "**********************************crearUnDirectorio - file_name: %s", file_name);

	int bloque = obtener_Nuevo_padre(fname);
	log_info(logPokeDexServer, "**********************************crearUnDirectorio - bloque: %i", bloque);

	bool found = false;
	for (k=0; k <= 2047; k++){
		//log_info(logPokeDexServer, "EN EL FOR");
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[k].state == DELETED && TABLA_DE_ARCHIVOS[k].state != REGULAR && TABLA_DE_ARCHIVOS[k].state != DIRECTORY){
			TABLA_DE_ARCHIVOS[k].state = DIRECTORY;

			TABLA_DE_ARCHIVOS[k].parent_directory = bloque;
			log_info(logPokeDexServer, "parent_directory: %i",bloque);

			//log_info(logPokeDexServer, "fname: %s", fname);
			strcpy(TABLA_DE_ARCHIVOS[k].fname, "\0");
			strcat(TABLA_DE_ARCHIVOS[k].fname, file_name);

			TABLA_DE_ARCHIVOS[k].file_size = 0;
			TABLA_DE_ARCHIVOS[k].lastmod = 0;
			log_info(logPokeDexServer, "lastmod");

			TABLA_DE_ARCHIVOS[k].first_block= 0;
			log_info(logPokeDexServer, "first_block: %i",0);

			found = true;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		if (found){
			break;
		}
	}//for (k=0; k <= 2047; k++)
	//log_info(logPokeDexServer, "afuera del if");



	log_info(logPokeDexServer, "k: %i", k);
	log_info(logPokeDexServer, "tablaDeArchivo[k].fname: %s", TABLA_DE_ARCHIVOS[k].fname);


	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	log_info(logPokeDexServer, "SALIO EN GUARDAR OSADA ");
	return k;
}

int borrarUnDirectorio(char *fname, uint16_t parent_directory){
	int pos=0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	log_info(logPokeDexServer, "borrarUnDirectorio - file_name: %s", file_name);
	bool found = false;
	for (pos=0; pos <= 2047; pos++){
		char *nac;
		char *n;

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].state == DIRECTORY && TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			log_info(logPokeDexServer, "EN EL if");
			TABLA_DE_ARCHIVOS[pos].state =  DELETED;
			TABLA_DE_ARCHIVOS[pos].parent_directory = -1;
			log_info(logPokeDexServer, "state");
			found = true;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		free(n);
		free(nac);

		if (found){
			break;
		}
	}

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	log_info(logPokeDexServer, "SALIO DE GUARDAR EN OSADA");
	return pos;
}

bool calcularEspacioTruncar(int actualSize,int nuevoSize) {

}
/*******************************************FIN DIRECTORIO*************************/
