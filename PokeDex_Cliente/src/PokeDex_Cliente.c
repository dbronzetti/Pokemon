/*
 ============================================================================
 Name        : PokeDex_Cliente.c
 ============================================================================
 */

#include "PokeDex_Cliente.h"

static const unsigned long long CUATROGB = 4294967296;
/************************************* JOEL GLOBALES *************************************************/
static int posDelaTablaDeArchivos = -999;
static int parent_directory = -999;
static int HABILITAR_MODIFICACION = 0;
static int HIZO_TRUNCATE = 0;
static int FILE_SIZE;
//TODO: TRUNCATE, HAGO LA MAODIFICACION
//TODO: SIN TRUNCATE, BORRO
//TODO: PONER CONTROL DE NOMBRE EL SERVIDOR, O EN EL OSADA
/************************************* FIN GLOBALES *************************************************/

int crearDirectorio(char *path){
			int exitCode = EXIT_FAILURE; //DEFAULT Failure
			int resultado = 1;
			t_list *listaBloques = list_create();
			enum_FUSEOperations fuseOperation =  FUSE_MKDIR;

			//0) Send Fuse Operations
			exitCode = sendMessage(&socketPokeServer, &fuseOperation , sizeof(fuseOperation));
			log_info(logPokeCliente, "fuseOperation: %d\n", fuseOperation);
			string_append(&path, "\0");

			//1) send path length (+1 due to \0)
			int pathLength = strlen(path) + 1;
			exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));

			log_info(logPokeCliente, "pathLength: %i\n", pathLength);

			//2) send path
			exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
			log_info(logPokeCliente, "path: %s\n", path);

			//Receive element Count
			int elementCount = -1;
			int receivedBytes = receiveMessage(&socketPokeServer, &elementCount ,sizeof(elementCount));

			log_info(logPokeCliente, "elementCount: %i\n", elementCount);
			if (receivedBytes > 0 && elementCount > 0){
				int messageSize = elementCount * sizeof(osada_file);
				char *messageRcv = malloc(messageSize);
				receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
				resultado = atoi(messageRcv);
				log_info(logPokeCliente, "messageRcv: %d\n", resultado);
			}
			log_info(logPokeCliente, "*********************************\n");
			return resultado;
};

int renombrarArchivo(char* oldname,char* newName){
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
			int resultado = 1;
			enum_FUSEOperations fuseOperation =  FUSE_RENAME;

			//0) Send Fuse Operations
			exitCode = sendMessage(&socketPokeServer, &fuseOperation , sizeof(fuseOperation));
			log_info(logPokeCliente, "renombrarArchivo - fuseOperation: %d\n", fuseOperation);
			string_append(&oldname, "\0");
			string_append(&newName, "\0");

			//1) send path length (+1 due to \0)
			int pathLength = strlen(oldname) + 1;
			exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));

			log_info(logPokeCliente, "renombrarArchivo - pathLength: %i\n", pathLength);

			//2) send path length (+1 due to \0)
			int newPathLength = strlen(newName) + 1;
			exitCode = sendMessage(&socketPokeServer, &newPathLength , sizeof(int));
			log_info(logPokeCliente, "renombrarArchivo - newPathLength: %i\n", newPathLength);

			//3) send path ORIGINAL
			exitCode = sendMessage(&socketPokeServer, oldname , strlen(oldname) + 1 );
			log_info(logPokeCliente, "renombrarArchivo - oldname: %s\n", oldname);



			//4) send path RENOMBRADO
			exitCode = sendMessage(&socketPokeServer, newName , newPathLength );
			log_info(logPokeCliente, "renombrarArchivo - newName: %s\n", newName);

			//5) send path RENOMBRADO
			exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(int) );
			log_info(logPokeCliente, "renombrarArchivo - parent_directory: %i\n", parent_directory);

			//Receive element Count
			int osada_block_pointer = -1;
			int receivedBytes = receiveMessage(&socketPokeServer, &osada_block_pointer ,sizeof(int));

			log_info(logPokeCliente, "renombrarArchivo - osada_block_pointer: %i\n", osada_block_pointer);

			return 0;
};

long int enviarArchivo(char* path,off_t offset){
	return 0;
};

