#include <stdint.h>
#include <stdio.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

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
osada_header *obtenerHeader(unsigned char *osada);
t_bitarray *obtenerBitmap(unsigned char *osada, osada_header *osadaHeaderFile);
osada_file *obtenerTablaDeArchivos(unsigned char *osada, osada_header *osadaHeaderFile);
int *obtenerTablaDeAsignacion(unsigned char *osada, osada_header *osadaHeaderFile);
char *obtenerBloqueDeDatos(unsigned char *osada, osada_header *osadaHeaderFile);

int setearTamanioDelArchivo(int archivoID);
int obtenerIDDelArchivo(char *ruta);

void guardarEnOsada(unsigned char *osada, int desde, void *elemento, int tamaniaDelElemento);

static int tamanioQueOcupaElHeader= 0;
static int tamanioDelBitMap= 0;
static int tamanioTablaDeArchivos= 0;
static int tamanioQueOcupaLaTablaDeAsignacion= 0;
static int tamanioQueOcupaLaTablaDeAsignacionEnBloques = 0;
static int tamanioQueOcupaElBloqueDeDatos = 0;
static int tamanioDelArchivoOSADAEnBytes = 0;

static int desdeParaBloqueDeDatos= 0;
static int desdeParaBitmap= 0;//LO QUE OCUPA EL HEADER
static int desdeParaTablaDeArchivos= 0;
static int desdeParaTablaAsigancion= 0;