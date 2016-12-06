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
	printf("iniciio guardarEnOsada\n");
	pthread_mutex_lock(&OSADAmutex);
	memcpy(&OSADA[desde], elemento, tamaniaDelElemento);
	pthread_mutex_unlock(&OSADAmutex);
	printf("fin guardarEnOsada\n");
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
	printf("Array tabla asignada: %i\n",asignado);
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
	printf("Empieza: %i****************\n",pos);
	printf("state_%i: %c\n",pos, tablaDeArchivo.state);
	printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
	printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
	printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
	printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
	printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
	printf("Termina: %i****************\n",pos);
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
			//printf("Bloque - %i - LIBRE\n",i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i) == 1){
			bloquesOcupados++;
			//printf("Bloque - %i - OCUPADO\n",i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

	}
	//printf("Bloques Ocupados: %i\n",bloquesOcupados);
	//printf("Bloques Libres: %i\n",bloquesLibres);
	//printf("bytes libres: %i\n",bloquesLibres*64);
	BYTES_LIBRES = bloquesLibres*64;
	BYTES_OCUPADOS = bloquesOcupados*64;
	printf("kb libres: %d\n", (bloquesLibres*64)/1024);

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
	printf("magic_number 2: %s\n",  osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %s\n",   osadaHeaderFile->padding);
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
	printf("ruta: %s\n", ruta);
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

	printf("HEADER->fs_blocks - HEADER->data_blocks: %i\n",HEADER->fs_blocks - HEADER->data_blocks);
	pthread_mutex_unlock(&HEADERmutex);
	printf("dataBlocks: %i\n",DATA_BLOCKS);

	DESDE_PARA_BITMAP = OSADA_BLOCK_SIZE;//LO QUE OCUPA EL HEADER
	DESDE_PARA_TABLA_DE_ARCHIVOS  = OSADA_BLOCK_SIZE + TAMANIO_DEL_BITMAP;
	DESDE_PARA_TABLA_ASIGNACION  = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS;
	DESDE_PARA_BLOQUE_DE_DATOS = TAMANIO_QUE_OCUPA_EL_HEADER + TAMANIO_DEL_BITMAP + TAMANIO_TABLA_DE_ARCHIVOS + TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION;
	printf("desdeParaTablaAsigancion: %i\n",DESDE_PARA_TABLA_ASIGNACION );
	printf("desdeParaBloqueDeDatos: %i\n",DESDE_PARA_BLOQUE_DE_DATOS);
}


unsigned char *inicializarOSADA(int archivoID){
	unsigned char *osada;
	setlocale(LC_ALL, "es_ES.UTF-8");
	/************************************************************/
	printf("Locale is: %s\n", setlocale(LC_ALL, "es_ES.UTF-8"));
	printf("Que paso?: %s\n", strerror(errno));
	printf("archivoID: %i\n", archivoID);
	printf("tamanio: %i\n", TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES);
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
	//printf("%i\n", dataBlocks);
	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&DATA_BLOCKSmutex);
	memcpy(bloqueDeDatos, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );
	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	pthread_mutex_unlock(&OSADAmutex);

//	for(i=1; i<=64; i++){
	bloqueDeDatos[OSADA_BLOCK_SIZE + 1] = '\0';
		printf("%s", bloqueDeDatos);
	//}
	//printf("\nTERMINO\n");
	free(bloqueDeDatos);



}

void verContenidoDeArchivo(t_list *conjuntoDeBloques){
	list_iterate(conjuntoDeBloques, (void*) _iterarParaVerContenido);
}

void _iterarBloques(int bloque){
	printf("_iterarBloques el proximo: %i\n", bloque);
}



t_list *crearPosicionesDeBloquesParaUnArchivo(int numeroBloques){
	int elProximo = 0;
	t_list *proximo = list_create();

	list_add(proximo, numeroBloques);

	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	elProximo = ARRAY_TABLA_ASIGNACION[numeroBloques];
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);

	while (elProximo != -1){
		list_add(proximo, elProximo);
		numeroBloques = elProximo;
		pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
		elProximo = ARRAY_TABLA_ASIGNACION[numeroBloques];
		pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
	}

	list_iterate(proximo, (void*) _iterarBloques);

	return proximo;
}