const char *full(const char *path) /* Anade un punto de montaje */
{

  char *ep, *buff;

  buff = strdup(path+1); if (buff == NULL) exit(1);
  //strdup: Esta función devuelve una String compuesta de caracteres repetidos.
  //El carácter que conforma la cadena es el primer carácter del argumento Character y
  //se duplica tantas veces como indique Number.

  ep = buff + strlen(buff) - 1; if (*ep == '/') *ep = '\0'; /* Puede que esto no suceda pero ante la duda mejor prevenia */

  if (*buff == '\0') strcpy(buff, "."); /* La magia! */

  return buff;
}

static void *fuse_init(struct fuse_conn_info *conn)
{
  conn->max_write=4187593113,6;

  printf("******************** fuse INIT\n");
  // truco para permitir el montaje como una superposición
  fchdir(save_dir);
  close(save_dir);
  return conn;
}

static int fuse_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	printf("********************************* fuse_getattr *********************\n");
	log_info(logPokeCliente, "****************fuse_getattr****************\n");
	if (strcmp(path, DIRECTORIO_RAIZ) == 0){
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	}else {
		if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
			//Esta funcion llama al socket y pide el bloque
			t_list* listaNodo = obtenerDirectorio(path, FUSE_GETATTR);
			if (listaNodo->elements_count == 1 ){// listaNodo->elements_count SIEMPRE va a ser 1, porque el servidor solo manda 1 elemento
				log_info(logPokeCliente, "FUSE_GETATTR -listaNodo->elements_count: %i\n", listaNodo->elements_count);
				osada_file *nodo =list_get(listaNodo,0);

				if (nodo->state == REGULAR)
				{
					log_info(logPokeCliente, "FUSE_GETATTR - REGULAR - nodo->fname: %s\n", nodo->fname);
					log_info(logPokeCliente, "FUSE_GETATTR - REGULAR - nodo->state: REGULAR\n");
					log_info(logPokeCliente, "FUSE_GETATTR - REGULAR - nodo->file_size: %i\n", nodo->file_size);
					stbuf->st_mode = S_IFREG | 0777;
					stbuf->st_nlink = 1;
					stbuf->st_size = nodo->file_size;

					printf("fuse_getattr - nodo->file_size: %i\n",nodo->file_size);
					//stbuf->st_atim
					parent_directory = nodo->parent_directory;

				}
				else if (nodo->state == DIRECTORY)
				{
					printf("NODO PADRE: %i\n",nodo->parent_directory );
					parent_directory = nodo->parent_directory;
					log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->fname: %s\n", nodo->fname);
					log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->state: DIRECTORY\n");
					stbuf->st_mode = S_IFDIR | 0777;
					stbuf->st_nlink = 2;
				}
			}else{
				res = -ENOENT;
			}
		}//if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp"))
		else{
			res = -ENOENT;
		}

	}

	return res;
}

static int fuse_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;

	int i;
	log_info(logPokeCliente, "****************fuse_readdir****************\n");
	log_info(logPokeCliente, "obtenerDirectorio FUSE_READDIR");
	t_list* nodos = obtenerDirectorio(path, FUSE_READDIR);

	if(nodos!=NULL){
		log_info(logPokeCliente, "FUSE_READDIR - listaNodo->elements_count: %i\n", nodos->elements_count);
		for (i = 0; i < nodos->elements_count; i++){
			osada_file *nodo = list_get(nodos,i);
			printf("NODE: %s\n", nodo->fname);
			if ((nodo->state == DIRECTORY)){
				filler(buffer, nodo->fname, NULL, 0);

			}

			if ((nodo->state == REGULAR)){
					filler(buffer, nodo->fname, NULL , 0);

			}
		}
	}else{
		return -ENOENT;
	}

	return 0;
}

static int fuse_rmdir(const char* path){
	printf("********************************* fuse_rmdir *********************\n");


	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	log_info(logPokeCliente, "****************fuse_mkdir****************\n");
	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_RMDIR;
	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

	string_append(&path, "\0");

	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_rmdir - pathLength: %i\n", pathLength);

	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "fuse_rmdir - path: %s\n", path);

	//3) send parent_directory
	exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
	log_info(logPokeCliente, "fuse_rmdir - parent_directory: %i\n", parent_directory);

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "fuse_rmdir - pos de la tabla de archivos: %i\n", messageSize);


	return 0;


}

