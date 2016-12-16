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
    pthread_mutex_init(&listaTablaDeArchivos, NULL);
}

void destroyMutexOsada(){
	pthread_mutex_destroy(&OSADAmutex);
	pthread_mutex_destroy(&HEADERmutex);
	pthread_mutex_destroy(&BITMAPmutex);
	pthread_mutex_destroy(&DATA_BLOCKSmutex);
	pthread_mutex_destroy(&ARRAY_TABLA_ASIGNACIONmutex);
	pthread_mutex_destroy(&TABLA_DE_ARCHIVOSmutex);
	pthread_mutex_destroy(&listaTablaDeArchivos);
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
	listaTablaDeArchivos = list_create();
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (i=0; i < fs_blocks; i++){//para 150k
		t_list* listaBloquesArchivo = list_create();
		list_add(listaTablaDeArchivos,listaBloquesArchivo);
		pthread_mutex_lock(&BITMAPmutex);
		if(!bitarray_test_bit(BITMAP, i)){
			bloquesLibres++;
			//log_info(logPokeDexServer, "Bloque - %i - LIBRE",i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i)){
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
	bitMap->mode = MSB_FIRST;
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
	START_DATA_BLOCKS= (HEADER->fs_blocks - HEADER->data_blocks);
	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	//dataBlocks=  osadaHeaderFile->allocations_table_offset + tamanioQueOcupaLaTablaDeAsignacionEnBloques;

	pthread_mutex_unlock(&HEADERmutex);

	DESDE_PARA_BITMAP = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	DESDE_PARA_TABLA_DE_ARCHIVOS  = OSADA_BLOCK_SIZE + TAMANIO_DEL_BITMAP;
	DESDE_PARA_TABLA_ASIGNACION  = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS;
	DESDE_PARA_BLOQUE_DE_DATOS = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS + TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION;
	//log_info(logPokeDexServer, "desdeParaTablaAsigancion: %i",DESDE_PARA_TABLA_ASIGNACION );
	//log_info(logPokeDexServer, "desdeParaBloqueDeDatos: %i",DESDE_PARA_BLOQUE_DE_DATOS);
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

void _iterarBloques(int bloque){
	log_info(logPokeDexServer, "_iterarBloques el proximo: %i", bloque);
}


t_list *obtenerElListadoDeBloquesCorrespondientesAlArchivo(int bloqueInicial, int offsetBloque){
	int count=0;
	int elProximo = 0;
	t_list *listaDeBloques = list_create();
	if ( bloqueInicial!=-999){

		if(offsetBloque > 0){
			while (count < offsetBloque){
				pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
				elProximo = ARRAY_TABLA_ASIGNACION[bloqueInicial];
				pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
				bloqueInicial = elProximo;
				count++;
			}
		}

		if ( bloqueInicial!=-1){

			list_add(listaDeBloques, bloqueInicial);

			pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
			elProximo = ARRAY_TABLA_ASIGNACION[bloqueInicial];
			pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

			while (elProximo != -1){
				list_add(listaDeBloques, elProximo);
				bloqueInicial = elProximo;
				pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
				elProximo = ARRAY_TABLA_ASIGNACION[bloqueInicial];
				pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
			}
		}
	}
	//list_iterate(listaDeBloques, (void*) _iterarBloques);
	return listaDeBloques;
}

void borrarListadoDeBloquesCorrespondientesAlArchivo(int bloqueDesde, int bloqueHasta){
	int elProximo = 0;

	if ( bloqueDesde!=-999){
		int bloqueDeseado = bloqueDesde;

		pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
		elProximo = ARRAY_TABLA_ASIGNACION[bloqueDeseado];
		//ARRAY_TABLA_ASIGNACION[bloqueDeseado] = -1; //reseteo el bloque deseado
		pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

		while (elProximo != -1){
			bloqueDeseado = elProximo;
			char *bloqueVacio = string_repeat('\0',OSADA_BLOCK_SIZE);
			guardarEnTablaDeDatos(bloqueDeseado, bloqueVacio);
			pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
			elProximo = ARRAY_TABLA_ASIGNACION[bloqueDeseado];
			ARRAY_TABLA_ASIGNACION[bloqueDeseado] = -1; //reseteo el bloque deseado
			pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

			if(elProximo == bloqueHasta){
				break;
			}
		}
		pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
		ARRAY_TABLA_ASIGNACION[bloqueDesde] = elProximo;
		pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

	}

	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
}

void borrarListadoDeBloquesDesde(int firstBloque, int bloqueDesde){


			printf("----------------------------------------------------------------\n");
			t_list* listaImprimible = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, 0);
			int i;
			for(i=0;i<list_size(listaImprimible);i++){
				printf("%d,",list_get(listaImprimible,i));
			}
			printf("\n");
			printf("\n");
			int j;

			log_info(logPokeDexServer, "firstBloque %i",firstBloque);
			t_list* listaArch = list_get(listaTablaDeArchivos,firstBloque);
			log_info(logPokeDexServer, "list_size(listaArch) %i",list_size(listaArch));

			for(j=0;j<list_size(listaArch);j++){
				printf("%d,",list_get(listaArch,j));
			}
			printf("\n");

	int elProximo = 0;

	if ( firstBloque!=-999){
		int bloqueDeseado = firstBloque;

		pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
		elProximo = ARRAY_TABLA_ASIGNACION[firstBloque];
		pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

		while (elProximo != bloqueDesde){
			bloqueDeseado = elProximo;
			pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
			elProximo = ARRAY_TABLA_ASIGNACION[bloqueDeseado];
			pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
		}

		borrarListadoDeBloquesCorrespondientesAlArchivo(bloqueDeseado,-1);

	}
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

void borrarBloqueDelBitmap(int bloque){

	pthread_mutex_lock(&BITMAPmutex);
//	if(bitarray_test_bit(BITMAP, bloque)){
		bitarray_clean_bit(BITMAP, bloque);
//	}
	pthread_mutex_unlock(&BITMAPmutex);

	pthread_mutex_lock(&OSADAmutex);
		BYTES_LIBRES -=  OSADA_BLOCK_SIZE;
		BYTES_OCUPADOS +=  OSADA_BLOCK_SIZE;
	pthread_mutex_unlock(&OSADAmutex);

}


void ingresarElUTIMENS(uint16_t pos_archivo, uint32_t tv_sec){
	log_info(logPokeDexServer, "******************************** ENTRO EN ingresarElUTIMENS  ******************************** ");

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	memcpy(&TABLA_DE_ARCHIVOS[pos_archivo].lastmod,&tv_sec,sizeof(tv_sec));
	log_info(logPokeDexServer, "TABLA_DE_ARCHIVOS[pos].lastmod = tv_sec %d",TABLA_DE_ARCHIVOS[pos_archivo].lastmod);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	log_info(logPokeDexServer, "TABLA_DE_ARCHIVOS[pos].lastmod = tv_sec %d",TABLA_DE_ARCHIVOS[pos_archivo].lastmod);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

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

void sobreescribirNombre(char *nombre, char *nuevoNombre){
	//NOTA aca el nuevo archivo siempre llega sabiendo que el padre del mismo existe por lo cual me olvido de validarlo porque ya lo hizo el fuse
	log_info(logPokeDexServer, "******************************** ENTRO EN sobreescribirNombre  ******************************** ");
	int posArchivoViejo=-1;
	int posArchivoNuevo=-1;
	log_info(logPokeDexServer, "old file_name: %s", nombre);
	log_info(logPokeDexServer, "nuevo file_name: %s", nuevoNombre);
	char * newFileName = strrchr (nuevoNombre, '/') + 1;
	log_info(logPokeDexServer, "nuevoNombre: %s", newFileName);

	//get padre from path received
	uint16_t parent_directoryOldFile = obtener_bloque_padre_NUEVO(nombre, &posArchivoViejo);
	uint16_t parent_directoryNewFile = obtener_bloque_padre_NUEVO(nuevoNombre, &posArchivoNuevo);

	if (parent_directoryOldFile != parent_directoryNewFile){
		parent_directoryOldFile = parent_directoryNewFile;// asigno nuevo padre al viejo si eran distintos
	}

	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	memset(TABLA_DE_ARCHIVOS[posArchivoViejo].fname, 0, OSADA_FILENAME_LENGTH); //reseteo nombre
	memcpy(TABLA_DE_ARCHIVOS[posArchivoViejo].fname, newFileName, OSADA_FILENAME_LENGTH); //compio nuevo nobre
	TABLA_DE_ARCHIVOS[posArchivoViejo].parent_directory = parent_directoryOldFile;
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

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
   char *file_name = strrchr (path, '/') + 1;
   int parent_dir = 65535;
   int pos_archivo = -666;

   int j;
   int i=0;
   bool found = false;
   while (vector_path[i] != NULL)
   {
	   found = false;
	   for (j=0;j<=2047;j++)
	   {
		   pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		   if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir) && (TABLA_DE_ARCHIVOS[j].state != DELETED)){
			   parent_dir = j;
			   if (strcmp(file_name, TABLA_DE_ARCHIVOS[j].fname) == 0){
				   pos_archivo = j;
				   found = true;
			   }
		   }
		   pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		   if (found){
			   break;
		   }
	   }
	   i++;
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

	int parent_dir = 65535;

	if ( strcmp (file_name, strrchr(path, '/')) !=0 )
	{
		int i = 0;
		while (vector_path[i] != NULL){
			int j;
			for (j = 0; j <= 2047; j++){
				pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);

				if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir)){

//					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - TABLA_DE_ARCHIVOS[j].fname: %s ",TABLA_DE_ARCHIVOS[j].fname);
//					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i ",TABLA_DE_ARCHIVOS[j].parent_directory);
//
//					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - vector_path[i]: %s ",vector_path[i]);
//					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - parent_dir: %i ", parent_dir);
//					log_info(logPokeDexServer, "****************obtener_Nuevo_padre - Bloque Tabla j: %i ", j);
//					log_info(logPokeDexServer, "-------------------------------------------------------------------------------------");
					if(sizeVectorPath != i){
						parent_dir = j;
					}

				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			i++;
		}
	}
//	log_info(logPokeDexServer, "****************obtener_Nuevo_padre - RETORNADO: parent_dir: %i ", parent_dir);
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

//					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].fname: %s ",TABLA_DE_ARCHIVOS[j].fname);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i ",TABLA_DE_ARCHIVOS[j].parent_directory);
//
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - vector_path[i]: %s ",vector_path[i]);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - parent_dir: %i ", parent_dir);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - Bloque Tabla j: %i ", j);
//					log_info(logPokeDexServer, "-------------------------------------------------------------------------------------");

					if(sizeVectorPath>i+1){
						parent_dir = j;
					}


				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			i++;
		}
	}
