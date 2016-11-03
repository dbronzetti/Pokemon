/*
 * osada.c
 *
 *  Created on: 9/9/2016
 *      Author: utnso
 */

#include "osada.h"
#include <errno.h>
#include <commons/string.h>


void guardarEnOsada2(int desde, void *elemento, int tamaniaDelElemento){
	printf("iniciio guardarEnOsada2\n");
	memcpy(&OSADA[desde], elemento, tamaniaDelElemento);
	printf("fin guardarEnOsada2\n");
}

void guardarEnOsada(unsigned char *osada, int desde, void *elemento, int tamaniaDelElemento){
	memcpy(&osada[desde], elemento, tamaniaDelElemento );
	int status = munmap(osada, tamaniaDelElemento);

	if (status == -1)
		printf("Estado del munmap: %i\n", status);
}




char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile){
	//unsigned char *bloqueDeDatos = malloc(sizeof(char) * osadaHeaderFile->data_blocks);OLD
	unsigned char *bloqueDeDatos = malloc(sizeof(char) * OSADA_BLOCK_SIZE * osadaHeaderFile->data_blocks);

	memcpy(bloqueDeDatos, &osada[DESDE_PARA_BLOQUE_DE_DATOS], TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS );
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

	memcpy(arrayTabla, &OSADA[DESDE_PARA_TABLA_ASIGNACION], TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION );

	//mostrarTodosLosAsignados(arrayTabla, numeroBloques);
	ARRAY_TABLA_ASIGNACION = arrayTabla;
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
	memcpy(tablaDeArchivo, &OSADA[DESDE_PARA_TABLA_DE_ARCHIVOS ], TAMANIO_TABLA_DE_ARCHIVOS);

	//mostrarTodaLaTablaDeArchivos(tablaDeArchivo);
	TABLA_DE_ARCHIVOS = tablaDeArchivo;
	return tablaDeArchivo;
}

