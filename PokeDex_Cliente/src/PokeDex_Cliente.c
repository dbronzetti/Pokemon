/*
 ============================================================================
 Name        : PokeDex_Cliente.c
 ============================================================================
 */

#include "PokeDex_Cliente.h"

static const unsigned long long CUATROGB = 4294967296;

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
	enum_FUSEOperations fuseOperation =  FUSE_RENAME;

	//0) Send Fuse Operations
	sendMessage(&socketPokeServer, &fuseOperation , sizeof(fuseOperation));
	log_info(logPokeCliente, "renombrarArchivo - fuseOperation: %d\n", fuseOperation);
	string_append(&oldname, "\0");
	string_append(&newName, "\0");

	//1) send path length (+1 due to \0)
	int pathLength = strlen(oldname) + 1;
	sendMessage(&socketPokeServer, &pathLength , sizeof(int));

	log_info(logPokeCliente, "renombrarArchivo - pathLength: %i\n", pathLength);

	//2) send path length (+1 due to \0)
	int newPathLength = strlen(newName) + 1;
	sendMessage(&socketPokeServer, &newPathLength , sizeof(int));
	log_info(logPokeCliente, "renombrarArchivo - newPathLength: %i\n", newPathLength);

	//3) send path ORIGINAL
	sendMessage(&socketPokeServer, oldname , strlen(oldname) + 1 );
	log_info(logPokeCliente, "renombrarArchivo - oldname: %s\n", oldname);

	//4) send path RENOMBRADO
	sendMessage(&socketPokeServer, newName , newPathLength );
	log_info(logPokeCliente, "renombrarArchivo - newName: %s\n", newName);

	//Receive element Count
	receiveMessage(&socketPokeServer, &exitCode ,sizeof(exitCode));

	log_info(logPokeCliente, "renombrarArchivo - exitCode: %i\n", exitCode);

	return exitCode;
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
	printf("********************************* INICIO: fuse_getattr *********************\n");
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
					//stbuf->st_atim.tv_nsec = nodo->lastmod;

					log_info(logPokeCliente, "FUSE_GETATTR - REGULAR - nodo->parent_directory: %i\n", nodo->parent_directory);

				}
				else if (nodo->state == DIRECTORY)
				{
					log_info(logPokeCliente, "NODO PADRE: %i\n",nodo->parent_directory );
					log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->fname: %s\n", nodo->fname);
					log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->state: DIRECTORY\n");
					stbuf->st_mode = S_IFDIR | 0777;
					stbuf->st_nlink = 2;
					//stbuf->st_atim.tv_nsec = nodo->lastmod;
				}
				free(nodo);
			}else{
				res = -ENOENT;
			}
			list_destroy(listaNodo);
		}//if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp"))
		else{
			res = -ENOENT;
		}
	}
	printf("********************************* FIN - fuse_getattr *********************\n");
	return res;
}

static int fuse_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;

	int i;
	log_info(logPokeCliente, "****************fuse_readdir****************\n");
	log_info(logPokeCliente, "obtenerDirectorio FUSE_READDIR");
	t_list* nodos = obtenerDirectorio(path, FUSE_READDIR);

	if(nodos->elements_count>0){
		log_info(logPokeCliente, "FUSE_READDIR - listaNodo->elements_count: %i\n", nodos->elements_count);
		for (i = 0; i < nodos->elements_count; i++){
			osada_file *nodo = list_get(nodos,i);

			if ((nodo->state == DIRECTORY)){
				filler(buffer, nodo->fname, NULL, 0);
			}

			if ((nodo->state == REGULAR)){
				filler(buffer, nodo->fname, NULL , 0);
			}

			free(nodo);
		}

	}
	list_destroy(nodos);
	return 0;
}