//	log_info(logPokeDexServer, "****************obtener_bloque_padre - RETORNADO parent_dir: %i ", parent_dir);
	return parent_dir;
}

int obtener_bloque_padre_NUEVO (const char* path,int* posArchivo)
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

//					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].fname: %s ",TABLA_DE_ARCHIVOS[j].fname);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i ",TABLA_DE_ARCHIVOS[j].parent_directory);
//
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - vector_path[i]: %s ",vector_path[i]);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - parent_dir: %i ", parent_dir);
//					log_info(logPokeDexServer, "****************obtener_bloque_padre - Bloque Tabla j: %i ", j);
//					log_info(logPokeDexServer, "-------------------------------------------------------------------------------------");

					if(sizeVectorPath>i+1){
						parent_dir = j;
					}

					*posArchivo  = j;
				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			free(vector_path[i]); //Libero el que ya lei
			i++;
		}
	}
//	log_info(logPokeDexServer, "****************obtener_bloque_padre - RETORNADO parent_dir: %i ", parent_dir);
	//Liberlo lo que declare en la funcion.
	free(vector_path);
	return parent_dir;
}

int noEsVacio(int tamanio){
	return tamanio !=0;
}

int elTamanioDelArchivoEntraEnElOsada(int tamanio){
	log_info(logPokeDexServer, "BYTES_LIBRES: %i",BYTES_LIBRES);
	return tamanio<=BYTES_LIBRES;
}

