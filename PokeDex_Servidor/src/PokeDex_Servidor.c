/*
 ============================================================================
 Name        : PokeDex_Servidor.c
 ============================================================================
 */

#include "PokeDex_Servidor.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *diskFile = NULL;
	pthread_t serverThread;
	pthread_mutex_init(&mutexG, NULL);
	initMutexOsada();

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameters
	int i;
	for (i = 0; i < argc; i++) {

		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}

		if (strcmp(argv[i], "-d") == 0) {
			diskFile = argv[i + 1];
			printf("disk File: '%s'\n", diskFile);
		}
	}

	int archivoID = obtenerIDDelArchivo(diskFile);
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	inicializarOSADA(archivoID);
	obtenerHeader();

	setearConstantesDePosicionDeOsada();

	obtenerBitmap();
    obtenerTablaDeArchivos();
    obtenerTablaDeAsignacion();

	logPokeDexServer = log_create(logFile, "POKEDEX_SERVER", 0, LOG_LEVEL_TRACE);

	puts("Soy el PokeDex Servidor"); /* prints Soy el PokeDex Servidor */

	//getting environment variable for listening
	PORT = atoi(getenv("POKEPORT"));

	pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);

	pthread_join(serverThread, NULL);

	pthread_mutex_destroy(&mutexG);
	destroyMutexOsada();

	return EXIT_SUCCESS;

}

void startServerProg() {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	int socketServer;

	exitCode = openServerConnection(PORT, &socketServer);
	log_info(logPokeDexServer, "SocketServer: %d", socketServer);

	//If exitCode == 0 the server connection is opened and listening
	if (exitCode == 0) {
		log_info(logPokeDexServer, "the server is opened");

		exitCode = listen(socketServer, SOMAXCONN);//SOMAXCONN = Constant with the maximum connections allowed by the SO

		if (exitCode < 0) {
			log_error(logPokeDexServer, "Failed to listen server Port.");
			return;
		}

		while (1) {
			newClients((void*) &socketServer);
		}
	}

}

void newClients(void *parameter) {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure

	t_serverData *serverData = malloc(sizeof(t_serverData));
	memcpy(&serverData->socketServer, parameter, sizeof(serverData->socketServer));

	// disparar un thread para acceptar cada cliente nuevo (debido a que el accept es bloqueante) y para hacer el handshake
	/**************************************/
	//Create thread attribute detached
	//			pthread_attr_t acceptClientThreadAttr;
	//			pthread_attr_init(&acceptClientThreadAttr);
	//			pthread_attr_setdetachstate(&acceptClientThreadAttr, PTHREAD_CREATE_DETACHED);
	//
	//			//Create thread for checking new connections in server socket
	//			pthread_t acceptClientThread;
	//			pthread_create(&acceptClientThread, &acceptClientThreadAttr, (void*) acceptClientConnection1, &serverData);
	//
	//			//Destroy thread attribute
	//			pthread_attr_destroy(&acceptClientThreadAttr);
	/************************************/

	exitCode = acceptClientConnection(&serverData->socketServer,&serverData->socketClient);

	if (exitCode == EXIT_FAILURE) {
		log_warning(logPokeDexServer,"There was detected an attempt of wrong connection");
		close(serverData->socketClient);
		free(serverData);
	} else {

		log_info(logPokeDexServer, "The was received a connection in socket: %d.",serverData->socketClient);
		//Create thread attribute detached
		pthread_attr_t handShakeThreadAttr;
		pthread_attr_init(&handShakeThreadAttr);
		pthread_attr_setdetachstate(&handShakeThreadAttr,PTHREAD_CREATE_DETACHED);

		//Create thread for checking new connections in server socket
		pthread_t handShakeThread;
		pthread_create(&handShakeThread, &handShakeThreadAttr,(void*) handShake, serverData);

		//Destroy thread attribute
		pthread_attr_destroy(&handShakeThreadAttr);

	}		// END handshakes

}