static int fuse_create (const char* path, mode_t mode, struct fuse_file_info * fi){
	int exitCode = -1; //DEFAULT Failure
	log_info(logPokeCliente, "**************** fuse_create ****************\n");
	printf("********************************* fuse_create *********************\n");
	char *file_name = strrchr (path, '/') + 1;
	printf("crearUnDirectorio - path: %s\n", path);

	if (string_length(file_name)>17){
		printf("fuse_create - EL fuse_mkdir ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "fuse_create - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		exitCode = -1;
	}else{
		if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){ 	// JOEL: NO DEBE GUARDARSE LOS  .swx y swp
			//0) Send Fuse Operations
			enum_FUSEOperations operacion = FUSE_CREATE;
			exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

			string_append(&path, "\0");
			//1) send path length (+1 due to \0)
			int pathLength = strlen(path) + 1;
			exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
			log_info(logPokeCliente, "fuse_create - pathLength: %i\n", pathLength);
			//2) send path
			exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
			log_info(logPokeCliente, "fuse_create - path: %s\n", path);

			//3) send parent_directory
			exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
			log_info(logPokeCliente, "fuse_create - parent_directory: %i\n", parent_directory);

			//Receive message Status
			int receivedBytes = receiveMessage(&socketPokeServer, &posDelaTablaDeArchivos ,sizeof(posDelaTablaDeArchivos));

			log_info(logPokeCliente, "fuse_create - posDelaTablaDeArchivos: %i\n", posDelaTablaDeArchivos);

			/*if(posDelaTablaDeArchivos == -1){
				printf("fuse_create - NO SE PUEDE CREAR MAS DE 2048\n");
				log_info(logPokeCliente, "fuse_create - NO SE PUEDE CREAR MAS DE 2048\n");
				exitCode = -1;
			}else{
				printf("fuse_create - Se pudo crear el archivo\n");
				exitCode = EXIT_SUCCESS; // no se estaba retornando OK al FUSE cuando la posicion era encontrada
			}*/

		}else{
			exitCode = EXIT_SUCCESS;
		}
	}
	return exitCode;
}


static int fuse_mkdir(const char* path, mode_t mode){

	printf("********************************* fuse_mkdir *********************\n");
	char *file_name = strrchr (path, '/') + 1;
	printf("crearUnDirectorio - path: %s\n", path);
	if (string_length(file_name)>17){
		printf("fuse_mkdir - EL fuse_mkdir ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "fuse_mkdir - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		return -1;
	}

    mode = S_IFDIR | 0777;
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	log_info(logPokeCliente, "****************fuse_mkdir****************\n");
	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_MKDIR;
	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

	string_append(&path, "\0");

	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_mkdir - pathLength: %i\n", pathLength);

	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "fuse_mkdir - path: %s\n", path);

	//3) send parent_directory
	exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
	log_info(logPokeCliente, "fuse_mkdir - parent_directory: %i\n", parent_directory);

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "fuse_mkdir - pos de la tabla de archivos: %i\n", messageSize);


	return 0;


}