void modificarEnLaTablaDeArchivos(int size, int posDelaTablaDeArchivos, int first_block){
//	log_info(logPokeDexServer, "modificarEnLaTablaDeArchivos - posDelaTablaDeArchivos: %i ", posDelaTablaDeArchivos);
//	log_info(logPokeDexServer, "modificarEnLaTablaDeArchivos - file_size: %i ", file_size);
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].file_size = size;
	if(TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].first_block==-999 || (first_block == -999)){
		//esto es para cuando el archivo es nuevo y asigno el primer bloque o cuando borro el archivo y dejo el primer bloque en -999
		TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].first_block = first_block;
	}
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	//TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].lastmod = 1;
}


int escribirEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos){
	int pos=0;
	int encontroLugar = 0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJOobtener_bloque_padre_NUEVO
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

	//log_info(logPokeDexServer, "HEADER->fs_blocks:  %i",HEADER->fs_blocks);
	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (i=START_DATA_BLOCKS; i < fs_blocks; i++){//SIEMPRE ARRANCO DESDE EL PRIMER BLOQUE DE DATOS

		pthread_mutex_lock(&BITMAPmutex);
		if(!bitarray_test_bit(BITMAP, i)){
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
	pthread_mutex_lock(&OSADAmutex);
		BYTES_LIBRES -=  bloquesObtenidos * OSADA_BLOCK_SIZE;
		BYTES_OCUPADOS +=  bloquesObtenidos * OSADA_BLOCK_SIZE;
	pthread_mutex_unlock(&OSADAmutex);


	pthread_mutex_lock(&BITMAPmutex);
	guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
	pthread_mutex_unlock(&BITMAPmutex);

	return listDeBloques;
}


void escribirTablaDeAsignacion(int pos, int bloqueSiguiente){

	ARRAY_TABLA_ASIGNACION[pos] = bloqueSiguiente;

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

void guardarEnTablaDeDatos(int bloquePos, unsigned char* contenido){
	//log_info(logPokeDexServer, "_guardarEnTablaDeDatos - Bloque Pos: %i", atoi(bloquePos));
	//log_info(logPokeDexServer, "_guardarEnTablaDeDatos - contenido: %s",contenido);
	int offsetDelBloque = bloquePos *64;

	pthread_mutex_lock(&OSADAmutex);
	memcpy(&OSADA[offsetDelBloque], contenido, OSADA_BLOCK_SIZE );
	pthread_mutex_unlock(&OSADAmutex);
//	log_info(logPokeDexServer, "_guardarEnTablaDeDatos - bloqueDeDatos: %s" ,contenido);

}



void guardarBloqueDeDatos(t_list* listado, unsigned char *contenido, int size){
	//log_info(logPokeDexServer, "********** INICIO guardarBloqueDeDatos");
	int cantBloqConte = (size / OSADA_BLOCK_SIZE);
	int off_bloque = (size % OSADA_BLOCK_SIZE);
	if(off_bloque>0){
		cantBloqConte++;
	}
	int bloquePos;
	int i=0;

	if(list_size(listado)<cantBloqConte){ // Si la cantidad de bloques reservados es menor a la solicitada por FUSE
		cantBloqConte=list_size(listado);
	}

	for(i = 0; i < cantBloqConte; i++){

		bloquePos = list_get(listado, i);
		int offsetDelBloque = bloquePos * OSADA_BLOCK_SIZE;
		pthread_mutex_lock(&OSADAmutex);
		memcpy(&OSADA[offsetDelBloque], &contenido[i * OSADA_BLOCK_SIZE ], OSADA_BLOCK_SIZE );
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


void modificarAgregandoBloquesEnLaTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles, int ultimoPuntero, int firstBloque){
	// Agrego a lista de Archivos
	t_list* bloquesAsociados;
	t_list* listaParalela = list_create();
	list_add_all(listaParalela,listadoLosIndicesDeLosBloquesDisponibles);

	if(ultimoPuntero!=-999){
		pthread_mutex_lock(&lista_bloq_archivosmutex);
		bloquesAsociados = list_get(listaTablaDeArchivos,firstBloque);
		pthread_mutex_unlock(&lista_bloq_archivosmutex);

	}else{ //Es mi primer bloque de asigancion.
		ultimoPuntero = list_remove(listaParalela,0);
		pthread_mutex_lock(&lista_bloq_archivosmutex);
		bloquesAsociados = list_get(listaTablaDeArchivos,ultimoPuntero);
		pthread_mutex_unlock(&lista_bloq_archivosmutex);
	}

	pthread_mutex_lock(&lista_bloq_archivosmutex);
	list_add_all(bloquesAsociados,listadoLosIndicesDeLosBloquesDisponibles);
	pthread_mutex_unlock(&lista_bloq_archivosmutex);
	//Linkeo en Tabla de Archivos
	bool flag = true;
	int cantidadDeElemento = 0;
	int bloquePos;
	int bloqueSig;
	int i;

	cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);
	log_info(logPokeDexServer,"cantidad de elementos en bloques disponibles: %d",cantidadDeElemento);
	for(i = 0; i < cantidadDeElemento; i++){

		if (i==0){
			if(ultimoPuntero!=-999){ //El ultimo puntero de un archivo nuevo se crea con -999
				bloquePos = ultimoPuntero;
				bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
			}else{
				bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
				flag = false;
			}
		}else{
			flag = true;
			bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i-1);
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		}

		if(flag){
			pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
			ARRAY_TABLA_ASIGNACION[bloquePos] = bloqueSig;
			pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
		}

	}
	log_info(logPokeDexServer,"F cantidad de elementos en bloques disponibles");
	// Al ultimo bloque utilizado le asigno el final de Archivo.
	bloquePos =bloqueSig;
	bloqueSig =-1;
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	ARRAY_TABLA_ASIGNACION[bloquePos] = bloqueSig;
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
	log_info(logPokeDexServer,"F list_destroy");

}


int diferenciaEntreTamanioViejoYNuevo(int tamanioViejo, int tamanioNuevo){
	return tamanioNuevo - tamanioViejo;
}

int hayNuevosDatosParaAgregar(int tamanioViejo, int tamanioNuevo){
	return diferenciaEntreTamanioViejoYNuevo(tamanioViejo, tamanioNuevo) > 0;
}


int agregarMasDatosAlArchivos(int firstBloque,int posDelaTablaDeArchivos, int tamanioNuevo){
	int exitCode = 0;//por default retorna OK
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	//t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, 0);

	t_list *conjuntoDeBloquesDelArchivo;
	if(firstBloque != -999){
		pthread_mutex_lock(&lista_bloq_archivosmutex);
		conjuntoDeBloquesDelArchivo = list_get(listaTablaDeArchivos, firstBloque);
		pthread_mutex_unlock(&lista_bloq_archivosmutex);
	}else{
		conjuntoDeBloquesDelArchivo = list_create();
	}

	int ultimoPuntero = firstBloque;
	pthread_mutex_lock(&lista_bloq_archivosmutex);
	if(list_size(conjuntoDeBloquesDelArchivo)!=0){
		ultimoPuntero = list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1);
	}
	pthread_mutex_unlock(&lista_bloq_archivosmutex);

	//(logPokeDexServer, "*********** agregarMasDatosAlArchivos - ultimoPuntero: %i", ultimoPuntero);

	int cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);

	listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadNuevaDeBloquesParaGrabar);

	if (list_size(listadoLosIndicesDeLosBloquesDisponibles)> 0){
		modificarAgregandoBloquesEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles, ultimoPuntero, firstBloque);

		char *bloqueVacio = string_repeat('\0',OSADA_BLOCK_SIZE);
		int i;
		for(i=0;i<list_size(listadoLosIndicesDeLosBloquesDisponibles);i++){
			int bloque = list_get(listadoLosIndicesDeLosBloquesDisponibles,i);
			guardarEnTablaDeDatos(bloque,bloqueVacio);
		}
		free(bloqueVacio);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		tamanioNuevo +=  TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].file_size;
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

		modificarEnLaTablaDeArchivos(tamanioNuevo, posDelaTablaDeArchivos, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0));
	}else{
		//verBitmap();
		exitCode = -1;//no hay mas bloques disponibles
		log_info(logPokeDexServer, "*********** agregarMasDatosAlArchivos - no hay mas bloques disponibles");
	}

	return exitCode;
}

