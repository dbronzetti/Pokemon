#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>


#ifndef __OSADA_H__
#define __OSADA_H__

#define OSADA_BLOCK_SIZE 64
#define OSADA_FILENAME_LENGTH 17

typedef unsigned char osada_block[OSADA_BLOCK_SIZE];
typedef uint32_t osada_block_pointer;

// set __attribute__((packed)) for this whole section
// See http://stackoverflow.com/a/11772340/641451
#pragma pack(push, 1)

typedef struct {
	unsigned char magic_number[7]; // OSADAFS
	uint8_t version;
	uint32_t fs_blocks; // total amount of blocks
	uint32_t bitmap_blocks; // bitmap size in blocks
	uint32_t allocations_table_offset; // allocations table's first block number
	uint32_t data_blocks; // amount of data blocks
	unsigned char padding[40]; // useless bytes just to complete the block size
} osada_header;

_Static_assert( sizeof(osada_header) == sizeof(osada_block), "osada_header size does not match osada_block size");

typedef enum __attribute__((packed)) {
    DELETED = '\0',
    REGULAR = '\1',
    DIRECTORY = '\2',
} osada_file_state;

_Static_assert( sizeof(osada_file_state) == 1, "osada_file_state is not a char type");

typedef struct {
	osada_file_state state;
	unsigned char fname[OSADA_FILENAME_LENGTH];
	uint16_t parent_directory;
	uint32_t file_size;
	uint32_t lastmod;
	osada_block_pointer first_block;
} osada_file;

_Static_assert( sizeof(osada_file) == (sizeof(osada_block) / 2.0), "osada_file size does not half osada_block size");

#pragma pack(pop)

void initMutexOsada();
void destroyMutexOsada();
unsigned char *inicializarOSADA(int archivoID);
osada_header *obtenerHeader();
t_bitarray *obtenerBitmap();
osada_file *obtenerTablaDeArchivos();
int *obtenerTablaDeAsignacion();
char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile);

int setearTamanioDelArchivo(int archivoID);
int obtenerIDDelArchivo(char *ruta);
void setearConstantesDePosicionDeOsada();

void guardarEnOsada(int desde, void *elemento, int tamaniaDelElemento);
char**  armar_vector_path(const char* text);
int escribirUnArchivo(unsigned char *contenido, int tamanio, char* fname, int offset);
int obtener_bloque_archivo(const char* path);
t_list* crearArbolAPartirDelPadre(int padre);
t_list *obtenerElListadoDeBloquesCorrespondientesAlArchivo(int bloqueActual);
int escribirEnLaTablaDeArchivos(int parent_directory, int file_size, char* fname, int first_block, int posDelaTablaDeArchivos);
osada_block_pointer comprobarElNombreDelArchivo(osada_file tablaDeArchivo, uint16_t parent_directory, char *nombre);
osada_block_pointer devolverOsadaBlockPointer(char *nombre, uint16_t parent_directory);
int inicializarNuevoArchivo( char* path);
void sobreescribirNombre(char *nombre, char *nuevoNombre);
int crearUnDirectorio(char *fname);
int borrarUnDirectorio(char *fname);
int hacerElTruncate(int offset, char* path,int* pos_archivo);

static int TAMANIO_QUE_OCUPA_EL_HEADER;
static int TAMANIO_DEL_BITMAP;
static int TAMANIO_TABLA_DE_ARCHIVOS;
static int TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION;
static int TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES;
static int TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS;
static int TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES;

static int DESDE_PARA_BLOQUE_DE_DATOS;
static int DESDE_PARA_BITMAP;//LO QUE OCUPA EL HEADER
static int DESDE_PARA_TABLA_DE_ARCHIVOS;
static int DESDE_PARA_TABLA_ASIGNACION;
static int BYTES_LIBRES;
static int BYTES_OCUPADOS;

static unsigned char *OSADA;
static osada_header *HEADER;
static t_bitarray *BITMAP;
static int DATA_BLOCKS;
static int *ARRAY_TABLA_ASIGNACION;
static osada_file *TABLA_DE_ARCHIVOS;

t_log* logPokeDexServer;
pthread_mutex_t OSADAmutex;
pthread_mutex_t HEADERmutex;
pthread_mutex_t BITMAPmutex;
pthread_mutex_t DATA_BLOCKSmutex;
pthread_mutex_t ARRAY_TABLA_ASIGNACIONmutex;
pthread_mutex_t TABLA_DE_ARCHIVOSmutex;

#endif __OSADA_H__






