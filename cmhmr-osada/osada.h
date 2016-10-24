#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
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

#endif __OSADA_H__

unsigned char *inicializarOSADA(int archivoID);
osada_header *obtenerHeader();
t_bitarray *obtenerBitmap();
osada_file *obtenerTablaDeArchivos();
int *obtenerTablaDeAsignacion();
char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile);

int setearTamanioDelArchivo(int archivoID);
int obtenerIDDelArchivo(char *ruta);
void setearConstantesDePosicionDeOsada();

void guardarEnOsada(unsigned char *osada, int desde, void *elemento, int tamaniaDelElemento);
void guardarEnOsada2(int desde, void *elemento, int tamaniaDelElemento);
osada_block_pointer buscarArchivo(char *nombre);
t_list* crearArbolAPartirDelPadre(int padre);

static int TAMANIO_QUE_OCUPA_EL_HEADER= 0;
static int TAMANIO_DEL_BITMAP= 0;
static int TAMANIO_TABLA_DE_ARCHIVOS= 0;
static int TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION= 0;
static int TAMANIO_QUE_OCUPA_LA_TABLA_DE_ASIGNACION_EN_BLOQUES = 0;
static int TAMANIO_QUE_OCUPA_EL_BLOQUE_DE_DATOS = 0;
static int TAMANIO_DEL_ARCHIVO_OSADA_EN_BYTES = 0;

static int DESDE_PARA_BLOQUE_DE_DATOS= 0;
static int DESDE_PARA_BITMAP= 0;//LO QUE OCUPA EL HEADER
static int DESDE_PARA_TABLA_DE_ARCHIVOS = 0;
static int DESDE_PARA_TABLA_ASIGNACION = 0;
static int BYTES_LIBRES = 0;
static int BYTES_OCUPADOS = 0;

static unsigned char *OSADA;
static osada_header *HEADER;
static t_bitarray *BITMAP;
static int DATA_BLOCKS;
static int *ARRAY_TABLA_ASIGNACION;
static osada_file *TABLA_DE_ARCHIVOS;