osada_block_pointer comprobarElNombreDelArchivo(osada_file tablaDeArchivo, uint16_t parent_directory, char *nombre){
	char *tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim;
	char *n;
	tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim = string_duplicate(&tablaDeArchivo.fname);
	n = string_duplicate(nombre);
	string_trim(&tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	string_trim(&n);
	/*
	printf("tablaDeArchivo.parent_directory: %i\n", tablaDeArchivo.parent_directory);
	printf("parent_directory_: %i\n", parent_directory);
	printf("nac: %s\n",tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	printf("n: %s\n",n);
	*/
	if (tablaDeArchivo.parent_directory == parent_directory && tablaDeArchivo.state == REGULAR && strcmp(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim, n) == 0){
		free(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
		free(n);
		/*
		printf("state_: %c\n", tablaDeArchivo.state);
		printf("parent_directory_: %i\n", tablaDeArchivo.parent_directory);
		printf("fname_: %s\n",&tablaDeArchivo.fname);
		printf("file_size_: %i\n",tablaDeArchivo.file_size);
		printf("lastmod_: %i\n", tablaDeArchivo.lastmod);
		printf("first_block_: %i\n",tablaDeArchivo.first_block);
		*/
		return tablaDeArchivo.first_block;
	}

	free(tablaDeArchivoNombreDeArchivoParaSerLimpiadoEnElTrim);
	free(n);
	return -666;
}



osada_block_pointer devolverOsadaBlockPointer(char *nombre, uint16_t parent_directory){
	printf("******************************** ENTRO EN EL devolverOsadaBlockPointer ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	printf("file_name: %s\n", file_name);
	bool found = false;
	for (pos=0; pos <= 2047; pos++){

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if ((posicion = comprobarElNombreDelArchivo(TABLA_DE_ARCHIVOS[pos], parent_directory,  file_name)) != -666){
			printf("devolverOsadaBlockPointer - Encontro archivo - pos:  %i\n", pos);
			found = true;
		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

		if (found){
			return posicion;
		}
	}

	printf("******************************* devolverOsadaBlockPointer- NO LO ENCONTRO! ******************************* \n");
	return posicion;
}

int buscarElArchivoYDevolverPosicion(char *nombre, uint16_t parent_directory){
	printf("******************************** ENTRO EN buscarElArchivo  ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	//printf("file_name: %s\n", file_name);
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
		//printf("nac: %s\n", &TABLA_DE_ARCHIVOS[pos].fname);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			printf("******************************* buscarElArchivoYDevolverPosicion - LO ENCONTRO! ******************************* \n");
			found = true;
		}else if(TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) != 0 && pos == 2047){
			pos =-666;
			printf("******************************* buscarElArchivoYDevolverPosicion - NO LO ENCONTRO! ******************************* \n");

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

osada_file buscarElArchivoYDevolverOsadaFile(char *nombre, uint16_t parent_directory){
	printf("******************************** ENTRO EN buscarElArchivoYDevolverOsadaFile  ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	//printf("file_name: %s\n", file_name);
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
		//printf("nac: %s\n", &TABLA_DE_ARCHIVOS[pos].fname);
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			printf("*******************************buscarElArchivoYDevolverOsadaFile - LO ENCONTRO! ******************************* \n");
			found = true;
		}else if(TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) != 0 && pos == 2047){
			printf("******************************* buscarElArchivoYDevolverOsadaFile -NO LO ENCONTRO! ******************************* \n");

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		free(nac);
		free(n);

		if(found){
			break;
		}
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
	printf("******************************** ENTRO EN borrarUnArchivo  ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	printf("file_name: %s\n", file_name);

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
			printf("******************************* LO ENCONTRO! ******************************* \n");
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
	printf("******************************** ENTRO EN borrarUnArchivo  ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	printf("file_name: %s\n", file_name);

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
			printf("******************************* LO ENCONTRO! ******************************* \n");
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
	printf("******************************** ENTRO EN sobreescribirNombre  ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	printf("antes file_name: %s\n", nombre);
	printf("antes nuevoNombre: %s\n", nuevoNombre);
	char *file_name = strrchr (nombre, '/') + 1;
	printf("file_name: %s\n", file_name);

	char *nuevo_Nombre = strrchr (nuevoNombre, '/') + 1;
	printf("nuevo_Nombre: %s\n", nuevo_Nombre);
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

		//printf("nac: %s\n", &TABLA_DE_ARCHIVOS[pos].fname);

		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			printf("******************************* LO ENCONTRO - sobreescribirNombre! ******************************* \n");

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
	   for (j=0; j<= 2047; j++)
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

int obtener_bloque_padre (const char* path)
{
	char** vector_path = armar_vector_path(path);
	char *file_name = strrchr (path, '/') + 1;

	int parent_dir = 65535;

	if ( strcmp (file_name, strrchr(path, '/')) !=0 )
	{
		int i = 0;
		while (vector_path[i] != NULL)
		{
			int j;
			for (j = 0; j <= 2047; j++)
			{
				pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
				if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir))
				{
					/*
					printf("****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].fname: %s \n",TABLA_DE_ARCHIVOS[j].fname);
					printf("****************obtener_bloque_padre - vector_path[i]: %s \n",vector_path[i]);
					printf("****************obtener_bloque_padre - TABLA_DE_ARCHIVOS[j].parent_directory: %i \n",TABLA_DE_ARCHIVOS[j].parent_directory);
					printf("****************obtener_bloque_padre - parent_dir: %i \n", parent_dir);
					printf("****************obtener_bloque_padre - j: %i \n", j);
					 */
					parent_dir = j;

				}
				pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
			}
			i++;
		}
	}
	printf("****************obtener_bloque_padre - parent_dir: %i \n", parent_dir);
	return parent_dir;
}


int noEsVacio(int tamanio){
	return tamanio !=0;
}

int elTamanioDelArchivoEntraEnElOsada(int tamanio){
	printf("BYTES_LIBRES: %i\n",BYTES_LIBRES);
 return tamanio<=BYTES_LIBRES;
}

void modificarEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos){
	printf("modificarEnLaTablaDeArchivos - posDelaTablaDeArchivos: %i \n", posDelaTablaDeArchivos);
	printf("modificarEnLaTablaDeArchivos - file_size: %i \n", file_size);
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].file_size = file_size;
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	//TABLA_DE_ARCHIVOS[posDelaTablaDeArchivos].lastmod = 1;
	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	printf("modificarEnLaTablaDeArchivos - guarda osada 2 fuera\n");

}
int escribirEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos){
	printf("****** escribirEnLaTablaDeArchivos\n");
	int pos=0;
	int encontroLugar = 0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	printf("file_name: %s\n", file_name);
	bool found = false;
    if (posDelaTablaDeArchivos == -999){//SI SE CREA EL ARCHIVO POR PRIMERA VEZ

		for (pos=0; pos <= 2047; pos++){
			//printf("EN EL FOR\n");
			pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
			if (TABLA_DE_ARCHIVOS[pos].state == DELETED && TABLA_DE_ARCHIVOS[pos].state != REGULAR && TABLA_DE_ARCHIVOS[pos].state !=DIRECTORY){
				//printf("EN EL if\n");
				TABLA_DE_ARCHIVOS[pos].state = REGULAR;
				//printf("state\n");

				TABLA_DE_ARCHIVOS[pos].parent_directory = parent_directory;
				//printf("parent_directory: %i\n",parent_directory);

				//printf("fname: %s\n", fname);
				//printf("sizeof(fname): %i\n", strlen(file_name));
				strcpy(TABLA_DE_ARCHIVOS[pos].fname, "\0");
				strcat(TABLA_DE_ARCHIVOS[pos].fname, file_name);

				//printf("fname: %s\n", file_name);
				TABLA_DE_ARCHIVOS[pos].file_size = file_size;
				//printf("file_size: %i\n",file_size);
				TABLA_DE_ARCHIVOS[pos].lastmod = 0;
				//printf("lastmod\n");

				TABLA_DE_ARCHIVOS[pos].first_block= first_block;
				//printf("first_block: %i\n",first_block);

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

	//printf("k: %i\n", k);
	//printf("tablaDeArchivo[k].fname: %s\n", TABLA_DE_ARCHIVOS[k].fname);
	//printf("tablaDeArchivo[k].first_block: %i\n", TABLA_DE_ARCHIVOS[k].first_block);
    pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	printf("guarda osada 2 fuera\n");
	return pos;

}

t_list* obtenerLosIndicesDeLosBloquesDisponiblesYGuardar(int cantidadBloques){
	t_list *listDeBloques = list_create();

	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	printf("HEADER->fs_blocks:  %i\n",HEADER->fs_blocks);
	pthread_mutex_lock(&HEADERmutex);
	uint32_t fs_blocks = HEADER->fs_blocks;
	pthread_mutex_unlock(&HEADERmutex);
	for (i=0; i < fs_blocks; i++){

		pthread_mutex_lock(&BITMAPmutex);
		if(bitarray_test_bit(BITMAP, i) == 0){
			list_add(listDeBloques, i);
			bloquesLibres++;
			//printf("Bloque - %i - LIBRE\n",i);
			bitarray_set_bit(BITMAP, i);
		}
		pthread_mutex_unlock(&BITMAPmutex);

		if (cantidadBloques == bloquesLibres)
			break;

	}

	//printf("DESDE_PARA_BITMAP - %i\n",DESDE_PARA_BITMAP);
	//-printf("TAMANIO_DEL_BITMAP - %i\n",TAMANIO_DEL_BITMAP);
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
	printf("el proximo: %i\n", bloque);
}

void _prepararLaVariableGlobalParaGuadar(char* bloquePos, int bloqueSig) {

	//printf("Bloque Pos: %i\n", atoi(bloquePos));
	//printf("Bloque Sig: %i\n", bloqueSig);
	escribirTablaDeAsignacion(atoi(bloquePos), bloqueSig);


	//free(archivo);

}

//JOEL: NO DEBERIA USARSE MAS
void _guardarEnTablaDeDatos(char* bloquePos, unsigned char* contenido){
	//printf("_guardarEnTablaDeDatos - Bloque Pos: %i\n", atoi(bloquePos));
	//printf("_guardarEnTablaDeDatos - contenido: %s\n",contenido);
	int bloquePosInt = 0;
	bloquePosInt = atoi(bloquePos);
	int tamanioDelBloque = bloquePosInt *64;

	pthread_mutex_lock(&OSADAmutex);
	pthread_mutex_lock(&DATA_BLOCKSmutex);

	memcpy(&OSADA[DATA_BLOCKS+tamanioDelBloque], contenido, OSADA_BLOCK_SIZE );
	printf("_guardarEnTablaDeDatos - bloqueDeDatos: %s\n" ,contenido);

	pthread_mutex_unlock(&DATA_BLOCKSmutex);
	pthread_mutex_unlock(&OSADAmutex);


}



void guardarBloqueDeDatos(t_list* listado, unsigned char *contenido){
	printf("********** guardarBloqueDeDatos\n");

	int cantidadDeBloques = list_size(listado);
	int bloquePos;
	int i,j=0;

	unsigned char *bloqueConDatos;
	bloqueConDatos = malloc(OSADA_BLOCK_SIZE);
	//printf("contenido: %s\n", contenido);
	for(i = 0; i < cantidadDeBloques; i++){
		//bloqueConDatos = string_repeat("\0", OSADA_BLOCK_SIZE);
		char *bloquePosStr;
		memset(bloqueConDatos, 0, OSADA_BLOCK_SIZE);

		bloquePos = list_get(listado, i);
		bloquePosStr = string_itoa(bloquePos);
		int tamanioDelBloque = bloquePos *64;


		memcpy(&OSADA[DATA_BLOCKS + tamanioDelBloque], &contenido[j * OSADA_BLOCK_SIZE ], OSADA_BLOCK_SIZE );
		printf("contenido bloque - %i: %s\n", j, &OSADA[DATA_BLOCKS+tamanioDelBloque]);
		j++;

		printf("GUARDO UN BLOQUE EN EL DICTIONARY\n");
	}

	printf("EMPIEZA A INTERAR PARA GUARDAR \n");
	//free(bloqueConDatos);
}

int calcularCantidadDeBloquesParaGrabar(int tamanio){
	int cantidadDeBloquesParaGrabar = 0;

	if(tamanio > 0 && tamanio < 64){
		return 1;
	}

	if(tamanio > 64){

		cantidadDeBloquesParaGrabar = tamanio / 64;
		int moduloTamanio=0;
		moduloTamanio = tamanio % 64;

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

	printf("cantidadDeElemento: %i\n", cantidadDeElemento);
	for(i=0;i<cantidadDeElemento;i++){
		bloquePosStr = string_repeat("\0", 10);
		bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i+1);
		printf("bloquePos: %i\n", bloquePos);

		if(bloqueSig==0){
			bloqueSig =-1;
		}

		sprintf(bloquePosStr, "%d", bloquePos);

		dictionary_put(dictionary, bloquePosStr, bloqueSig);

		//printf("bloqueSig: %i\n",bloqueSig);

	}
	free(bloquePosStr);
	return dictionary;
}

void modificarAgregandoBloquesEnLaTablaDeAsignacion_archivosGrandes(t_list* listadoLosIndicesDeLosBloquesDisponibles, int ultimoPuntero){
	char *bloquePosStr=malloc(10);
	int cantidadDeElemento = 0;
	int bloquePos;
	int bloqueSig;
	int i;
	t_dictionary *dictionary = dictionary_create();
	cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);

	printf("cantidadDeElemento: %i\n", cantidadDeElemento);
	for(i = 0; i < cantidadDeElemento; i++){
		bloquePosStr = string_repeat("\0", 10);

		if (i==0){
			bloquePos =ultimoPuntero;
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		}else{
			bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i)-1;
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		}
		printf("bloquePos: %i\n", bloquePos);

		if(bloqueSig==0){
			bloqueSig =-1;
			sprintf(bloquePosStr, "%d", bloquePos);
			dictionary_put(dictionary, bloquePosStr, bloqueSig);
			break;
		}

		sprintf(bloquePosStr, "%d", bloquePos);

		dictionary_put(dictionary, bloquePosStr, bloqueSig);

		printf("bloqueSig: %i\n",bloqueSig);

	}
	free(bloquePosStr);

	dictionary_iterator(dictionary, (void*) _prepararLaVariableGlobalParaGuadar);
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
}

void modificarAgregandoBloquesEnLaTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles, t_list* conjuntoDeBloquesDelArchivoViejo){
	char *bloquePosStr=malloc(10);
	int cantidadDeElemento = 0;
	int bloquePos;
	int bloqueSig;
	int i;
	t_dictionary *dictionary = dictionary_create();
	cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);

	printf("cantidadDeElemento: %i\n", cantidadDeElemento);
	for(i = 0; i < cantidadDeElemento; i++){
		bloquePosStr = string_repeat("\0", 10);

		if (i==0){
			bloquePos =list_get(conjuntoDeBloquesDelArchivoViejo,  list_size(conjuntoDeBloquesDelArchivoViejo)-1);
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
		}else{
			bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i+1);
		}
		printf("bloquePos: %i\n", bloquePos);

		if(bloqueSig==0){
			bloqueSig =-1;
			sprintf(bloquePosStr, "%d", bloquePos);
			dictionary_put(dictionary, bloquePosStr, bloqueSig);
			break;
		}

		sprintf(bloquePosStr, "%d", bloquePos);

		dictionary_put(dictionary, bloquePosStr, bloqueSig);

		printf("bloqueSig: %i\n",bloqueSig);

	}
	free(bloquePosStr);

	dictionary_iterator(dictionary, (void*) _prepararLaVariableGlobalParaGuadar);
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
}

void guardarEnLaTablaDeAsignacion(t_list* listadoLosIndicesDeLosBloquesDisponibles){
	dictionary_iterator(armarDicDeTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles), (void*) _prepararLaVariableGlobalParaGuadar);
	pthread_mutex_lock(&ARRAY_TABLA_ASIGNACIONmutex);
	guardarEnOsada(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);
	pthread_mutex_unlock(&ARRAY_TABLA_ASIGNACIONmutex);
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
		printf(
				"*SI ES LA MISMA CANTIDAD DE BLOQUES, ENTONCES SOBREESCRIBO LOS BLOQUES CON EL NUEVO CONTENIDO\n");
		guardarBloqueDeDatos(conjuntoDeBloquesDelArchivo, contenido);
		modificarEnLaTablaDeArchivos(parent_directory, tamanioNuevo, fname,
				list_get(conjuntoDeBloquesDelArchivo, 0),
				posDelaTablaDeArchivos);
	}
}