static int fuse_rmdir(const char* path){
	int exitCode;

	log_info(logPokeCliente, "****************fuse_rmdir****************\n");
	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_RMDIR;
	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

	string_append(&path, "\0");

	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_rmdir - pathLength: %d", pathLength);

	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "fuse_rmdir - path: %s", path);

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &exitCode ,sizeof(exitCode));
	log_info(logPokeCliente, "fuse_rmdir - exitCode received: %i", exitCode);

	if (exitCode == 1){
		exitCode = -ENOENT;
	}

	return exitCode;


}

static int fuse_create (const char* path, mode_t mode, struct fuse_file_info * fi){
	int exitCode = -1; //DEFAULT Failure
	log_info(logPokeCliente, "**************** fuse_create ****************\n");
	printf("********************************* fuse_create *********************\n");
	char *file_name = strrchr (path, '/') + 1;
	printf("fuse_create - path: %s\n", path);


	if (string_length(file_name)>17){
		printf("fuse_create - EL fuse_create ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "fuse_create - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		exitCode = -1;
	}else{
		if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){ 	// JOEL: NO DEBE GUARDARSE LOS  .swx y swp
			int posDelaTablaDeArchivos = -999;
			//0) Send Fuse Operations
			enum_FUSEOperations operacion = FUSE_CREATE;
			exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

			//1) send path length (+1 due to \0)
			string_append(&path, "\0");
			int pathLength = strlen(path) + 1;
			exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
			log_info(logPokeCliente, "fuse_create - pathLength: %i\n", pathLength);

			//2) send path
			exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
			log_info(logPokeCliente, "fuse_create - path: %s\n", path);

			//Receive message Status
			int receivedBytes = receiveMessage(&socketPokeServer, &posDelaTablaDeArchivos ,sizeof(posDelaTablaDeArchivos));

			log_info(logPokeCliente, "fuse_create - posDelaTablaDeArchivos: %i\n", posDelaTablaDeArchivos);

			if(posDelaTablaDeArchivos == -1){
				printf("fuse_create - NO SE PUEDE CREAR MAS DE 2048\n");
				log_info(logPokeCliente, "fuse_create - NO SE PUEDE CREAR MAS DE 2048\n");
				exitCode = -1;
			}else{
				printf("fuse_create - Se pudo crear el archivo\n");
				exitCode = EXIT_SUCCESS; // no se estaba retornando OK al FUSE cuando la posicion era encontrada
			}

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

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "fuse_mkdir - pos de la tabla de archivos: %i\n", messageSize);


	return 0;


}


static int fuse_truncate(const char* path, off_t offset) {

	if (offset > CUATROGB) {
		printf("NO CUMPLE TAMANIO DE ARCHIVO OSADA: %llu\n", offset);
		return -1;
	}

	int exitCode;

	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
		log_info(logPokeCliente, "--------------------- fuse_truncate ------------ \n");
		log_info(logPokeCliente,"offset: %llu\n", offset);

		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_TRUNCATE;
		sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

		//1) send path length (+1 due to \0)
		string_append(&path, "\0");
		int pathLength = strlen(path) + 1;

		sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_truncate - pathLength: %i\n", pathLength);

		//2) send path
		sendMessage(&socketPokeServer, path , pathLength );
		log_info(logPokeCliente, "fuse_truncate - path: %s\n", path);

		//3) send offsetSend length (+1 due to \0)
		int offsetSend = offset;
		sendMessage(&socketPokeServer, &offsetSend , sizeof(offsetSend));

		//Receive message size
		receiveMessage(&socketPokeServer, &exitCode ,sizeof(exitCode));
		log_info(logPokeCliente, "fuse_truncate -RECEIVE - exitCode: %d", exitCode);

	}else{

		exitCode = EXIT_SUCCESS;
	}

	return exitCode;

}

static int fuse_unlink(const char* path){
//	int resultado = borrarArchivo (path);
	log_info(logPokeCliente, "--------------------- fuse_unlink ------------ %s\n", path);
	int exitCode;

	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp")){
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

		//Receive message size
		int receivedBytes = receiveMessage(&socketPokeServer, &exitCode ,sizeof(exitCode));
		log_info(logPokeCliente, "fuse_unlink - exitCode: %i\n", exitCode);
	}else{
		exitCode=EXIT_SUCCESS;
	}


	return exitCode;
}

