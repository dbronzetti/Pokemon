#include <stdio.h>
typedef uint32_t osada_block_pointer;
#define OSADA_FILENAME_LENGTH 17

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




void serializeOsadaFileVector(osada_file *value, char *buffer){
	int offset = 0;
	int count;
	char *bufferAux;

	//0)Conformo un bufferAux para tener el contenido de todos los archivso a serializar
	while(&value[count]){
			memcpy(bufferAux, &value[count]->file_size, sizeof(&value[count]->file_size));
			memcpy(bufferAux, &value[count]->first_block, sizeof(&value[count]->first_block));
			memcpy(bufferAux, &value[count]->fname, sizeof(&value[count]->fname));
			memcpy(bufferAux, &value[count]->lastmod, sizeof(&value[count]->lastmod));
			memcpy(bufferAux, &value[count]->parent_directory, sizeof(&value[count]->parent_directory));
			memcpy(bufferAux, &value[count]->state, sizeof(&value[count]->state));
			count++;
	}

	/** Seteo mi estructura de respuesta
	 * typedef struct{
			int cantidadLargo; <--- Aca va el count total.
			char* contenido;   <--- Aca meto el bufferAux
		} t_MessagePokeServer_Client;
	 */


}