int agregarMasDatosAlArchivos_archivosGrandes(char *contenido, int tamanioNuevo, char* fname,  uint16_t parent_directory, int ultimoPuntero){
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	printf("*********** agregarMasDatosAlArchivos_archivosGrandes - ultimoPuntero: %i\n", ultimoPuntero);
	osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory);
	osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
	t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
	int cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);
	int posDelaTablaDeArchivos = buscarElArchivoYDevolverPosicion(fname, parent_directory);

	int nuevoSize = elArchivo.file_size + tamanioNuevo;

	//printf("agregarMasDatosAlArchivos_archivosGrandes - SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO\n");
	listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadNuevaDeBloquesParaGrabar);
	modificarAgregandoBloquesEnLaTablaDeAsignacion_archivosGrandes(listadoLosIndicesDeLosBloquesDisponibles, ultimoPuntero);

	printf("*********** agregarMasDatosAlArchivos_archivosGrandes - ultimoPuntero: %i\n", ultimoPuntero);
	guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
	modificarEnLaTablaDeArchivos(parent_directory, nuevoSize, fname, list_get(conjuntoDeBloquesDelArchivo, 0), posDelaTablaDeArchivos);


	return list_get(listadoLosIndicesDeLosBloquesDisponibles, listadoLosIndicesDeLosBloquesDisponibles->elements_count-1);
}