void handShake(void *parameter) {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure

	t_serverData *serverData = (t_serverData*) parameter;

	//Receive message size
	int messageSize = 0;
	char *messageRcv = malloc(sizeof(messageSize));
	int receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,sizeof(messageSize));

	//Receive message using the size read before
	memcpy(&messageSize, messageRcv, sizeof(int));
	messageRcv = realloc(messageRcv, messageSize);
	receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,messageSize);

	//starting handshake with client connected
	t_MessageGenericHandshake *message = malloc(sizeof(t_MessageGenericHandshake));
	deserializeHandShake(message, messageRcv);

	//Now it's checked that the client is not down
	if (receivedBytes == 0) {
		log_error(logPokeDexServer,	"The client went down while handshaking! - Please check the client '%d' is down!",serverData->socketClient);
		close(serverData->socketClient);
		free(serverData);
	} else {
		switch ((int) message->process) {
		case POKEDEX_CLIENTE: {
			log_info(logPokeDexServer, "Message from '%s': %s", getProcessString(message->process), message->message);
			log_info(logPokeDexServer, "Ha ingresado un nuevo Poke Cliente");

			exitCode = sendClientAcceptation(&serverData->socketClient);

			if (exitCode == EXIT_SUCCESS) {

				//Create thread attribute detached
				pthread_attr_t processMessageThreadAttr;
				pthread_attr_init(&processMessageThreadAttr);
				pthread_attr_setdetachstate(&processMessageThreadAttr, PTHREAD_CREATE_DETACHED);

				//Create thread for checking new connections in server socket
				pthread_t processMessageThread;
				pthread_create(&processMessageThread, &processMessageThreadAttr, (void*) processMessageReceived, parameter);

				//Destroy thread attribute
				pthread_attr_destroy(&processMessageThreadAttr);
			}

			break;
		}
		default: {
			log_error(logPokeDexServer,	"Process not allowed to connect - Invalid process '%s' tried to connect to MAPA", getProcessString(message->process));
			close(serverData->socketClient);
			free(serverData);
			break;
		}
		}
	}

	free(message->message);
	free(message);
	free(messageRcv);
}