void contarBloques(){
	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	for (i=0; i < HEADER->fs_blocks; i++){//para 150k

		if(bitarray_test_bit(BITMAP, i) == 0){
			bloquesLibres++;
			//printf("Bloque - %i - LIBRE\n",i);
		}

		if(bitarray_test_bit(BITMAP, i) == 1){
			bloquesOcupados++;
			//printf("Bloque - %i - OCUPADO\n",i);
		}

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
	memcpy(unBitMapSinFormato, &OSADA[DESDE_PARA_BITMAP], TAMANIO_DEL_BITMAP );
	bitMap = bitarray_create(unBitMapSinFormato, TAMANIO_DEL_BITMAP );
	BITMAP = bitMap;
	contarBloques();

	return bitMap;

}



void mostrarHeader(osada_header *osadaHeaderFile){
	printf("magic_number 2: %s\n",  osadaHeaderFile->magic_number);
	printf("version: %i\n", osadaHeaderFile->version);
	printf("fs_blocks: %i\n", osadaHeaderFile->fs_blocks);
	printf("bitmap_blocks: %i\n", osadaHeaderFile->bitmap_blocks);
	printf("allocations_table_offset: %i\n", osadaHeaderFile->allocations_table_offset);
	printf("data_blocks: %i\n", osadaHeaderFile->data_blocks);
	printf("padding: %s\n",   osadaHeaderFile->padding);
}

osada_header *obtenerHeader(){
	osada_header *osadaHeaderFile = malloc(sizeof(osada_header));
	memcpy(osadaHeaderFile, OSADA, OSADA_BLOCK_SIZE);


	mostrarHeader(osadaHeaderFile);
	HEADER = osadaHeaderFile;
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
	TAMANIO_DEL_BITMAP = HEADER->bitmap_blocks * OSADA_BLOCK_SIZE;
	TAMANIO_TABLA_DE_ARCHIVOS =  2048 * sizeof(osada_file);
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION = (HEADER->fs_blocks - 1 - HEADER->bitmap_blocks - 1024) * 4;
	TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES = (HEADER->fs_blocks - 1 - HEADER->bitmap_blocks - 1024) * 4 / OSADA_BLOCK_SIZE;
	TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS = OSADA_BLOCK_SIZE* HEADER->data_blocks;
	DATA_BLOCKS= (HEADER->fs_blocks - HEADER->data_blocks)*64;
	//dataBlocks=  osadaHeaderFile->allocations_table_offset + tamanioQueOcupaLaTablaDeAsignacionEnBloques;

	printf("HEADER->fs_blocks - HEADER->data_blocks: %i\n",HEADER->fs_blocks - HEADER->data_blocks);
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

	/************************************************************/
	printf("Que paso?: %s\n", strerror(errno));
	printf("archivoID: %i\n", archivoID);
	printf("tamanio: %i\n", TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES);
	/************************************************************/


	osada = mmap(0, TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES, PROT_READ|PROT_WRITE,MAP_SHARED, archivoID, 0);
	int statusCerrar = close(archivoID);
	OSADA = osada;
	return osada;

}
/********************************************ARCHIVOS**************************************/

void _iterarParaVerContenido(int bloque){

	char *bloqueDeDatos = malloc(OSADA_BLOCK_SIZE + 1);
	int bloque2 = bloque *64;
	int i;
	//tamanioQueOcupaElBloqueDeDatos ir de atras con los bloques.
	//printf("%i\n", dataBlocks);
	memcpy(bloqueDeDatos, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );

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
	while ((elProximo = ARRAY_TABLA_ASIGNACION[numeroBloques]) != -1){
		list_add(proximo, elProximo);
		numeroBloques = elProximo;

	}

	list_iterate(proximo, (void*) _iterarBloques);

	return proximo;
}

osada_block_pointer devolverBloque(osada_file tablaDeArchivo, uint16_t parent_directory, char *nombre){
	char *nac;
	char *n;
	nac = string_duplicate(&tablaDeArchivo.fname);
	n = string_duplicate(nombre);
	string_trim(&nac);
	string_trim(&n);

	if (tablaDeArchivo.parent_directory == parent_directory && tablaDeArchivo.state == REGULAR && strcmp(nac, n) == 0){
		free(nac);
		free(n);
		printf("state_: %c\n", tablaDeArchivo.state);
		printf("parent_directory_: %i\n", tablaDeArchivo.parent_directory);
		printf("fname_: %s\n",&tablaDeArchivo.fname);
		printf("file_size_: %i\n",tablaDeArchivo.file_size);
		printf("lastmod_: %i\n", tablaDeArchivo.lastmod);
		printf("first_block_: %i\n",tablaDeArchivo.first_block);

		return tablaDeArchivo.first_block;
	}

	free(nac);
	free(n);
	return -666;
}



osada_block_pointer buscarArchivo(char *nombre, uint16_t parent_directory){
	printf("******************************** ENTRO EN EL buscarArchivo ******************************** \n");
	int pos=0;
	osada_block_pointer posicion = 0;
	char *file_name = strrchr (nombre, '/') + 1;
	printf("file_name: %s\n", file_name);

	for (pos=0; pos <= 2047; pos++){

		if ((posicion = devolverBloque(TABLA_DE_ARCHIVOS[pos],parent_directory,  file_name)) != -666){
			printf("encontro>! , %i\n", pos);
			return posicion;
		};
	}

	printf("******************************* NO LO ENCONTRO! ******************************* \n");
	return posicion;
}

void borrarBloqueDelBitmap(int bloque){

	if(bitarray_test_bit(BITMAP, bloque) == 1){
		bitarray_clean_bit(BITMAP, bloque);
	}



}

void borrarBloquesDelBitmap(t_list *listado){
	list_iterate(listado, (void*)borrarBloqueDelBitmap);
	guardarEnOsada2(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);

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
		nac = string_duplicate(&TABLA_DE_ARCHIVOS[pos].fname);
		n = string_duplicate(file_name);
		string_trim(&nac);
		string_trim(&n);
		printf("nac: %s\n", &TABLA_DE_ARCHIVOS[pos].fname);

		if (TABLA_DE_ARCHIVOS[pos].parent_directory == parent_directory  && strcmp(nac, n) == 0){
			printf("******************************* LO ENCONTRO! ******************************* \n");
			TABLA_DE_ARCHIVOS[pos].state = DELETED;
			posicion = pos;

		}
	}
	guardarEnOsada2(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);

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

   while (vector_path[i] != NULL)
   {
	   for (j=0;j<2047;j++)
	   {
		   if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir) && (TABLA_DE_ARCHIVOS[j].state != DELETED))
		   {
			   parent_dir = j;
			   pos_archivo = j;
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
			for (j = 0; j < 2047; j++)
			{
				if ((strcmp(TABLA_DE_ARCHIVOS[j].fname, vector_path[i]) == 0) && (TABLA_DE_ARCHIVOS[j].parent_directory == parent_dir))
				{
					if ((i == 0) && (parent_dir == 65535))
					{
						parent_dir = j;
					}
					if ((i > 0) && (parent_dir != 0))
					{
						parent_dir = j;
					}
				}
			}
			i++;
		}
	}

	return parent_dir;
}