void modificarUnArchivo(char *contenido, int tamanioNuevo, char* fname,  uint16_t parent_directory){
	int cantidadNuevaDeBloquesParaGrabar = 0;
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	int restaEntreNuevoTamanioYViejo = 0;
	int posDelaTablaDeArchivos = 0;

	int i=0;

	if(elTamanioDelArchivoEntraEnElOsada(tamanioNuevo) && noEsVacio(tamanioNuevo)){
		printf("Tamanio: %i\n", tamanioNuevo);
		osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory);
		posDelaTablaDeArchivos = buscarElArchivoYDevolverPosicion(fname, parent_directory);
		printf("El archivo size: %i\n", elArchivo.file_size);

		osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);

		if (posicion == -999){
			crearUnArchivo(contenido, tamanioNuevo, fname, posDelaTablaDeArchivos, parent_directory);
			return;
		}

		t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);

		if (hayNuevosDatosParaAgregar(elArchivo.file_size, tamanioNuevo)){
			printf("************ HAY MAS DATOS PARA AGREGAR\n");
			cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);

			guardarLaMismaCantidadDeBloques(cantidadNuevaDeBloquesParaGrabar,
					parent_directory, tamanioNuevo, posDelaTablaDeArchivos,
					conjuntoDeBloquesDelArchivo, contenido, fname);

			if (conjuntoDeBloquesDelArchivo->elements_count <  cantidadNuevaDeBloquesParaGrabar){
				//*SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO\n
				char *nuevoContenido = string_new();
				printf("*SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO\n");
				listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadNuevaDeBloquesParaGrabar);
				modificarAgregandoBloquesEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles, conjuntoDeBloquesDelArchivo);
				nuevoContenido = string_substring(contenido, elArchivo.file_size, tamanioNuevo - elArchivo.file_size);
				printf("nuevoContenido: %s\n", nuevoContenido);
				guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, nuevoContenido);
				modificarEnLaTablaDeArchivos(parent_directory, tamanioNuevo, fname, list_get(conjuntoDeBloquesDelArchivo, 0), posDelaTablaDeArchivos);
			}