static int fuse_truncate(const char* path, off_t offset)
{
//	int resultado = borrarArchivo (path);
	// truncate --size=-5 - ESTO RESTA EL TAMANO ACTUAL CON 5, viejo 12 - 5 =7 - le quito caracteres
	// truncate --size=+5 - ESTO suma EL TAMANO ACTUAL CON 5, viejo 12 + 5 =17 - agrego espacio
	// truncate --size=5 - libero 5byes - agreo espacio
	// truncate -s 2k 6 - libero 2k

	//tengo 17
	//1024


	printf("********************************* fuse_truncate *********************\n");
	//-2147483648
	printf("offset: %llu\n", offset);
	printf("FILE_SIZE: %i\n", FILE_SIZE);
	if (offset > CUATROGB) {
		printf("NO CUMPLE TAMANIO DE ARCHIVO OSADA: %llu\n", offset);
		return -1;
	}

	if (FILE_SIZE == 0 && offset == 0) {
		printf("ESTA EN TRUNCATE, PERO ACABA DE HACER EL WRITE\n");
		return 0;
	}

	//printf("buf: %s\n", buf);

	//ME FIJO LOS BYTES.
	//SI ES MAYOR AL ACTUAL, SE LE ASIGNA MAS BLOQUES SI ES NECESARIO
	//SI ES MAYOR AL ACTUAL,  SE LE ASGINA MENOS BLOQUES SI ES NECESARIO.
	//SI ES IGUAL AL ACTUAL, NO SE ALTERA NADA.

	log_info(logPokeCliente, "--------------------- fuse_truncate ------------ \n");
	int resultado = 0;
	int exitCode;

	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
		log_info(logPokeCliente, "**************** FUSE_TRUNCATE ****************\n");
		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_TRUNCATE;
		exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

		string_append(&path, "\0");
		//1) send path length (+1 due to \0)
		int pathLength = strlen(path) + 1;
		exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_truncate - pathLength: %i\n", pathLength);

		//2) send path
		exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
		log_info(logPokeCliente, "fuse_truncate - path: %s\n", path);

		//3) send parent_directory
		exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
		log_info(logPokeCliente, "fuse_truncate - parent_directory: %i\n", parent_directory);

		//4) send parent_directory
		exitCode = sendMessage(&socketPokeServer, &offset , sizeof(offset));
		log_info(logPokeCliente, "fuse_truncate - parent_directory: %i\n", offset);

		//Receive message size
		int messageSize = -1;
		int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
		log_info(logPokeCliente, "fuse_truncate - MessageSize: %i\n", messageSize);

	}
		else
	{
		exitCode = EXIT_SUCCESS;
	}

	return resultado;

}

static int fuse_unlink(const char* path, int hizoElOpen)
{
//	int resultado = borrarArchivo (path);
	printf("********************************* fuse_unlink *********************\n");
	printf(" hizoElOpen: %i \n",hizoElOpen);
	log_info(logPokeCliente, "--------------------- fuse_unlink ------------ %s\n", path);
	int resultado = 1;
	int exitCode;

	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
		if (HIZO_TRUNCATE){
			printf("********************************* hizo fuseOPEN | HIZO_TRUNCATE | ahora fuse_unlink *********************\n");
			HIZO_TRUNCATE = 0;
			return 0;
		}else if(hizoElOpen != 666){//PARA BORRAR, NO TIENE QUE HACER EL OPEN, NI EL TRUNCATE.
			printf("HIZO_TRUNCATE: %i\n",HIZO_TRUNCATE);
			log_info(logPokeCliente, "****************FUSE_UNLINK****************\n");


				//0) Send Fuse Operations
				enum_FUSEOperations operacion = FUSE_UNLINK;
				exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

				string_append(&path, "\0");
				//1) send path length (+1 due to \0)
				int pathLength = strlen(path) + 1;
				exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
				log_info(logPokeCliente, "fuse_unlink - pathLength: %i\n", pathLength);

				//2) send path
				exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
				log_info(logPokeCliente, "fuse_unlink - path: %s\n", path);

				//3) send parent_directory
				exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
				log_info(logPokeCliente, "fuse_unlink - parent_directory: %i\n", parent_directory);

				//Receive message size
				int messageSize = -1;
				int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
				log_info(logPokeCliente, "fuse_unlink - MessageSize: %i\n", messageSize);
		}
	}else{
		exitCode=EXIT_SUCCESS;
	}


		return 0;
}