void verBitmap(){
	int contador1 = 0;
	int espacio =0;

	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (contador1=START_DATA_BLOCKS; contador1 < fs_blocks; contador1++){//SIEMPRE ARRANCO DESDE EL PRIMER BLOQUE DE DATOS

		pthread_mutex_lock(&BITMAPmutex);
		log_info(logPokeDexServer, "Bloque - %i - estado: %d",contador1, bitarray_test_bit(BITMAP, contador1));
		pthread_mutex_unlock(&BITMAPmutex);

	}
}

unsigned char *creoContenidoBinario(int tamanio){
	unsigned char *contenido = malloc(tamanio);
	memset(contenido, 0, tamanio);
	return contenido;

}

int hacerElTruncate(int offset, char* path,int* pos_archivo){
	*pos_archivo= obtener_bloque_archivo(path);

	//Achicar archivo
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	osada_file elArchivo = TABLA_DE_ARCHIVOS[*pos_archivo];
	int fileSize = elArchivo.file_size;
	int firstBloque = elArchivo.first_block;
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

	if(fileSize > offset)	{

		log_info(logPokeDexServer, "FUSE_TRUNCATE - Achicar archivo");
		int cant_bloques = (offset / OSADA_BLOCK_SIZE);
		int off_bloque = (offset % OSADA_BLOCK_SIZE);

		t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, 0);//TRAEMOS el total de bloques del archivo

		if(offset == 0) {//borro el archivo completo
			log_info(logPokeDexServer, "FUSE_TRUNCATE - borro el archivo completo");
			int i;

			for(i=0; i < list_size(conjuntoDeBloquesDelArchivo);i++){ //I= 0 BORRO TODOS LOS BLOQUES OCUPADOS
				borrarBloqueDelBitmap(list_get(conjuntoDeBloquesDelArchivo, i));
			}
			//Actualizo la ListaDe
			t_list* bloquesArchivos = list_get(listaTablaDeArchivos,firstBloque);
			list_clean(bloquesArchivos);

			pthread_mutex_lock(&BITMAPmutex);
			guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
			pthread_mutex_unlock(&BITMAPmutex);

			borrarListadoDeBloquesCorrespondientesAlArchivo(firstBloque, -1); // -1 --> porque tiene que ir hasta el final de bloques del archivo

			//DESPUES DE BORRRAR TODOS LOS BLOQUES DE DATOS borro el primero
			char *bloqueVacio = string_repeat('\0',OSADA_BLOCK_SIZE);
			guardarEnTablaDeDatos(firstBloque, bloqueVacio);

			fileSize = 0;
			//DESPUES DE BORRAR TODOS LOS BLOQUES le asigno -999 al first block del archivo
			modificarEnLaTablaDeArchivos(fileSize,*pos_archivo,-999);//-999 --> esto es porque mi primer bloque ahora debe ser vacio

			return 0;
		}

		if((cant_bloques == 0) && (off_bloque > 0)){//Menos de un bloque
			log_info(logPokeDexServer, "FUSE_TRUNCATE - borro Menos de un bloque");
			int i;

			for(i=1; i < list_size(conjuntoDeBloquesDelArchivo);i++){// i=1 DEJA EL PRIMER BLOQUE OCUPADO
				borrarBloqueDelBitmap(list_get(conjuntoDeBloquesDelArchivo, i));
			}

			//Actualizo la ListaDe
			t_list* bloquesArchivos = list_get(listaTablaDeArchivos,firstBloque);
			list_clean(bloquesArchivos);
			list_add(bloquesArchivos,firstBloque);

			pthread_mutex_lock(&BITMAPmutex);
			guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
			pthread_mutex_unlock(&BITMAPmutex);

			borrarListadoDeBloquesCorrespondientesAlArchivo(firstBloque, -1); // -1 --> porque tiene que ir hasta el final de bloques del archivo

			fileSize = OSADA_BLOCK_SIZE;

			modificarEnLaTablaDeArchivos(fileSize,*pos_archivo, firstBloque);

			return 0;
		}

		if(cant_bloques > 0){ //1 Bloque entero y un poco mas || N Bloques enteros y un poco mas
			log_info(logPokeDexServer, "FUSE_TRUNCATE - borro 1 Bloque entero y un poco mas || N Bloques enteros y un poco mas");
			int nuevaCantidadBloques = cant_bloques;

			if(off_bloque > 0) {//Valido si estoy en la mitad del bloque, si es asi, paso al siguiente
				log_info(logPokeDexServer, "Valido si estoy en la mitad del bloque, si es asi, paso al siguiente");
				nuevaCantidadBloques++;
				if(list_size(conjuntoDeBloquesDelArchivo)<nuevaCantidadBloques){
					log_info(logPokeDexServer, "Retorno Error si la cantidad de bloques a eliminar es mayor al archivo actual.");
					return -1; // Retorno Error si la cantidad de bloques a eliminar es mayor al archivo actual.
				}
			}

			conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, nuevaCantidadBloques);

			int i;
			for(i=0; i < list_size(conjuntoDeBloquesDelArchivo);i++){
				borrarBloqueDelBitmap(list_get(conjuntoDeBloquesDelArchivo, i));
			}

			//Tomo los primeros elementos de la lista
			t_list* bloqueRemanente=  list_create();
			t_list* bloquesArchivos = list_get(listaTablaDeArchivos,firstBloque);
			for(i=0;i<nuevaCantidadBloques;i++){
				list_add(bloqueRemanente,list_get(bloquesArchivos,i));
			}
			list_clean(bloquesArchivos);
			list_add_all(bloquesArchivos,bloqueRemanente);

			pthread_mutex_lock(&BITMAPmutex);
			guardarEnOsada(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);
			pthread_mutex_unlock(&BITMAPmutex);

			int bloqueDesde = list_get(conjuntoDeBloquesDelArchivo, 0);
			borrarListadoDeBloquesDesde(firstBloque, bloqueDesde); // -1 --> porque tiene que ir hasta el final de bloques del archivo
			fileSize = nuevaCantidadBloques*OSADA_BLOCK_SIZE;

			modificarEnLaTablaDeArchivos(fileSize,*pos_archivo, firstBloque);
			return 0;
		}
	}//Hasta aca chequeado

	//Agrandar archivo
	if(fileSize < offset){
//		log_info(logPokeDexServer, "FUSE_TRUNCATE - Agrandar archivo");
		int bytes_por_reservar;
		if (fileSize == 0) {//el archivo es nuevo
			bytes_por_reservar = offset;
		} else { //filesize > 0 El archivo ya tiene datos grabados
			bytes_por_reservar = offset - fileSize;
		}
		int exitCode = agregarMasDatosAlArchivos(firstBloque,*pos_archivo, bytes_por_reservar);
		return exitCode;
	}

	return 0;

}