int noEsVacio(int tamanio){
	return tamanio !=0;
}

int elTamanioDelArchivoEntraEnElOsada(int tamanio){
	printf("BYTES_LIBRES: %i\n",BYTES_LIBRES);
 return tamanio<=BYTES_LIBRES;
}

int escribirEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos){
	osada_file *tablaDeArchivo = obtenerTablaDeArchivos();
	int k=0;
	//TODO: HACERLO RECURSIVO LA LINEA DE ABAJO
	char *file_name = strrchr (fname, '/') + 1;
	printf("file_name: %s\n", file_name);

    if (posDelaTablaDeArchivos == -999){//SI SE CREA EL ARCHIVO POR PRIMERA VEZ
		for (k=0; k <= 2047; k++){
			//printf("EN EL FOR\n");
			if (tablaDeArchivo[k].state == DELETED){
				printf("EN EL if\n");
					tablaDeArchivo[k].state = REGULAR;
					printf("state\n");

					tablaDeArchivo[k].parent_directory = parent_directory;
					printf("parent_directory: %i\n",parent_directory);

					//printf("fname: %s\n", fname);
					printf("sizeof(fname): %i\n", strlen(file_name));
					strcpy(tablaDeArchivo[k].fname, "\0");
					strcat(tablaDeArchivo[k].fname, file_name);

					printf("fname: %s\n", file_name);
					tablaDeArchivo[k].file_size = file_size;
					printf("file_size: %i\n",file_size);
					tablaDeArchivo[k].lastmod = 0;
					printf("lastmod\n");

					tablaDeArchivo[k].first_block= first_block;
					printf("first_block: %i\n",first_block);

					break;

			}
		}//for (k=0; k <= 2047; k++)
			//printf("afuera del if\n");
	}
    else
	{
		tablaDeArchivo[posDelaTablaDeArchivos].file_size = file_size;
		tablaDeArchivo[posDelaTablaDeArchivos].first_block= first_block;
		k=posDelaTablaDeArchivos;
	}



	printf("k: %i\n", k);
	printf("tablaDeArchivo[k].fname: %s\n", tablaDeArchivo[k].fname);
	printf("tablaDeArchivo[k].first_block: %i\n", tablaDeArchivo[k].first_block);

	guardarEnOsada2(DESDE_PARA_TABLA_DE_ARCHIVOS, tablaDeArchivo, TAMANIO_TABLA_DE_ARCHIVOS);
	printf("guarda osada 2 fuera\n");
	return k;

}

