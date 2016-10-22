/*
 ============================================================================
 Name        : PokeDex_Cliente.c
 ============================================================================
 */

#include "PokeDex_Cliente.h"

const char *full(const char *path) /* Anade un punto de montaje */;//Esto da un warning no importa!
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

t_list* obtenerTablaDeArchivos(char* path){
	t_list* resultado;

	return resultado;
}

static int fuse_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	path = full(path);
	printf("[Fuse] getattr(%s)\n", path);
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, DIRECTORIO_RAIZ) == 0)
	{
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else 	{

		//@TODO: Esta funcion llama al socket y pide el bloque
		osada_file* nodo = obtenerTablaDeArchivos(path);
		if (nodo != NULL)
		{
			if (nodo->state == 1)
			{
				stbuf->st_mode = S_IFREG | 0777;
				stbuf->st_nlink = 1;
				stbuf->st_size = nodo->file_size;
			}
			else if (nodo->state == 2)
			{
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_nlink = 1;
			}
		} else {
			res = -ENOENT;
		}
	}

	return res;
}

static int fuse_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;

	int i;
	t_list* nodos = obtenerDirectorio(path);

	if(nodos!=NULL){
		for (i = 0; i < nodos->elements_count; i++) 		{
			osada_file *nodo = list_get(nodos,i);
			if ((nodo->state != 0)){
				filler(buffer, nodo->fname, NULL, 0);
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

static int fuse_mkdir(const char* path){

	int resultado = crearDirectorio(path);

	if (resultado!=0)	{
		printf("[Error_Fuse] mkdir(%s)\n", path);
		return 1;
	}

	return resultado;
}

static int fuse_unlink(const char* path)
{
	int resultado = borrarArchivo (path);

	if (resultado!=0)	{
			printf("[Error_Fuse] unlink(%s)\n", path);
			return 1;
		}

		return resultado;
}

static int fuse_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	osada_file nodos = obtener_bloque_archivo(path);

	//@TODO: ver implementacion.
	return 0;
}

static int fuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	long int bytes_escritos = 0;
	bytes_escritos = enviarArchivo(path,offset);
	return bytes_escritos;
}

static int fuse_rename (const char *oldname, const char *newName){
	int resultado = renombrarArchivo(oldname,newName);
	if (resultado!=0)	{
			printf("[Error_Fuse] rename(%s,%s)\n", oldname,newName);
			return 1;
		}

	return resultado;

}

static struct fuse_operations xmp_oper = {
    .init       = fuse_init,
    .getattr	= fuse_getattr,
    .rmdir		= fuse_rmdir,
    .readdir	= fuse_readdir,
    .unlink		= fuse_unlink,
    .open		= fuse_open,
    .mkdir		= fuse_mkdir,
    .rename		= fuse_rename,
    .write		= fuse_write,
    .read		= fuse_read,
//    .access		= fuse_access,
//    .readlink	= fuse_readlink,
//    .mknod		= fuse_mknod,
//    .link		= fuse_link,
//    .chmod		= fuse_chmod,
//    .chown		= fuse_chown,
//    .statfs		= fuse_statfs,

	#ifdef HAVE_SETXATTR
		.setxattr	= fuse_setxattr,
		.getxattr	= fuse_getxattr,
		.listxattr	= fuse_listxattr,
		.removexattr= fuse_removexattr,
	#endif
};



int main(int argc, char **argv) {
	char *logFile = NULL;
	char *disco = string_new();
	int exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	for (i = 0; i < argc; i++) {

		//check disk file parameter
		if (strcmp(argv[i], "-d") == 0) {
			disco = argv[i + 1];
			printf("El disco a trabajar: '%s'\n", disco);
		}

		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	logPokeCliente = log_create(logFile, "POKEDEX_CLIENTE", 0, LOG_LEVEL_TRACE);

	//getting environment variable for connecting to server
	IP_SERVER = getenv("POKEIP");
	PORT = atoi(getenv("POKEPORT"));

	exitCode = connectTo(POKEDEX_SERVIDOR, &socketPokeServer);
	if (exitCode == EXIT_SUCCESS) {
		log_info(logPokeCliente, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

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
	exitcode = openClientConnection(ip, port, socketClient);

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

t_list * obtenerDirectorio(char* path){
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	t_list *listaBloques = list_create();

	string_append(&path, "\0");
	//1) send path length (+1 due to \0)
	int pathLength = strlen(path) + 1;
	exitCode = sendMessage(&socketPokeServer, &pathLength , sizeof(int));
	//2) send path
	exitCode = sendMessage(&socketPokeServer, path , strlen(path) + 1 );

	//Receive message size
	int messageSize = 0;
	int receivedBytes = receiveMessage(&socketPokeServer, &messageSize ,sizeof(messageSize));

	if (receivedBytes > 0){
		char *messageRcv = malloc(sizeof(messageSize));
		receivedBytes = receiveMessage(&socketPokeServer, messageRcv ,messageSize);
		deserializeListaBloques(listaBloques,messageRcv);
	}

	return listaBloques;

}