int escribirUnArchivo(unsigned char *contenido, int size, char* fname, int offset){
	time_t tiempo1 = time(0);
	int offsetBloque = 0;
	int pos_archivo=0;
	int ultimoPuntero = hacerElTruncate(size + offset, fname, &pos_archivo);

	if(ultimoPuntero != -1){// -1 ES CUANDO EL DISCO ESTA LLENO
		if(offset!=0){
			offsetBloque = (offset / OSADA_BLOCK_SIZE);
		}

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		int firstBloque = TABLA_DE_ARCHIVOS[pos_archivo].first_block;
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

		t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, offsetBloque);
		// Agrandar Archivos
		guardarBloqueDeDatos(conjuntoDeBloquesDelArchivo,contenido,size);
		ultimoPuntero =  list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1);
		time_t tiempo2 = time(0);
		double segsSinResponder = difftime(tiempo2, tiempo1);
		log_info(logPokeDexServer, "Tiempo escribirUnArchivo %f | Size %i",segsSinResponder,size + offset);

// verifica estado de TABLA de ARCHIVOS contra Lista de bloques de archivos
//		printf("----------------------------------------------------------------\n");
//		t_list* listaImprimible = obtenerElListadoDeBloquesCorrespondientesAlArchivo(firstBloque, 0);
//		int i;
//		for(i=0;i<list_size(listaImprimible);i++){
//			printf("%d,",list_get(listaImprimible,i));
//		}
//		printf("\n");
//		printf("\n");
//		int j;
//
//		log_info(logPokeDexServer, "firstBloque %i",firstBloque);
//		t_list* listaArch = list_get(listaTablaDeArchivos,firstBloque);
//		log_info(logPokeDexServer, "list_size(listaArch) %i",list_size(listaArch));
//
//		for(j=0;j<list_size(listaArch);j++){
//			printf("%d,",list_get(listaArch,j));
//		}
//		printf("\n");

	}

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
			memset(TABLA_DE_ARCHIVOS[k].fname, 0, OSADA_FILENAME_LENGTH);
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