static int fuse_open(const char *path, struct fuse_file_info *fi) {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	printf("********************************* fuse_open ********************* \n");
	log_info(logPokeCliente, "--------------------- fuse_open ------------ \n");
	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
		log_info(logPokeCliente, "****************FUSE_UNLINK****************\n");

		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_OPEN;
		exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

		string_append(&path, "\0");
		//1) send path length (+1 due to \0)
		int pathLength = strlen(path) + 1;
		exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_open - pathLength: %i\n", pathLength);

		//2) send path
		exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
		log_info(logPokeCliente, "fuse_open - path: %s\n", path);

		//3) send parent_directory
		exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
		log_info(logPokeCliente, "fuse_open - parent_directory: %i\n", parent_directory);

		//Receive message file_size
		int receivedBytes = receiveMessage(&socketPokeServer, &FILE_SIZE ,sizeof(FILE_SIZE));
		log_info(logPokeCliente, "fuse_open - file_size: %i\n", FILE_SIZE);
		HABILITAR_MODIFICACION = 1;
		fuse_unlink(path, 666);

	}else{
		exitCode=EXIT_SUCCESS;
	}


	return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp") && !string_is_empty(path)){
		printf("********************************* fuse_read *********************\n");
		printf("********************************* fuse_read - size: %i\n", size);
		printf("********************************* fuse_read - offset: %i\n", offset);

		log_info(logPokeCliente, "****************fuse_read****************\n");
		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_READ;
		exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

		string_append(&path, "\0");
		//1) send path length (+1 due to \0)
		int pathLength = strlen(path) + 1;
		exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_read - pathLength: %i\n", pathLength);

		//2) send path
		exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
		log_info(logPokeCliente, "fuse_read - path: %s\n", path);

		//3) send parent_directory
		exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
		log_info(logPokeCliente, "fuse_read - parent_directory: %i\n", parent_directory);

		//Receive message size
		unsigned long int messageSize = -1;
		int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
		log_info(logPokeCliente, "fuse_read - MessageSize: %lu\n", messageSize);

		if (receivedBytes > 0){
			char *messageRcv = malloc(messageSize);
			receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
			//log_info(logPokeCliente, "messageRcv: %s\n", messageRcv);
			memcpy(buf, messageRcv, messageSize);
			log_info(logPokeCliente, "fuse_read - HIZO MEMCPY \n");
			return messageSize;
		}
	}else{
		exitCode=EXIT_SUCCESS;
	}

	return exitCode;
}
void modificarElArchivo(const char* path, const char* buf, size_t size){
	int bytes_escritos = 0;
	printf("********************************* FUSE_MODIFICAR *********************\n");
	printf("buf: %s\n", buf);
	printf("size: %i\n", size);
	log_info(logPokeCliente, "FUSE_MODIFICAR  - path: %s\n", path);
		log_info(logPokeCliente, "FUSE_MODIFICAR  - buf: %s\n", buf);
		log_info(logPokeCliente, "FUSE_MODIFICAR  - size: %i\n", size);
		int exitCode = EXIT_FAILURE; //DEFAULT Failure
		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_MODIFICAR;

		log_info(logPokeCliente, "FUSE_MODIFICAR -  ENVIO MENSAJE\n");
		exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));
		printf("********************************* sendMessage 1 *********************\n");
		log_info(logPokeCliente, "FUSE_MODIFICAR -  RECIBIO MENSAJE\n");

		string_append(&path, "\0");
		log_info(logPokeCliente, "FUSE_MODIFICAR -  &path: %s\n", path);
		log_info(logPokeCliente, "FUSE_MODIFICAR -  &buf: %s\n", buf);
		//1) send path length (+1 due to \0)
		int pathLength = strlen(path) + 1;
		log_info(logPokeCliente, "FUSE_MODIFICAR -  ENVIO MENSAJE: %i\n",pathLength);
		exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_write - pathLength: %i\n", pathLength);
		printf("********************************* sendMessage 2 *********************\n");
		//2) send path
		exitCode = sendMessage(&socketPokeServer, path , pathLength );
		log_info(logPokeCliente, "FUSE_MODIFICAR - path: %s\n", path);
		printf("********************************* sendMessage 3 *********************\n");

		//3) send buffer length (+1 due to \0)
		int bufferSize = size;
		exitCode = sendMessage(&socketPokeServer, &bufferSize , sizeof(int));
		log_info(logPokeCliente, "FUSE_MODIFICAR - bufferSize: %i\n", bufferSize);
		printf("********************************* sendMessage \4 *********************\n");
		//4) send buffer
		exitCode = sendMessage(&socketPokeServer, buf , bufferSize );
		printf("********************************* sendMessage 4 *********************\n");
		log_info(logPokeCliente, "FUSE_MODIFICAR - buffer: %s\n", buf);

		//5) send posDelaTablaDeArchivos
		exitCode = sendMessage(&socketPokeServer, &posDelaTablaDeArchivos , sizeof(int) );
		log_info(logPokeCliente, "FUSE_MODIFICAR - posDelaTablaDeArchivos: %i\n", posDelaTablaDeArchivos);
		printf("********************************* sendMessage 5 *********************\n");

		//6) send parent_directory
		exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(parent_directory));
		log_info(logPokeCliente, "FUSE_MODIFICAR - parent_directory: %i\n", parent_directory);
		printf("********************************* sendMessage 6 *********************\n");

		//Receive message size
		int receivedBytes = receiveMessage(&socketPokeServer, &bytes_escritos ,sizeof(bytes_escritos));
		log_info(logPokeCliente, "FUSE_MODIFICAR - bytes_escritos: %i\n", bytes_escritos);
		HIZO_TRUNCATE = 0;
		usleep(500000);

}