void processMessageReceived(void *parameter){
	t_serverData *serverData = (t_serverData*) parameter;

	t_list* lista = list_create();
	enum_FUSEOperations *FUSEOperation = malloc(sizeof(enum_FUSEOperations));
	bool exitLoop = false;

	while(1){
		//0) Receive FUSE Operation
		int receivedBytes = receiveMessage(&serverData->socketClient, FUSEOperation, sizeof(enum_FUSEOperations));

		//pthread_mutex_lock(&mutexG);
		if ( receivedBytes > 0 ){

			log_info(logPokeDexServer, "Processing POKEDEX_CLIENTE message received,  FUSEOperation: %i",*FUSEOperation);
			printf("Processing POKEDEX_CLIENTE message received,  FUSEOperation: %i\n",*FUSEOperation);

			switch (*FUSEOperation){
				case FUSE_UTIMENS:{
					log_info(logPokeDexServer, "-------Processing FUSE_UTIMENS message");
					int parent_directory=0;
					int pathLength = 0;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "FUSE_UTIMENS - Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "FUSE_UTIMENS - Message path received : %s\n",path);

					//3) Receive parent_directory
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "FUSE_UTIMENS - Message parent_directory received : %i\n",parent_directory);

					//4) time
					long int tiempo=0;
					receiveMessage(&serverData->socketClient, &tiempo, sizeof(int));
					log_info(logPokeDexServer, "FUSE_UTIMENS - Message tiempo received : %i\n",tiempo);

					ingresarElUTIMENS(path, parent_directory, tiempo);

					sendMessage(&serverData->socketClient, &tiempo , sizeof(tiempo));
					break;
				}
				case FUSE_RMDIR:{
					log_info(logPokeDexServer, "Processing FUSE_RMDIR message");
					printf("************************ Processing FUSE_RMDIR message ********************************\n");
					int parent_directory=0;
					int pathLength = 0;
					int posTablaDeArchivos=0;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);

					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					//3) Receive parent_directory
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					posTablaDeArchivos = borrarUnDirectorio(path, parent_directory);
					log_info(logPokeDexServer, "Message posTablaDeArchivosreceived : %i\n",posTablaDeArchivos);

					sendMessage(&serverData->socketClient, &posTablaDeArchivos , sizeof(int));
					break;
				}
				case FUSE_WRITE:{
						log_info(logPokeDexServer, "Processing FUSE_WRITE message");
						printf("************************ Processing FUSE_WRITE message ********************************\n");
						int posDelaTablaDeArchivos = -999;
						int pathLength = 0;
						uint16_t parent_directory;
						int ultimoPunteroDeLosBloques = 1;

						//1) Receive path length
						receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
						log_info(logPokeDexServer, "FUSE_WRITE - pathLength'%d': %d", serverData->socketClient, pathLength);
						char *path = malloc(pathLength);
						//2) Receive path
						receiveMessage(&serverData->socketClient, path, pathLength);
						log_info(logPokeDexServer, "FUSE_WRITE - path: %s\n",path);
						//3) Content size
						int contentSize = 0;
						receiveMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
						log_info(logPokeDexServer, "FUSE_WRITE - Content size: %d", contentSize);
						char *content = malloc(contentSize);
						//4) Content path
						receiveMessage(&serverData->socketClient, content, contentSize);
						log_info(logPokeDexServer, "FUSE_WRITE - Message content received : %s\n",content);

						//5) posDelaTablaDeArchivos
						receiveMessage(&serverData->socketClient, &posDelaTablaDeArchivos, sizeof(posDelaTablaDeArchivos));
						log_info(logPokeDexServer, "FUSE_WRITE - Message posDelaTablaDeArchivos received : %i\n",posDelaTablaDeArchivos);

						//6) Receive parent_directory
						//log_info(logPokeDexServer, "Message parent_directory received --> \n");
					     receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
						log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

						int ultimoPuntero = crearUnArchivo(content, contentSize, path, posDelaTablaDeArchivos, parent_directory);
						log_info(logPokeDexServer, "FUSE_WRITE - ultimoPuntero: %d\n", ultimoPuntero);
						printf("FUSE_WRITE - ultimoPunteroDeLosBloques: %d\n", ultimoPunteroDeLosBloques);

						sendMessage(&serverData->socketClient, &ultimoPuntero, sizeof(int));

						log_info(logPokeDexServer, "FUSE_WRITE - FIN sendMessage");
						printf("********************************* TERMINO EL WRITE *********************\n");
						free(content);
						free(path);


						break;
				}
				case FUSE_CREATE:{
					log_info(logPokeDexServer, "Processing FUSE_CREATE message");
					int pathLength = 0;
					int posDelaTablaDeArchivos = -999;
					int first_block_init = -999;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message received : %s\n",path);

					//get padre from path received for passing it to escribirEnLaTablaDeArchivos
					int posBloquePadre = obtener_bloque_padre(path);

					log_info(logPokeDexServer, "FUSE_CREATE - escribirEnLaTablaDeArchivos");
					posDelaTablaDeArchivos = escribirEnLaTablaDeArchivos(posBloquePadre, 0, path, first_block_init, posDelaTablaDeArchivos);

					log_info(logPokeDexServer, "FUSE_CREATE - posDelaTablaDeArchivos a enviar %d", posDelaTablaDeArchivos);

					sendMessage(&serverData->socketClient, &posDelaTablaDeArchivos, sizeof(int));
					log_info(logPokeDexServer, "FUSE_CREATE - TERMINO");
					break;
				}
				case FUSE_UNLINK:{
					log_info(logPokeDexServer, "-------Processing FUSE_UNLINK message");
					int parent_directory=0;
					int pathLength = 0;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message path received : %s\n",path);

					//3) Receive parent_directory

					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					osada_block_pointer posicion = devolverOsadaBlockPointer(path, parent_directory);
					printf("FUSE_UNLINK - posicion: %i\n",posicion);
					if (posicion != -999){
						t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
						borrarBloquesDelBitmap(conjuntoDeBloquesDelArchivo);
					}
					borrarUnArchivo(path, parent_directory);

					sendMessage(&serverData->socketClient, &posicion , sizeof(posicion));

					break;
				}
				case FUSE_TRUNCATE:{
					    log_info(logPokeDexServer, "-------Processing FUSE_TRUNCATE message");
						printf("******************* Processing FUSE_TRUNCATE message ****************\n");
						int posDelaTablaDeArchivos = -999;
						int pathLength = 0;
						uint16_t parent_directory;
						int ultimoPunteroDeLosBloques = 1;

						//1) Receive path length
						receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
						log_info(logPokeDexServer, "FUSE_TRUNCATE - pathLength : %d", pathLength);
						char *path = malloc(pathLength);

						//2) Receive path
						receiveMessage(&serverData->socketClient, path, pathLength);
						log_info(logPokeDexServer, "FUSE_TRUNCATE - path: %s\n",path);

						//3) Content size
						int contentSize = 0;
						receiveMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
						log_info(logPokeDexServer, "FUSE_TRUNCATE - Content size: %d", contentSize);
						char *content = malloc(contentSize);

						//4) Content
						receiveMessage(&serverData->socketClient, content, contentSize);
						log_info(logPokeDexServer, "FUSE_TRUNCATE - Message content received : %s\n",content);

						//5) posDelaTablaDeArchivos
						receiveMessage(&serverData->socketClient, &posDelaTablaDeArchivos, sizeof(posDelaTablaDeArchivos));
						log_info(logPokeDexServer, "FUSE_TRUNCATE - Message posDelaTablaDeArchivos received : %i\n",posDelaTablaDeArchivos);

						//6) Receive parent_directory
					     receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
						log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

						int ultimoPuntero = hacerElTruncate(content, contentSize, path, posDelaTablaDeArchivos, parent_directory);
						log_info(logPokeDexServer, "FUSE_TRUNCATE - ultimoPuntero: %d\n", ultimoPuntero);
						printf("FUSE_TRUNCATE - ultimoPunteroDeLosBloques: %d\n", ultimoPunteroDeLosBloques);

						sendMessage(&serverData->socketClient, &ultimoPuntero, sizeof(int));
						log_info(logPokeDexServer, "-------FIN FUSE_TRUNCATE message");
						printf("******************* Processing FUSE_TRUNCATE message ****************\n");

						break;

				}
				case FUSE_MKDIR:{
					log_info(logPokeDexServer, "Processing FUSE_MKDIR message");
					int parent_directory=0;
					int pathLength = 0;
					int posTablaDeArchivos=0;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);

					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					//3) Receive parent_directory
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					posTablaDeArchivos = crearUnDirectorio(path, parent_directory);
					log_info(logPokeDexServer, "Message posTablaDeArchivosreceived : %i\n",posTablaDeArchivos);

					sendMessage(&serverData->socketClient, &posTablaDeArchivos , sizeof(int));
					break;
				}
				case FUSE_MODIFICAR:{
					log_info(logPokeDexServer, "Processing FUSE_MODIFICAR message");
					int posDelaTablaDeArchivos = -999;
					int pathLength = 0;
					uint16_t parent_directory;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "FUSE_MODIFICAR - Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "FUSE_MODIFICAR - Message path received : %s\n",path);
					//3) Content size
					int contentSize = 0;
					receiveMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
					log_info(logPokeDexServer, "FUSE_MODIFICAR - Content size: %d", contentSize);
					char *content = malloc(contentSize);
					//4) Content path
					receiveMessage(&serverData->socketClient, content, contentSize);
					log_info(logPokeDexServer, "FUSE_MODIFICAR - Message content received : %s\n",content);

					//6) Receive parent_directory
					log_info(logPokeDexServer, "Message parent_directory received --> \n");
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					modificarUnArchivo(content, contentSize, path, parent_directory);
					log_info(logPokeDexServer, "FUSE_MODIFICAR - TERMINO DE CREAR\n");

					sendMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));

					log_info(logPokeDexServer, "FUSE_MODIFICAR - FIN sendMessage");
					printf("********************************* TERMINO EL FUSE_MODIFICAR *********************\n");
					free(content);
					free(path);
					break;
				}
				case FUSE_OPEN:{
					log_info(logPokeDexServer, "-------Processing FUSE_OPEN message");
					printf("******************* Processing FUSE_OPEN message ****************\n");
					int parent_directory=0;
					int pathLength = 0;
					osada_file osadaFile;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message path received : %s\n",path);

					//3) Receive parent_directory
					log_info(logPokeDexServer, "Message parent_directory received --> \n");
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					osadaFile = buscarElArchivoYDevolverOsadaFile(path, parent_directory);
					sendMessage(&serverData->socketClient, &osadaFile.file_size , sizeof(int));

					log_info(logPokeDexServer, "-------FIN FUSE_OPEN message");
					printf("******************* Processing FUSE_OPEN message ****************\n");

					break;
				}
				case FUSE_READ:{

					printf("******************* Processing FUSE_READ message ****************\n");
					log_info(logPokeDexServer, "Processing FUSE_READ message");
					int parent_directory=0;
					int pathLength = 0;

					//1) Receive path length
					printf("******************* FUSE_READ - Receive path length ****************\n");
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "FUSE_READ - path Length received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);

					//2) Receive path
					printf("******************* FUSE_READ - Receive path ****************\n");
					receiveMessage(&serverData->socketClient, path, pathLength);
					if (path != NULL){
						log_info(logPokeDexServer, "FUSE_READ - Message path received : %s\n",path);

						//3) Receive parent_directory
						printf("******************* FUSE_READ - Receive parent_directory ****************\n");
						receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
						log_info(logPokeDexServer, "FUSE_READ - Message parent_directory received : %i\n",parent_directory);

						osada_block_pointer posicion = devolverOsadaBlockPointer(path, parent_directory);

						printf("FUSE_READ - posicion: %i\n",posicion);
						//char *string = string_new();
						if (posicion != -999){// -999 = NO LO ENCONTRO
							t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
							printf("FUSE_READ - conjuntoDeBloquesDelArchivo first: %i\n",list_get(conjuntoDeBloquesDelArchivo, 0));
							printf("FUSE_READ - conjuntoDeBloquesDelArchivo last: %i\n",list_get(conjuntoDeBloquesDelArchivo, conjuntoDeBloquesDelArchivo->elements_count-1));
							int i;
							printf("FUSE_READ - conjuntoDeBloquesDelArchivo: %i\n",conjuntoDeBloquesDelArchivo->elements_count);
							log_info(logPokeDexServer, "FUSE_READ - conjuntoDeBloquesDelArchivo: %i\n",conjuntoDeBloquesDelArchivo->elements_count);
							//memcpy(string, &conjuntoDeBloquesDelArchivo->elements_count, sizeof(int));
							//printf("string: %s\n",string);
							char *bloqueDeDatos = malloc(OSADA_BLOCK_SIZE);
							int cantidadBloques = conjuntoDeBloquesDelArchivo->elements_count;
							sendMessage(&serverData->socketClient, &cantidadBloques , sizeof(cantidadBloques));

							for (i = 0; i < conjuntoDeBloquesDelArchivo->elements_count; i++) {

								int bloque2 = list_get(conjuntoDeBloquesDelArchivo, i);
								bloque2 *= 64;

								pthread_mutex_lock(&OSADAmutex);
								memcpy(bloqueDeDatos, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );
								pthread_mutex_unlock(&OSADAmutex);
								//printf("bloqueDeDatos: %s\n",bloqueDeDatos);
								//printf("bloqueDeDatos[0]: %c\n",bloqueDeDatos[0]);
								log_info(logPokeDexServer, "bloqueDeDatos: %s\n", bloqueDeDatos);

								bloqueDeDatos[OSADA_BLOCK_SIZE] = '\0';
								int messageSize = strlen(bloqueDeDatos);
								log_info(logPokeDexServer, "messageSize: %i\n", messageSize);
								printf("read - servidor - conjuntoDeBloquesDelArchivo->elements_count: %i\n", i);

								if (strcmp(bloqueDeDatos ,string_repeat('\0', OSADA_BLOCK_SIZE)) == 0){
									printf("es un bloque barra cero de 64\n");
									int osadaBlockSize = OSADA_BLOCK_SIZE;
									sendMessage(&serverData->socketClient, &osadaBlockSize , sizeof(int));
									sendMessage(&serverData->socketClient, string_repeat('\0', OSADA_BLOCK_SIZE) , osadaBlockSize);
								}
									else
								{
									sendMessage(&serverData->socketClient, &messageSize , sizeof(messageSize));
									sendMessage(&serverData->socketClient, bloqueDeDatos , messageSize);
								}

								//string_append(&string, bloqueDeDatos);

							}

							free(bloqueDeDatos);
							//TODO: CUANDO HAGO EL APPEND EL PRIMER ELEMENTO DEL STRING CONTIENE UNA BASURA
							//printf("******************* string to be sent for file ****************\n");
							//string_append(&string, "\0");
							//log_info(logPokeDexServer, "string to be sent for file '%s': %s\n", path, string);


							//int messageSize = strlen(string) +1; //+1 due to /0
							//log_info(logPokeDexServer, "messageSize: %i\n", messageSize);
							//sendMessage(&serverData->socketClient, &messageSize , sizeof(messageSize));
							//printf("2 - string: %s\n",string);
							//sendMessage(&serverData->socketClient, string , messageSize);
						}else{
							log_info(logPokeDexServer, "file %s not found",path);
							int error = posicion;
							sendMessage(&serverData->socketClient, &error , sizeof(error));//si no encontro el file retorno -999 como error al fuse
						}

						printf("******************* termino ****************\n");

					}
					break;
				}
				case FUSE_READDIR:{
					log_info(logPokeDexServer, "Processing FUSE_READDIR message");
					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "FUSE_READDIR - Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "FUSE_READDIR - Message size received : %s\n",path);

					//get padre from path received for passing it to crearArbolAPartirDelPadre
					int posBloquePadre = obtener_bloque_padre(path);
					log_info(logPokeDexServer,"FUSE_READDIR - posBloquePadre: %i\n", posBloquePadre);
					printf("FUSE_READDIR - posBloquePadre: %i\n", posBloquePadre);

					lista = crearArbolAPartirDelPadre(posBloquePadre);

					log_info(logPokeDexServer,"FUSE_READDIR - lista->elements_count: %i\n",lista->elements_count);
					printf("FUSE_READDIR - lista->elements_count: %i\n",lista->elements_count);

					int messageSize = 0;
					char *mensajeOsada = serializeListaBloques(lista, &messageSize);
					sendMessage(&serverData->socketClient, "hola\0" ,strlen("hola\0")+1);
					sendMessage(&serverData->socketClient, mensajeOsada , messageSize);

					break;
				}
				case FUSE_GETATTR:{
					log_info(logPokeDexServer, "Processing FUSE_GETATTR message");
					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "FUSE_GETATTR - Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "FUSE_GETATTR - Message size received : %s\n",path);

					int posArchivo = obtener_bloque_archivo(path);

					int elementCount;
					char *mensajeOsada = malloc(sizeof(elementCount));
					int messageSize = sizeof(elementCount);

					if (posArchivo != -666){
						osada_file bloqueArchivo = TABLA_DE_ARCHIVOS[posArchivo];

						printf("FUSE_GETATTR - Paso el buscarArchivo: \n");
						printf("FUSE_GETATTR - File Name: %s\n", bloqueArchivo.fname);
						elementCount = 1;
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain only 1 OSADA_FILE
						log_info(logPokeDexServer, "FUSE_GETATTR - !=666 elementCount: %i\n",elementCount);
						//TODO: PARA ANALIZAR
						mensajeOsada = serializeBloque(&bloqueArchivo, mensajeOsada, &messageSize);

					}else{
						elementCount = 0;
						log_info(logPokeDexServer, "FUSE_GETATTR - ==-666 - elementCount: %i\n",elementCount);
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain 0 OSADA_FILE because the file was not found
					}
					log_info(logPokeDexServer, "FUSE_GETATTR - messageSize: %i\n",messageSize);
					sendMessage(&serverData->socketClient, "hola\0" ,strlen("hola\0")+1);

					sendMessage(&serverData->socketClient, mensajeOsada , messageSize);

					log_info(logPokeDexServer, "HIZO SEND\n");

					break;
				}
				case FUSE_RENAME:{
					log_info(logPokeDexServer, "-------Processing FUSE_RENAME message");
					int parent_directory=0;
					int pathLength = 0;
					int newPathLength = 0;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);

					//2) Receive new path length
					receiveMessage(&serverData->socketClient, &newPathLength, sizeof(newPathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, newPathLength);
					char *newPath = malloc(newPathLength);

					//3) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message path received : %s\n",path);

					//4) Receive new path
					receiveMessage(&serverData->socketClient, newPath, newPathLength);
					log_info(logPokeDexServer, "Message newPath received : %s\n",newPath);

					//5) Receive parent_directory
					log_info(logPokeDexServer, "Message parent_directory received --> \n");
					receiveMessage(&serverData->socketClient, &parent_directory, sizeof(parent_directory));
					log_info(logPokeDexServer, "Message parent_directory received : %i\n",parent_directory);

					int posicion = sobreescribirNombre(path, newPath, parent_directory);
					log_info(logPokeDexServer, "Message posicion received : %i\n",posicion);

					sendMessage(&serverData->socketClient, &posicion , sizeof(int));

					break;
				}
				default:{
					//memset(FUSEOperation ,0 , sizeof(enum_FUSEOperations));
					log_error(logPokeDexServer,"Invalid operation received '%d'", *FUSEOperation);
					break;
				}
			}

		}else if (receivedBytes == 0 ){
			//The client is down when bytes received are 0
			log_error(logPokeDexServer,"The client went down while receiving! - Please check the client '%d' is down!", serverData->socketClient);
			close(serverData->socketClient);
			free(serverData);
			exitLoop = true;
		}else{
			log_error(logPokeDexServer, "Error - No able to received - Error receiving from socket '%d', with error: %d",serverData->socketClient,errno);
			close(serverData->socketClient);
			free(serverData);
			exitLoop = true;;
		}
		//pthread_mutex_unlock(&mutexG);

		if (exitLoop){
			break;
		}

	}
	free(FUSEOperation);
}