/*
			Viejo: 65 -2bloques
			Nuevo: 129 -3bloques
			Desde: 129-65=64
			*/

		}else{
			printf("************ HAY MENOS DATOS PARA AGREGAR\n");
			cantidadNuevaDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanioNuevo);

			guardarLaMismaCantidadDeBloques(cantidadNuevaDeBloquesParaGrabar,
					parent_directory, tamanioNuevo, posDelaTablaDeArchivos,
					conjuntoDeBloquesDelArchivo, contenido, fname);

			if (conjuntoDeBloquesDelArchivo->elements_count > cantidadNuevaDeBloquesParaGrabar){
				//*SI ES MENOR LA CANTIDAD DE BLOQUES, ENTONCES SACO  LOS VIEJO BLOQUES CON EL NUEVO CONTENIDO\n
				char *nuevoContenido = string_new();
				printf("*SI ES MAYOR LA CANTIDAD DE BLOQUES, ENTONCES CREO  LOS NUEVOS BLOQUES CON EL NUEVO CONTENIDO\n");
				 t_list* sublist = list_take(conjuntoDeBloquesDelArchivo, cantidadNuevaDeBloquesParaGrabar);
				borrarBloquesDelBitmap(conjuntoDeBloquesDelArchivo);
				list_add_in_index(sublist, cantidadNuevaDeBloquesParaGrabar, 0);

				/*
				for(i=0; i <conjuntoDeBloquesDelArchivo->elements_count; i++ ){
					printf("conjuntoDeBloquesDelArchivo - pos: %i\n", list_get(conjuntoDeBloquesDelArchivo, i));
				}

				for(i=0; i < sublist->elements_count; i++ ){
					printf("sublist - pos: %i\n", list_get(sublist, i));
				}


				int posicionDondeTieneQueHaberUnCero = conjuntoDeBloquesDelArchivo->elements_count - cantidadNuevaDeBloquesParaGrabar;
					for(i=0; i < sublist->elements_count; i++ ){
					printf("sublist - pos: %i\n", list_get(sublist, i));
				}
				*/

				guardarEnLaTablaDeAsignacion(sublist);
				//nuevoContenido = string_substring(contenido, elArchivo.file_size, tamanioNuevo);
				//printf("nuevoContenido: %s\n", nuevoContenido);
				//guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, nuevoContenido);
				modificarEnLaTablaDeArchivos(parent_directory, tamanioNuevo, fname, list_get(conjuntoDeBloquesDelArchivo, 0), posDelaTablaDeArchivos);
			}
		}

	}


	printf("************************ FIN MODIFICAR UN ARCHIVO ************************\n");
}