static int fuse_write(const char* path, const char* buf, size_t size,  off_t offset)
{
	int ultimoPunteroDeLosBloques_write = 666;

	printf("********************************* fuse_write *********************\n");
/*
	if (size > CUATROGB) {
		printf("NO CUMPLE TAMANIO DE ARCHIVO OSADA: %i\n", size);
		return -1;
	}
*/

	char *file_name = strrchr (path, '/') + 1;
	printf("fuse_write - path: %s\n", path);

	if (string_length(file_name)>17){
		printf("fuse_write - EL fuse_write ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "fuse_write - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		return -1;
	}

	/*
	if (HABILITAR_MODIFICACION == 1){
		modificarElArchivo(path, buf, size);
		return size;
	}
	*/

	printf("fuse_write - size: %i\n", size);
	printf("fuse_write - offset: %llu\n", offset);
	printf("fuse_write - buf: %s\n", buf);

	if (size > offset){
		printf("PRIMERA LLAMADA \n");
	}

	if (size == offset){
		printf("SEGUNDA LLAMADA \n");
	}

	if (size < offset){
		printf("SEGUNDA LLAMADA o mas \n");
	}
	//bytes_escritos = enviarArchivo(path,offset);
	log_info(logPokeCliente, "fuse_write - path: %s\n", path);
	//log_info(logPokeCliente, "fuse_write - buf: %s\n", buf);
	log_info(logPokeCliente, "fuse_write - size: %i\n", size);

	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_WRITE;

	log_info(logPokeCliente, "fuse_write -  ENVIO MENSAJE\n");
	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));
	//printf("********************************* sendMessage 1 *********************\n");
	log_info(logPokeCliente, "fuse_write -  RECIBIO MENSAJE\n");

	string_append(&path, "\0");
	log_info(logPokeCliente, "fuse_write -  &path: %s\n", path);
	//log_info(logPokeCliente, "fuse_write -  &buf: %s\n", buf);
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	log_info(logPokeCliente, "fuse_write -  ENVIO MENSAJE: %i\n",pathLength);
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_write - pathLength: %i\n", pathLength);
	//printf("********************************* sendMessage 2 *********************\n");
	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , pathLength );
	log_info(logPokeCliente, "fuse_write - path: %s\n", path);
	//printf("********************************* sendMessage 3 *********************\n");

	//3) send buffer length (+1 due to \0)
	int bufferSize = size;
	exitCode = sendMessage(&socketPokeServer, &bufferSize , sizeof(int));
	log_info(logPokeCliente, "fuse_write - bufferSize: %i\n", bufferSize);
	//printf("********************************* sendMessage \4 *********************\n");
	//4) send buffer
	exitCode = sendMessage(&socketPokeServer, buf , bufferSize );
	//printf("********************************* sendMessage 4 *********************\n");
	//log_info(logPokeCliente, "fuse_write - buffer: %s\n", buf);

	//5) send posDelaTablaDeArchivos
	exitCode = sendMessage(&socketPokeServer, &posDelaTablaDeArchivos , sizeof(int) );
	log_info(logPokeCliente, "fuse_write - posDelaTablaDeArchivos: %i\n", posDelaTablaDeArchivos);
	//printf("********************************* sendMessage 5 *********************\n");

	//6) send parent_directory
	exitCode = sendMessage(&socketPokeServer, &parent_directory , sizeof(int));
	log_info(logPokeCliente, "fuse_write - parent_directory: %i\n", parent_directory);
	//printf("********************************* sendMessage 6 *********************\n");

	//Receive message size
	int ultimoPunteroDeLosBloques_write2;
	int receivedBytes = receiveMessage(&socketPokeServer, &ultimoPunteroDeLosBloques_write2 ,sizeof(int));
	log_info(logPokeCliente, "fuse_write -RECEIVE - ultimoPunteroDeLosBloques_write2: %d\n", ultimoPunteroDeLosBloques_write2);
	printf("fuse_write - ultimoPunteroDeLosBloques_write2: %d\n", ultimoPunteroDeLosBloques_write2);
	usleep(500000);
	buf="";
	return size;
}
//TODO
//resultado esta devolviendo -999
static int fuse_rename (const char *oldname, const char *newName){
	printf("********************************* fuse_rename *********************\n");
	if (string_length(newName)>17){
		printf("RENAME - EL RENAME ES MAYOR A 17: %i\n", string_length(newName));
		log_info(logPokeCliente, "RENAME - EL RENAME ES MAYOR A 17: %i\n", string_length(newName));
		return -1;
	}

	int resultado = renombrarArchivo(oldname, newName);

	if (resultado!=-999)	{
			printf("[Error_Fuse] rename(%s,%s)\n", oldname, newName);
			return 1;
		}

	return 1;

}
static int fuse_utimens(const char * path, const struct timespec ts[2]){
	printf("***************** fuse_utimens - ts[1].tv_nsec: %i***********************\n", ts[1].tv_nsec);
	//			NOTA VEAMOS JUNTOS COMO IMPLEMENTARLO LO QUE HAY QUE OBTENER ES EL
	//			uint32_t lastmod;
	//			int messageSize = elementCount * sizeof(osada_file);
	//			uint32_t *messageRcv = malloc(messageSize);
	//			receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
	//			resultado = atoi(messageRcv);
	//			log_info(logPokeCliente, "messageRcv: %d\n", resultado);

	return 0;
}