static int fuse_open(const char *path, struct fuse_file_info *fi) {
	return 0;
}

/*
 * 	if (size + offset == 1048576){
		printf("********************************* 1048576: %i\n", size + offset);
		return 0;
	}
 */

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int exitCode = EXIT_FAILURE; //DEFAULT Failure


	if(!string_ends_with(path, "swx") && !string_ends_with(path, "swp") && !string_is_empty(path)){

		int fileFound = -1;
		int receivedBytes;
		log_info(logPokeCliente, "****************fuse_read****************\n");
		log_info(logPokeCliente,"********************************* fuse_read - size: %i\n", size);
		log_info(logPokeCliente,"********************************* fuse_read - offset: %i\n", offset);


		//0) Send Fuse Operations
		enum_FUSEOperations operacion = FUSE_READ;
		exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

		string_append(&path, "\0");
		//1) send path length (+1 due to \0)
		int pathLength = strlen(path) + 1;
		exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
		log_info(logPokeCliente, "fuse_read - pathLength: %i\n", pathLength);

		//2) send path
		exitCode = sendMessage(&socketPokeServer, path , pathLength );
		log_info(logPokeCliente, "fuse_read - path: %s\n", path);

		//3) send offset
		int offsetTosend = offset; // sending an int because off_t has issues with the sockets
		exitCode = sendMessage(&socketPokeServer, &offsetTosend , sizeof(offsetTosend));
		log_info(logPokeCliente, "fuse_read - offset: %i\n", offsetTosend);

		//4) send size to read
		int sizeTosend = size; //sending an int because size_t has issues with the sockets
		exitCode = sendMessage(&socketPokeServer, &sizeTosend , sizeof(sizeTosend));
		log_info(logPokeCliente, "fuse_read - size to read: %i\n", sizeTosend);

		//Receive if the file was found
		receivedBytes = receiveMessage(&socketPokeServer, &fileFound ,sizeof(fileFound));
		log_info(logPokeCliente, "fuse_read - fileFound: %d\n", fileFound);
//		pthread_mutex_lock(&ReadMutex);

		if (receivedBytes > 0){
			if (fileFound != -999){//el archivo no fue encontrado por el server

				int off_bloque = (offset % OSADA_BLOCK_SIZE);

				int messageSize = 0;
				memset(buf, 0 , size); //Limpio el buffer
				int bytes_leidos = 0;

				int cant_bloques_por_leer = size / OSADA_BLOCK_SIZE;
				int bytes_por_leer = size % OSADA_BLOCK_SIZE;

				log_info(logPokeCliente, "fuse_read - cant_bloques_por_leer: %d\n", cant_bloques_por_leer );

				//Receive message size
				receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));

				char *messageRcv = malloc(messageSize);
				receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);

				int bloqueLeido = 0;

				if ((OSADA_BLOCK_SIZE - off_bloque) >= size){ //menos de un bloque para leer.
					//Receive message size
					memcpy(buf, messageRcv + (bloqueLeido * OSADA_BLOCK_SIZE), size);
					bytes_leidos = size;

				}else{ //1 Bloque entero y un poco mas || N Bloques enteros y un poco mas
					memcpy(buf, messageRcv + (bloqueLeido * OSADA_BLOCK_SIZE), OSADA_BLOCK_SIZE - off_bloque);
					bloqueLeido++;
					bytes_leidos += OSADA_BLOCK_SIZE - off_bloque;
					size -= OSADA_BLOCK_SIZE - off_bloque;

					cant_bloques_por_leer = size / OSADA_BLOCK_SIZE;
					bytes_por_leer = size % OSADA_BLOCK_SIZE;

					if (cant_bloques_por_leer == 0){
						memcpy(buf + bytes_leidos, messageRcv + (bloqueLeido * OSADA_BLOCK_SIZE), bytes_por_leer);
						bloqueLeido++;
						bytes_leidos += bytes_por_leer;
						size -= bytes_por_leer;

					}else{
						int k;
						for (k = 1; k <= cant_bloques_por_leer; k++){
							//log_info(logPokeCliente, "fuse_read - MessageSize #'%d': %d\n",k , messageSize);

							memcpy(buf + bytes_leidos, messageRcv + (bloqueLeido * OSADA_BLOCK_SIZE), OSADA_BLOCK_SIZE);
							bloqueLeido++;
							bytes_leidos += OSADA_BLOCK_SIZE;
							size -= OSADA_BLOCK_SIZE;
							//string_append(&buf, messageRcv);
						}

						printf("read - cantidadBloques: %i\n", k);

						if(bytes_por_leer > 0){
							printf("entro en el bytes por leet\n");
							//log_info(logPokeCliente, "fuse_read - MessageSize #'%d': %d\n",i , messageSize);

							memcpy(buf + bytes_leidos, messageRcv + (bloqueLeido * OSADA_BLOCK_SIZE), bytes_por_leer);
							bytes_leidos += bytes_por_leer;
							size -= bytes_por_leer;
						}
					}
				}

				free(messageRcv);
				//log_info(logPokeCliente, "messageRcv: %s\n", messageRcv);
				//memcpy(buf, "hola\0", strlen("hola\0")+1);
				log_info(logPokeCliente, "fuse_read - buf: %s\n", buf);
				log_info(logPokeCliente, "fuse_read - bytes_leidos: %d\n", bytes_leidos);

				return bytes_leidos;

			}else{
				exitCode = EXIT_SUCCESS;
			}//if (cantidadBloques != -999)
		}else{
			exitCode=EXIT_SUCCESS;
		}//if (receivedBytes > 0)