int hacerElTruncate(char *contenido, int tamanio, char* fname, int posDelaTablaDeArchivos, uint16_t parent_directory){
	int cantidadDeBloquesParaGrabar = 0;
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	/*****************************************/
	printf("************************ FUNCION: hacerElTruncate ************************\n");


	osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory);
	posDelaTablaDeArchivos = buscarElArchivoYDevolverPosicion(fname, parent_directory);
	osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
	printf("hacerElTruncate - El archivo size: %i\n", elArchivo.file_size);


	if (posicion != -999 && elArchivo.file_size != 0){
		if (elArchivo.file_size  > tamanio){
			//JOEL: CUANDO HAGO EL TRUNCATE CON MENOR SIZE PUEDO REUTILZIAR LA FUNCION DE modificarUnARchivo para bajar contenido;
			printf("crearUnArchivo - entra para truncate - elArchivo.file_size: %i, tamanio: %i\n", elArchivo.file_size,tamanio);
			modificarUnArchivo(contenido, tamanio,fname, parent_directory);
			return 1;
		}
			else
		{
			printf("crearUnArchivo -ES UN ARHIVO CON MUCHOS BLOQUES- elArchivo.file_size: %i, tamanio: %i\n", elArchivo.file_size,tamanio);
			t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
			return agregarMasDatosAlArchivos_archivosGrandes(contenido, tamanio, fname,  parent_directory, list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1));

		}
	}



	/********************************************************/

	if(elTamanioDelArchivoEntraEnElOsada(tamanio) && noEsVacio(tamanio)){
		printf("crearUnArchivo - tamanio: %i\n", tamanio);

		cantidadDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanio);
		printf("crearUnArchivo - cantidadDeBloquesParaGrabar: %i\n", cantidadDeBloquesParaGrabar);


		listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadDeBloquesParaGrabar);

		guardarEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles);
		guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
		escribirEnLaTablaDeArchivos(parent_directory, tamanio, fname, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0), posDelaTablaDeArchivos);

	}
	return list_get(listadoLosIndicesDeLosBloquesDisponibles, listadoLosIndicesDeLosBloquesDisponibles->elements_count-1);
	printf("************************ FIN CREAR UN ARCHIVO ************************\n");
}