static int fuse_mknod(const char* path, mode_t mode, dev_t rdev){
	printf("***************** FUSE_MKNOD***********************\n");
	return 1;
}

static int fuse_release(const char *path, struct fuse_file_info *file){
	printf("***************** fuse_release ***********************\n");
	return 1;
}

static int fuse_releasedir(const char *path, struct fuse_file_info *file){
	printf("***************** fuse_releasedir ***********************\n");
	return 0;
}

static int fuse_opendir(const char *path, struct fuse_file_info *file){
	printf("***************** fuse_opendir ***********************\n");
	return 0;
}

static struct fuse_operations xmp_oper = {
    .init       = fuse_init,
    .getattr	= fuse_getattr,
    .readdir	= fuse_readdir,
	.create 	= fuse_create,
    .mkdir		= fuse_mkdir,
    .unlink		= fuse_unlink,
    .rmdir		= fuse_rmdir,
    .rename		= fuse_rename,
    .open		= fuse_open,
    .read		= fuse_read,
    .write		= fuse_write,
	.truncate   = fuse_truncate,
	.utimens    = fuse_utimens,
	.mknod		= fuse_mknod,
	.release	= fuse_release,
	.opendir	= fuse_opendir,
	.releasedir	= fuse_releasedir,
};





int main(int argc, char **argv) {
	char *logFile = NULL;
	char *mountPoint = string_new();
	int exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

	char *el_fuse;

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	int logPosition = -1;
	for (i = 0; i < argc; i++) {
		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			logPosition = i;
			printf("Log File: '%s'\n", logFile);
		}
	}

	//delete parameter
	if (logPosition != -1){
		//has to be deleted the parameter and its value
		argv[logPosition] = argv[logPosition + 2];
		argv[logPosition + 1] = argv[logPosition + 3];
		argc = argc - 2 ;
	}

	logPokeCliente = log_create(logFile, "POKEDEX_CLIENTE", 0, LOG_LEVEL_TRACE);

	//getting environment variable for connecting to server
	IP_SERVER = getenv("POKEIP");
	PORT = atoi(getenv("POKEPORT"));
	printf("%s\n",IP_SERVER);
	printf("%i\n",PORT);
	exitCode = connectTo(POKEDEX_SERVIDOR, &socketPokeServer);
	printf("%i\n",exitCode);
	if (exitCode == EXIT_SUCCESS) {
		log_info(logPokeCliente, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");
		printf("argv: %s\n", argv);
		printf("argc: %i\n", argc);

		return fuse_main(argc, argv, &xmp_oper, NULL);

	} else {
		log_error(logPokeCliente,"No server available - shutting down proces!!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int connectTo(enum_processes processToConnect, int *socketClient) {
	int exitcode = EXIT_FAILURE; //DEFAULT VALUE
	int port;
	char *ip = string_new();

	switch (processToConnect) {
		case POKEDEX_SERVIDOR: {
			string_append(&ip, IP_SERVER);
			port = PORT;
			break;
		}
		default: {
			log_info(logPokeCliente, "Process '%s' NOT VALID to be connected.\n",getProcessString(processToConnect));
			break;
		}
	}
	printf("- openClientConnection -\n");
	exitcode = openClientConnection(ip, port, socketClient);
	printf("exitcode: %i\n",exitcode);
	//If exitCode == 0 the client could connect to the server
	if (exitcode == EXIT_SUCCESS) {

		// ***1) Send handshake
		exitcode = sendClientHandShake(socketClient, POKEDEX_CLIENTE);

		if (exitcode == EXIT_SUCCESS) {

			// ***2)Receive handshake response
			//Receive message size
			int messageSize = 0;
			char *messageRcv = malloc(sizeof(messageSize));
			int receivedBytes = receiveMessage(socketClient, messageRcv,sizeof(messageSize));

			if (receivedBytes > 0) {
				//Receive message using the size read before
				memcpy(&messageSize, messageRcv, sizeof(int));
				messageRcv = realloc(messageRcv, messageSize);
				receivedBytes = receiveMessage(socketClient, messageRcv,messageSize);

				//starting handshake with client connected
				t_MessageGenericHandshake *message = malloc(sizeof(t_MessageGenericHandshake));
				deserializeHandShake(message, messageRcv);

				free(messageRcv);

				switch (message->process) {
				case ACCEPTED: {
					log_info(logPokeCliente, "Conectado a POKE SERVER - Messsage: %s\n",message->message);
					break;
				}
				default: {
					log_error(logPokeCliente,"Process couldn't connect to SERVER - Not able to connect to server %s. Please check if it's down.\n",ip);
					close(*socketClient);
					break;
				}
				}
			} else if (receivedBytes == 0) {
				//The client is down when bytes received are 0
				log_error(logPokeCliente,"The client went down while receiving! - Please check the client '%d' is down!\n",	*socketClient);
				close(*socketClient);
			} else {
				log_error(logPokeCliente,"Error - No able to received - Error receiving from socket '%d', with error: %d\n",*socketClient, errno);
				close(*socketClient);
			}
		}

	} else {
		log_error(logPokeCliente,"I'm not able to connect to the server! - My socket is: '%d'\n",*socketClient);
		close(*socketClient);
	}

	return exitcode;
}

//TODO CUANDO CREO UN ARCHIVO, LLAMA AL ATRIBUTO
t_list * obtenerDirectorio(const char* path, enum_FUSEOperations fuseOperation){
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	enum_FUSEOperations fuseOperation2;

	t_list *listaBloques = list_create();
	if (fuseOperation == FUSE_GETATTR){
		fuseOperation2 = FUSE_GETATTR;
	}else
	{
		fuseOperation2 = FUSE_READDIR;
	}
	//0) Send Fuse Operations
	exitCode = sendMessage(&socketPokeServer, &fuseOperation2 , sizeof(fuseOperation2));
	log_info(logPokeCliente, "fuseOperation2: %d", fuseOperation2);
	printf("fuseOperation2: %d\n", fuseOperation2);


	string_append(&path, "\0");
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "pathLength: %i\n", pathLength);
	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "path: %s\n", path);

	//Receive element Count
	int elementCount = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &elementCount ,sizeof(elementCount));
	log_info(logPokeCliente, "elementCount: %i\n", elementCount);
	if (receivedBytes > 0 && elementCount > 0){
		int messageSize = elementCount * sizeof(osada_file);
		char *messageRcv = malloc(messageSize);
		receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
		log_info(logPokeCliente, "messageRcv: %s\n", messageRcv);
		deserializeListaBloques(listaBloques,messageRcv,elementCount);
	}
	log_info(logPokeCliente, "*********************************\n");
	return listaBloques;

}

