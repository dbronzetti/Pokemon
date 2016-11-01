/*
 ============================================================================
 Name        : PokeDex_Cliente.c
 ============================================================================
 */

#include "PokeDex_Cliente.h"

int  borrarArchivo (const char* path){
		int exitCode = EXIT_FAILURE; //DEFAULT Failure
		int resultado = 1;
		t_list *listaBloques = list_create();
		enum_FUSEOperations fuseOperation =  FUSE_UNLINK;

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

int directorioBorrar(char *path){
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
			int resultado = 1;
			t_list *listaBloques = list_create();
			enum_FUSEOperations fuseOperation =  FUSE_UNLINK;

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
			t_list *listaBloques = list_create();
			enum_FUSEOperations fuseOperation =  FUSE_RENAME;

			//0) Send Fuse Operations
			exitCode = sendMessage(&socketPokeServer, &fuseOperation , sizeof(fuseOperation));
			log_info(logPokeCliente, "fuseOperation: %d\n", fuseOperation);
			string_append(&oldname, "\0");

			//1) send path length (+1 due to \0)
			int pathLength = strlen(oldname) + 1;
			exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));

			log_info(logPokeCliente, "pathLength: %i\n", pathLength);

			//2) send path ORIGINAL
			exitCode = sendMessage(&socketPokeServer, oldname , strlen(oldname) + 1 );
			log_info(logPokeCliente, "path: %s\n", oldname);

			//2.1) send path RENOMBRADO
			exitCode = sendMessage(&socketPokeServer, newName , strlen(newName) + 1 );
			log_info(logPokeCliente, "path: %s\n", newName);

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

static void *fuse_init(void)
{
  printf("[Fuse] init()\n");
  // truco para permitir el montaje como una superposición
  fchdir(save_dir);
  close(save_dir);
  return NULL;
}

static int fuse_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	//path = full(path);
	//printf("[Fuse] getattr(%s)\n", path);
	memset(stbuf, 0, sizeof(struct stat));
	log_info(logPokeCliente, "****************fuse_getattr****************\n");
	if (strcmp(path, DIRECTORIO_RAIZ) == 0){
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	}else {

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
			}
			else if (nodo->state == DIRECTORY)
			{
				log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->fname: %s\n", nodo->fname);
				log_info(logPokeCliente, "FUSE_GETATTR - DIRECTORY - nodo->state: DIRECTORY\n");
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_nlink = 2;
			}

		}
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

	int resultado = directorioBorrar(path);

	if (resultado!=0)	{
		printf("[Error_Fuse] rmdir(%s)\n", path);
		return 1;
	}

	return resultado;
}

static int fuse_create (const char* path, mode_t mode, struct fuse_file_info * fi){
	int exitCode = -1; //DEFAULT Failure
	log_info(logPokeCliente, "****************fuse_create****************\n");
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

	//Receive message Status
	int receivedBytes = receiveMessage(&socketPokeServer, &exitCode ,sizeof(exitCode));
	log_info(logPokeCliente, "fuse_create - MessageStatus: %i\n", exitCode);

	return exitCode;
}


static int fuse_mkdir(const char* path, mode_t mode){

	//int resultado = crearDirectorio(path);
	printf("********************************* fuse_mkdir *********************\n");
	//if (resultado!=0)	{
		//printf("[Error_Fuse] mkdir(%s)\n", path);
		//return 1;
	//}
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
	log_info(logPokeCliente, "fuse_read - pathLength: %i\n", pathLength);
	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );
	log_info(logPokeCliente, "fuse_read - path: %s\n", path);

	//Receive message size
	int elementCount = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &elementCount ,sizeof(elementCount));
	log_info(logPokeCliente, "fuse_read - elementCount: %i\n", elementCount);

	if (receivedBytes > 0){
		}

	return 0;


}

static int fuse_unlink(const char* path)
{
	int resultado = borrarArchivo (path);
	printf("********************************* fuse_unlink *********************\n");
	if (resultado!=0)	{
			printf("[Error_Fuse] unlink(%s)\n", path);
			return 1;
		}

		return resultado;
}

static int fuse_open(const char *path, struct fuse_file_info *fi) {
	printf("********************************* fuse_open *********************\n");
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
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

	//Receive message size
	int messageSize = -1;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));
	log_info(logPokeCliente, "fuse_read - MessageSize: %i\n", messageSize);

	if (receivedBytes > 0){
		char *messageRcv = malloc(messageSize);
		receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
		log_info(logPokeCliente, "messageRcv: %s\n", messageRcv);
		memcpy(buf, messageRcv, messageSize);
		return messageSize;
	}

	return 0;
}

static int fuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	long int bytes_escritos = 0;
	printf("********************************* fuse_write *********************\n");
	//bytes_escritos = enviarArchivo(path,offset);

	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	//0) Send Fuse Operations
	enum_FUSEOperations operacion = FUSE_WRITE;

	exitCode = sendMessage(&socketPokeServer, &operacion , sizeof(enum_FUSEOperations));

	string_append(&path, "\0");
	string_append(&buf, "\0");
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	log_info(logPokeCliente, "fuse_write - pathLength: %i\n", pathLength);
	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , pathLength );
	log_info(logPokeCliente, "fuse_write - path: %s\n", path);
	//3) send buffer length (+1 due to \0)
	int bufferSize = strlen(buf) + 1;
	exitCode = sendMessage(&socketPokeServer, &size , sizeof(int));
	log_info(logPokeCliente, "fuse_write - bufferSize: %i\n", size);
	//4) send path
	exitCode = sendMessage(&socketPokeServer, buf , bufferSize );
	log_info(logPokeCliente, "fuse_write - path: %s\n", buf);

	//Receive message size
	int receivedBytes = receiveMessage(&socketPokeServer, &bytes_escritos ,sizeof(bytes_escritos));
	log_info(logPokeCliente, "fuse_write - bytes_escritos: %i\n", bytes_escritos);

	return bytes_escritos;
}

static int fuse_rename (const char *oldname, const char *newName){
	int resultado = renombrarArchivo(oldname,newName);
	printf("********************************* fuse_rename *********************\n");
	if (resultado!=0)	{
			printf("[Error_Fuse] rename(%s,%s)\n", oldname,newName);
			return 1;
		}

	return resultado;

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
	//.mknod		= fuse_mknod,

	#ifdef HAVE_SETXATTR
		.setxattr	= fuse_setxattr,
		.getxattr	= fuse_getxattr,
		.listxattr	= fuse_listxattr,
		.removexattr= fuse_removexattr,
	#endif
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

t_list * obtenerDirectorio(const char* path, enum_FUSEOperations fuseOperation){
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	t_list *listaBloques = list_create();

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
		log_info(logPokeCliente, "messageRcv: %s\n", messageRcv);
		deserializeListaBloques(listaBloques,messageRcv,elementCount);
	}
	log_info(logPokeCliente, "*********************************\n");
	return listaBloques;

}