t_list* obtenerLosIndicesDeLosBloquesDisponibles(int cantidadBloques){
	t_list *listDeBloques = list_create();

	int bloquesOcupados  = 0;
	int bloquesLibres = 0;
	int i = 0;

	for (i=0; i < HEADER->fs_blocks; i++){

		if(bitarray_test_bit(BITMAP, i) == 0){
			list_add(listDeBloques, i);
			bloquesLibres++;
			//printf("Bloque - %i - LIBRE\n",i);
			bitarray_set_bit(BITMAP, i);
		}

		if (cantidadBloques == bloquesLibres)
			break;

	}

	//printf("DESDE_PARA_BITMAP - %i\n",DESDE_PARA_BITMAP);
	//-printf("TAMANIO_DEL_BITMAP - %i\n",TAMANIO_DEL_BITMAP);
	guardarEnOsada2(DESDE_PARA_BITMAP, BITMAP->bitarray, TAMANIO_DEL_BITMAP);

	return listDeBloques;
}

void escribirElBitmap(int bloques){

}

void escribirTablaDeAsignacion(int pos, int bloqueSiguiente){
	ARRAY_TABLA_ASIGNACION[pos] = bloqueSiguiente;
}

void escribirBloquesDeDatos(){

}

void _interarBloquesQueSeranAsignados(int bloque,int hola){
	printf("el proximo: %i\n", bloque);
}

void _recorrerComoSeriaElArray(char* bloquePos, int bloqueSig) {

	printf("Bloque Pos: %i\n", atoi(bloquePos));
	printf("Bloque Sig: %i\n", bloqueSig);
	escribirTablaDeAsignacion(atoi(bloquePos), bloqueSig);


	//free(archivo);

}

void _guardarEnTablaDeDatos(char* bloquePos, char* contenido){
	printf("_guardarEnTablaDeDatos - Bloque Pos: %i\n", atoi(bloquePos));
	//printf("_guardarEnTablaDeDatos - contenido: %s\n",contenido);
	int bloquePosInt = 0;
	bloquePosInt = atoi(bloquePos);

	char *bloqueDeDatos = malloc(strlen(contenido));

	int bloque2 = bloquePosInt *64;
	strcpy(bloqueDeDatos, contenido);
	printf("_guardarEnTablaDeDatos - bloqueDeDatos: %s\n",bloqueDeDatos);
	memcpy(&OSADA[DATA_BLOCKS+bloque2], bloqueDeDatos, OSADA_BLOCK_SIZE );


}



void prepararBloquesDeDatos(t_list* listado, char *contenido){
	int cantidadDeBloques = list_size(listado);
	int bloquePos;
	int i,j=0;
	t_dictionary *dicBloqueDeDatos = dictionary_create();


	for(i = 0; i < cantidadDeBloques; i++){
		char *bloqueConDatos;
		char *bloquePosStr;

		bloquePos = list_get(listado, i);
		bloquePosStr = string_itoa(bloquePos);

		bloqueConDatos = string_itoa(i);

		//TODO: PREGUNTAR A DAMIAN ESTA FORMA DE CODEAR PARA DIVIDIR CADA BLOQUE DE DATOS
		if(i == (cantidadDeBloques - 1)){
			bloqueConDatos = malloc(strlen(contenido));
			printf("contenido 2 - %i: %c\n",j, contenido[j * OSADA_BLOCK_SIZE ]);
			memcpy(bloqueConDatos, &contenido[j * OSADA_BLOCK_SIZE ], strlen(contenido) );
			bloqueConDatos[strlen(contenido) +1 ]= '\0';
			printf("bloqueConDatos: %s\n", bloqueConDatos);
			j++;
		}else{
			bloqueConDatos = malloc(64);
			printf("contenido 3 - %i: %c\n", j, contenido[j * OSADA_BLOCK_SIZE ]);
			memcpy(bloqueConDatos, &contenido[j * OSADA_BLOCK_SIZE ], OSADA_BLOCK_SIZE);
			bloqueConDatos[65]= '\0';
			printf("bloqueConDatos: %s\n", bloqueConDatos);
			j++;
		}

		dictionary_put(dicBloqueDeDatos, bloquePosStr, bloqueConDatos);
		printf("ENTRO EN EL DICTIONARY\n");
	}
	printf("QJUIEE INTERAR \n");
	dictionary_iterator(dicBloqueDeDatos, (void*) _guardarEnTablaDeDatos);
}