int crearUnArchivo(unsigned char *contenido, int tamanio, char* fname, int posDelaTablaDeArchivos, uint16_t parent_directory){
	int cantidadDeBloquesParaGrabar = 0;
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	/*****************************************/
	osada_file elArchivo = buscarElArchivoYDevolverOsadaFile(fname, parent_directory);
	posDelaTablaDeArchivos = buscarElArchivoYDevolverPosicion(fname, parent_directory);
	osada_block_pointer posicion = devolverOsadaBlockPointer(fname, parent_directory);
	printf("crearUnArchivo - El archivo size: %i\n", elArchivo.file_size);


	if (posicion != -999 && elArchivo.file_size != 0){
			printf("crearUnArchivo -ES UN ARHIVO CON MUCHOS BLOQUES- elArchivo.file_size: %i, tamanio: %i\n", elArchivo.file_size,tamanio);
			t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
			return agregarMasDatosAlArchivos_archivosGrandes(contenido, tamanio, fname,  parent_directory, list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1));

	}



	/********************************************************/

	if(elTamanioDelArchivoEntraEnElOsada(tamanio) && noEsVacio(tamanio)){
		printf("crearUnArchivo - tamanio: %i\n", tamanio);

		cantidadDeBloquesParaGrabar = calcularCantidadDeBloquesParaGrabar(tamanio);
		printf("crearUnArchivo - cantidadDeBloquesParaGrabar: %i\n", cantidadDeBloquesParaGrabar);


		listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponiblesYGuardar (cantidadDeBloquesParaGrabar);

		guardarEnLaTablaDeAsignacion(listadoLosIndicesDeLosBloquesDisponibles);
		guardarBloqueDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
		escribirEnLaTablaDeArchivos(parent_directory, tamanio, fname, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0), posDelaTablaDeArchivos);

	}
	return list_get(listadoLosIndicesDeLosBloquesDisponibles, listadoLosIndicesDeLosBloquesDisponibles->elements_count-1);
	printf("************************ FIN CREAR UN ARCHIVO ************************\n");
}
/************************FIN ARCHIVO************************************************/

/**************************INICIO DIRECTORIOS**************************************/
/****************LISTAR TODO *************************************************/
void mostrarLosDirectorios(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DIRECTORY){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}

void mostrarLosRegulares(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == REGULAR){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}

void mostrarLosBorrados(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state == DELETED){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
	}
}