//		pthread_mutex_unlock(&ReadMutex);
	}else{
		exitCode=EXIT_SUCCESS;
	}


	return exitCode;
}

static int fuse_write(const char* path, const char* buf, size_t size,  off_t offset) {
	time_t tiempo1 = time(0);
	int ultimoPunteroDeLosBloques_write = 666;


	char *file_name = strrchr (path, '/') + 1;

	if (string_length(file_name)>17){
		//printf("fuse_write - EL fuse_write ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "fuse_write - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		return -1;
	}


	int exitCode = EXIT_FAILURE; //DEFAULT Failure

	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_WRITE;

	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(operacion));

	string_append(&path, "\0");
	//log_info(logPokeCliente, "fuse_write -  &path: %s\n", path);

	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(pathLength));
	//log_info(logPokeCliente, "fuse_write - pathLength: %i\n", pathLength);

	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , pathLength );
	//log_info(logPokeCliente, "fuse_write - path: %s\n", path);

	//3) send buffer length (+1 due to \0)
	int bufferSize = size;
	exitCode = sendMessage(&socketPokeServer, &bufferSize , sizeof(bufferSize));
	log_info(logPokeCliente, "fuse_write - size: %d\n", size);
	log_info(logPokeCliente, "fuse_write - bufferSize: %d\n", bufferSize);

	//4) send buffer
	exitCode = sendMessage(&socketPokeServer, buf , bufferSize );
	//log_info(logPokeCliente, "fuse_write - buffer: %s\n", buf);

	//5) send offset
	int bufferOffset = offset;
	exitCode = sendMessage(&socketPokeServer, &bufferOffset , sizeof(bufferOffset));
	//log_info(logPokeCliente, "fuse_write - bufferSize: %i\n", bufferSize);

	//Receive message size
	int receivedBytes = receiveMessage(&socketPokeServer, &ultimoPunteroDeLosBloques_write ,sizeof(ultimoPunteroDeLosBloques_write));
	log_info(logPokeCliente, "fuse_write -RECEIVE - ultimoPunteroDeLosBloques_write2: %d\n", ultimoPunteroDeLosBloques_write);

	if(ultimoPunteroDeLosBloques_write != -1 ){
		memset(buf,0,size);

		time_t tiempo2 = time(0);
		double segsSinResponder = difftime(tiempo2, tiempo1);
		printf("\n------------------> Tiempo WRITE %f  | Size: %i | offset %i \n",segsSinResponder,size,offset);
		return size;
	}else{
		return -1;// EL DISCO ESTA LLENO
	}
}