void crearUnArchivo(char *contenido, int tamanio, char* fname, int posDelaTablaDeArchivos, uint16_t parent_directory){
	int cantidadDeBloquesParaGrabar = 0;
	t_list* listadoLosIndicesDeLosBloquesDisponibles;
	int i=0;
	int cantidadDeElemento = 0;

	t_dictionary *comoSeriaElArray = dictionary_create();
	int bloquePos;
	int bloqueSig;


	if(elTamanioDelArchivoEntraEnElOsada(tamanio) && noEsVacio(tamanio)){

		printf("tamanio: %i\n", tamanio);

		if(tamanio > 0 && tamanio < 64){
			cantidadDeBloquesParaGrabar = 1;
		}


		if(tamanio > 64){

			cantidadDeBloquesParaGrabar = tamanio / 64;
			int moduloTamanio=0;
			moduloTamanio = tamanio % 64;

			if (moduloTamanio>0){
				cantidadDeBloquesParaGrabar = cantidadDeBloquesParaGrabar + 1;
			}

		}

		printf("cantidadDeBloquesParaGrabar: %i\n", cantidadDeBloquesParaGrabar);

		//cantidadDeBloquesParaGrabar = tamanio /64;
		listadoLosIndicesDeLosBloquesDisponibles = obtenerLosIndicesDeLosBloquesDisponibles(cantidadDeBloquesParaGrabar);
		cantidadDeElemento = list_size(listadoLosIndicesDeLosBloquesDisponibles);
		printf("cantidadDeElemento: %i\n", cantidadDeElemento);

		for(i=0;i<cantidadDeElemento;i++){
			bloquePos = list_get(listadoLosIndicesDeLosBloquesDisponibles, i);
			bloqueSig = list_get(listadoLosIndicesDeLosBloquesDisponibles, i+1);
			printf("bloquePos: %i\n", bloquePos);

			if(bloqueSig==0){
				bloqueSig =-1;
			}

			char bloquePosStr[10];
			sprintf(bloquePosStr, "%d", bloquePos);

			dictionary_put(comoSeriaElArray, bloquePosStr, bloqueSig);

			//printf("bloqueSig: %i\n",bloqueSig);

		}

		//QUE HACER CUANDO LOS BLOQUES NO SON ENTEROS.
		//list_iterate(listado, (int)_interarBloquesQueSeranAsignados);
		dictionary_iterator(comoSeriaElArray, (void*) _recorrerComoSeriaElArray);
		guardarEnOsada2(DESDE_PARA_TABLA_ASIGNACION, ARRAY_TABLA_ASIGNACION, TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION);

		prepararBloquesDeDatos(listadoLosIndicesDeLosBloquesDisponibles, contenido);
		escribirEnLaTablaDeArchivos(parent_directory, tamanio, fname, list_get(listadoLosIndicesDeLosBloquesDisponibles, 0), posDelaTablaDeArchivos);
	}


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
		mostrarLosRegulares(TABLA_DE_ARCHIVOS[pos], pos);
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



	if (archivo->parent_directory == padre){
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
		reconocerArchivosParaArbol(&TABLA_DE_ARCHIVOS[pos], pos, padre, lista);
	}

	return lista;

}

void encontrarArbolPadre(int padre){
	int pos=0;

	//for (pos=0; pos <= 2047; pos++){
		reconocerDirectorioPadre(&TABLA_DE_ARCHIVOS[pos], pos, padre);
	//}

}

/*****************************************************/
void crearUnDirectorio(){

}
/*******************************************FIN DIRECTORIO*************************/