void mostrarOtrosEstados(osada_file tablaDeArchivo, int pos){
	if (tablaDeArchivo.state != DELETED && tablaDeArchivo.state != REGULAR && tablaDeArchivo.state !=DIRECTORY){
		printf("Empieza: %i****************\n",pos);
		printf("state_%i: %c\n",pos, tablaDeArchivo.state);
		printf("parent_directory_%i: %i\n",pos, tablaDeArchivo.parent_directory);
		printf("fname_%i: %s\n",pos, &tablaDeArchivo.fname);
		printf("file_size_%i: %i\n",pos, tablaDeArchivo.file_size);
		printf("lastmod_%i: %i\n",pos, tablaDeArchivo.lastmod);
		printf("first_block_%i: %i\n",pos, tablaDeArchivo.first_block);
		printf("Termina: %i****************\n",pos);
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
	printf("Diccionario - Carpeta en el root: %s\n", key);
	printf("Un elemento de la lista list, que seria el primer hijo del root: %s\n", archivo->fname);
	printf("archivo->parent_directory: %i\n", archivo->parent_directory);

	//free(archivo);

}

void reconocerDirectorio(osada_file *archivo, int pos, t_dictionary *dictionary){

	t_list *list = list_create();

	if (archivo->state == DIRECTORY  && archivo->parent_directory == 65535){
		char str[10];
		sprintf(str, "%d", pos);
		list_add(list, archivo);

		//printf("pos: %s", str);

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
		//printf("error!");
		char str[10];
		sprintf(str, "%d", archivo->parent_directory);
		//list_add(list, archivo);

		//printf("pos: %i\n",  archivo->parent_directory);

		//dictionary_put(dictionary, (char *)archivo->fname , list);
		t_list *list = dictionary_get(dictionaryDirRoot, str);
		//printf("list: %i\n",  list);

		if (list != 0){
			list_add(list, archivo);
			dictionary_put(dictionaryDirRoot, str , list);
		}else{

			printf("Directorios sin padres: %i\n",  archivo->parent_directory);
			printf("Sin Nombre: %s\n", archivo->fname);
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
		printf("EMPIEZA reconocerArchivosParaArbol %i: ****************\n", pos);
		printf("state_: %c\n", archivo->state);
		printf("parent_directory_: %i\n", archivo->parent_directory);
		printf("fname_: %s\n", &archivo->fname);
		printf("file_size_: %i\n", archivo->file_size);
		printf("lastmod_: %i\n", archivo->lastmod);
		printf("first_block_: %i\n", archivo->first_block);
		printf("Termina reconocerArchivosParaArbol %i ****************\n", pos);
		list_add(lista, archivo);
	}

}

void reconocerDirectorioPadre(osada_file *archivo, int pos, int padre){
	if (archivo[padre].state == DIRECTORY  ){
		printf("EMPIEZA reconocerDirectorioPadre %i: ****************\n", pos);
		printf("state_: %c\n", archivo->state);
		printf("parent_directory_: %i\n", archivo->parent_directory);
		printf("fname_: %s\n", &archivo->fname);
		printf("file_size_: %i\n", archivo->file_size);
		printf("lastmod_: %i\n", archivo->lastmod);
		printf("first_block_: %i\n", archivo->first_block);
		printf("Termina reconocerDirectorioPadre %i: ****************\n", pos);
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
int crearUnDirectorio(char *fname, int parent_directory){
	int k=0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	printf("**********************************crearUnDirectorio - file: %s\n", fname);
	printf("**********************************crearUnDirectorio - file_name: %s\n", file_name);


	int bloque = obtener_bloque_padre(fname);
	printf("**********************************crearUnDirectorio - bloque: %i\n", bloque);

	bool found = false;
	for (k=0; k <= 2047; k++){
		//printf("EN EL FOR\n");
		pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
		if (TABLA_DE_ARCHIVOS[k].state == DELETED && TABLA_DE_ARCHIVOS[k].state != REGULAR && TABLA_DE_ARCHIVOS[k].state != DIRECTORY){
			TABLA_DE_ARCHIVOS[k].state = DIRECTORY;

			TABLA_DE_ARCHIVOS[k].parent_directory = bloque;
			printf("parent_directory: %i\n",bloque);

			//printf("fname: %s\n", fname);
			strcpy(TABLA_DE_ARCHIVOS[k].fname, "\0");
			strcat(TABLA_DE_ARCHIVOS[k].fname, file_name);

			TABLA_DE_ARCHIVOS[k].file_size = 0;
			TABLA_DE_ARCHIVOS[k].lastmod = 0;
			printf("lastmod\n");

			TABLA_DE_ARCHIVOS[k].first_block= 0;
			printf("first_block: %i\n",0);

			found = true;

		}
		pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
		if (found){
			break;
		}
	}//for (k=0; k <= 2047; k++)
	//printf("afuera del if\n");



	printf("k: %i\n", k);
	printf("tablaDeArchivo[k].fname: %s\n", TABLA_DE_ARCHIVOS[k].fname);


	pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
	guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
	pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
	printf("guarda osada 2 fuera\n");
	return k;
}

int borrarUnDirectorio(char *fname, int parent_directory){
	int pos=0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	printf("borrarUnDirectorio - file_name: %s\n", file_name);
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
			printf("EN EL if\n");
			TABLA_DE_ARCHIVOS[pos].state =  DELETED;
			printf("state\n");
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
	printf("guarda osada 2 fuera\n");
	return pos;
}
/*******************************************FIN DIRECTORIO*************************/