static int fuse_rename (const char *oldname, const char *newName){
	printf("********************************* fuse_rename *********************\n");
	char *file_name = strrchr (newName, '/') + 1;
	printf("fuse_rename - path: %s\n", file_name);

	if (string_length(file_name)>17){
		printf("RENAME - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		log_info(logPokeCliente, "RENAME - EL RENAME ES MAYOR A 17: %i\n", string_length(file_name));
		return -1;
	}

	int resultado = renombrarArchivo(oldname, newName);

	if (resultado == -999)	{
			printf("[Error_Fuse] rename(%s,%s)\n", oldname, newName);
			return 1;
		}

	return 0;

}
static int fuse_utimens(const char *path, const struct timespec ts[2]){
	printf("***************** fuse_utimens - ts[1].tv_nsec: %i***********************\n", ts[1].tv_nsec);
	//			NOTA VEAMOS JUNTOS COMO IMPLEMENTARLO LO QUE HAY QUE OBTENER ES EL
	//			uint32_t lastmod;

	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_UTIMENS;
	int exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

	string_append(&path, "\0");
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_utimens - pathLength: %i\n", pathLength);

	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "fuse_utimens - path: %s\n", path);

	//3) send time
	exitCode = sendMessage(&socketPokeServer, &ts[1].tv_nsec , sizeof(long int));//del otro lado se esta recibiendo un long int
	log_info(logPokeCliente, "fuse_utimens - time: %i\n", ts[1].tv_nsec);

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "fuse_utimens - MessageSize: %i\n", messageSize);

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
		for (i = logPosition; i <= argc - 2 ; i++) {
			//has to be deleted the parameter and its value
			argv[i] = argv[i + 2];
		}
		argc = argc - 2 ;
	}

	logPokeCliente = log_create(logFile, "POKEDEX_CLIENTE", 0, LOG_LEVEL_TRACE);

	pthread_mutex_init(&ReadMutex, NULL);

	//getting environment variable for connecting to server
	IP_SERVER = getenv("POKEIP");
	PORT = atoi(getenv("POKEPORT"));
	log_info(logPokeCliente,"IP_SERVER: %s\n",IP_SERVER);
	log_info(logPokeCliente,"PORT: %d\n",PORT);
	exitCode = connectTo(POKEDEX_SERVIDOR, &socketPokeServer);

	if (exitCode == EXIT_SUCCESS) {
		log_info(logPokeCliente, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

		log_info(logPokeCliente,"argc: %i\n", argc);
		for (i = 0; i < argc; i++) {
			log_info(logPokeCliente,"argv[%d]: %s\n",i, argv[i]);
		}

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

t_list * obtenerDirectorio(const char* path, enum_FUSEOperations fuseOperation){
	enum_FUSEOperations fuseOperation2;


	if (fuseOperation == FUSE_GETATTR){
		fuseOperation2 = FUSE_GETATTR;
	}else
	{
		fuseOperation2 = FUSE_READDIR;
	}
	//0) Send Fuse Operations
	sendMessage(&socketPokeServer, &fuseOperation2 , sizeof(fuseOperation2));
	log_info(logPokeCliente, "fuseOperation2: %d", fuseOperation2);

	string_append(&path, "\0");
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "pathLength: %i\n", pathLength);

	//2) send path
	sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "path: %s\n", path);

	//Receive element Count
	int messageSize = 0;
	receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "messageSize: %d", messageSize);

	char *messageRcv = malloc(messageSize);
	receiveMessage(&socketPokeServer, messageRcv ,messageSize);
	t_list *listaBloques = deserializeListaBloques(messageRcv);

	log_info(logPokeCliente, "element count: %d\n", listaBloques->elements_count);

	free(messageRcv);
	return listaBloques;

}