int borrarUnDirectorio(char *fname){
	int pos=-1;
	int exitCode = 1;//error por default

	//get padre from path
	uint16_t parent_directory = obtener_bloque_padre_NUEVO(fname, &pos);

	char *file_name = strrchr (fname, '/') + 1;
	log_info(logPokeDexServer, "borrarUnDirectorio - file_name: %s", file_name);

	if(pos !=-1 ){
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		TABLA_DE_ARCHIVOS[pos].state =  DELETED;
		TABLA_DE_ARCHIVOS[pos].parent_directory = -1;
		guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		exitCode = 0;
	}

	return exitCode;
}

int buscar_nodo_vacio ()
{
	int i;
	for (i = 0; i <= 2047; i++)	{
		if (TABLA_DE_ARCHIVOS[i].state == DELETED){
			return i;
		}
	}
	return -1;
}

int inicializarNuevoArchivo( char* path){
	int posFree;
	int posArchivo;
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	posFree = buscar_nodo_vacio ();
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

	if (posFree != -1)	{

		int bloquePadre = obtener_bloque_padre_NUEVO(path,&posArchivo);
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);

		TABLA_DE_ARCHIVOS[posFree].state = REGULAR;
		TABLA_DE_ARCHIVOS[posFree].lastmod = 0;
		char * newFileName = strrchr (path, '/') + 1;
		memset(TABLA_DE_ARCHIVOS[posFree].fname, 0, OSADA_FILENAME_LENGTH); //reseteo nombre
		memcpy(TABLA_DE_ARCHIVOS[posFree].fname, newFileName, OSADA_FILENAME_LENGTH); //compio nuevo nobre

		TABLA_DE_ARCHIVOS[posFree].file_size = 0;
		TABLA_DE_ARCHIVOS[posFree].first_block=-999;
		TABLA_DE_ARCHIVOS[posFree].parent_directory = bloquePadre; //&posArchivo es condicion del metodo no es necesario

		guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);

		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	}

	return posFree;
}
