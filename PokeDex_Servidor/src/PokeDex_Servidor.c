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

	logPokeDexServer = log_create(logFile, "POKEDEX_SERVER", 0, LOG_LEVEL_TRACE);

	int archivoID = obtenerIDDelArchivo(diskFile);
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	inicializarOSADA(archivoID);
	obtenerHeader();

	setearConstantesDePosicionDeOsada();

	obtenerBitmap();
    obtenerTablaDeArchivos();
    obtenerTablaDeAsignacion();


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

					//3) time
					long int tiempo=0;
					receiveMessage(&serverData->socketClient, &tiempo, sizeof(tiempo));
					log_info(logPokeDexServer, "FUSE_UTIMENS - Message tiempo received : %i\n",tiempo);

					//get padre from path received
					parent_directory = obtener_bloque_padre(path);

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

					int exitCode = borrarUnDirectorio(path);
					log_info(logPokeDexServer, "Message exitCode : %i\n",exitCode);

					sendMessage(&serverData->socketClient, &exitCode , sizeof(exitCode));
					break;
				}
				case FUSE_WRITE:{
					//log_info(logPokeDexServer,"************************ Processing FUSE_WRITE message ********************************\n");
					int posDelaTablaDeArchivos = -999;
					int pathLength = 0;
					int ultimoPunteroDeLosBloques = 1;

					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					//log_info(logPokeDexServer, "FUSE_WRITE - pathLength'%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);

					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					//log_info(logPokeDexServer, "FUSE_WRITE - path: %s\n",path);

					//3) Content size
					int contentSize = 0;
					receiveMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
					log_info(logPokeDexServer, "FUSE_WRITE - Content size: %d", contentSize);

					unsigned char *content = malloc(contentSize);

					//4) Content path
					receiveMessage(&serverData->socketClient, content, contentSize);
					//log_info(logPokeDexServer, "FUSE_WRITE - Message content received : %s\n",content);

					//5) Receive offset
					int bufferOffset = 0;
					receiveMessage(&serverData->socketClient, &bufferOffset, sizeof(bufferOffset));

					int ultimoPuntero = escribirUnArchivo(content, contentSize, path,bufferOffset);
					//log_info(logPokeDexServer, "FUSE_WRITE - ultimoPuntero: %d\n", ultimoPuntero);
					//printf("FUSE_WRITE - ultimoPunteroDeLosBloques: %d\n", ultimoPunteroDeLosBloques);

					sendMessage(&serverData->socketClient, &ultimoPuntero, sizeof(ultimoPuntero));

					//log_info(logPokeDexServer,"********************************* TERMINO EL WRITE *********************\n");
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
					int posBloquePadre = obtener_Nuevo_padre(path);

					log_info(logPokeDexServer, "FUSE_CREATE - escribirEnLaTablaDeArchivos");
					posDelaTablaDeArchivos = escribirEnLaTablaDeArchivos(posBloquePadre, 0, path, first_block_init, posDelaTablaDeArchivos);

					log_info(logPokeDexServer, "FUSE_CREATE - posDelaTablaDeArchivos a enviar %d", posDelaTablaDeArchivos);

					sendMessage(&serverData->socketClient, &posDelaTablaDeArchivos, sizeof(posDelaTablaDeArchivos));
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

					//get padre from path received
					int posArchivo = 0;

					hacerElTruncate(0,path, &posArchivo); //No semaforear

					pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
					memset(TABLA_DE_ARCHIVOS[posArchivo].fname, 0, OSADA_FILENAME_LENGTH);
					TABLA_DE_ARCHIVOS[posArchivo].state = 0;
					TABLA_DE_ARCHIVOS[posArchivo].parent_directory = 65535;//reseteo a bloque padre
					guardarEnOsada(DESDE_PARA_TABLA_DE_ARCHIVOS, TABLA_DE_ARCHIVOS, TAMANIO_TABLA_DE_ARCHIVOS);
					pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);

					int exitCode = EXIT_SUCCESS;
					sendMessage(&serverData->socketClient, &exitCode , sizeof(exitCode));

					break;
				}
				case FUSE_TRUNCATE:{
					    log_info(logPokeDexServer, "-------Processing FUSE_TRUNCATE message");
						printf("******************* Processing FUSE_TRUNCATE message ****************\n");
						int pathLength = 0;

						//1) Receive path length
						receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
						log_info(logPokeDexServer, "FUSE_TRUNCATE - pathLength : %d", pathLength);
						char *path = malloc(pathLength);

						//2) Receive path
						receiveMessage(&serverData->socketClient, path, pathLength);
						log_info(logPokeDexServer, "FUSE_TRUNCATE - path: %s\n",path);

						//3) offset
						int offset = 0;
						receiveMessage(&serverData->socketClient, &offset, sizeof(offset));
						log_info(logPokeDexServer, "FUSE_TRUNCATE - offset: %d", offset);

						int possArchivo;
						int exit_code = hacerElTruncate(offset, path,&possArchivo); //possArchivo->Esto esta porque se utiliza tambien en el write
						log_info(logPokeDexServer, "FUSE_TRUNCATE - ultimoPuntero: %d\n", exit_code);

						sendMessage(&serverData->socketClient, &exit_code, sizeof(exit_code));
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

					posTablaDeArchivos = crearUnDirectorio(path);
					log_info(logPokeDexServer, "Message posTablaDeArchivosreceived : %i\n",posTablaDeArchivos);

					sendMessage(&serverData->socketClient, &posTablaDeArchivos , sizeof(int));
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
					log_info(logPokeDexServer, "FUSE_READ - Message path received : %s\n",path);

					//3) Receive offset
					printf("******************* FUSE_READ - Receive offset ****************\n");
					int offset = 0;
					receiveMessage(&serverData->socketClient, &offset, sizeof(offset));
					log_info(logPokeDexServer, "FUSE_READ - Message offset received : %i\n",offset);

					//4) Receive size to read
					printf("******************* FUSE_READ - Receive size ****************\n");
					int size = 0;
					receiveMessage(&serverData->socketClient, &size, sizeof(size));
					log_info(logPokeDexServer, "FUSE_READ - Message size received : %i\n",size);

					if (path != NULL){

						parent_directory = obtener_bloque_padre(path);
						log_info(logPokeDexServer, "FUSE_READ - parent_directory : %d\n",parent_directory);
						osada_block_pointer posicion = devolverOsadaBlockPointer(path, parent_directory);

						printf("FUSE_READ - posicion: %i\n",posicion);
						//char *string = string_new();
						if (posicion != -999){// -999 = NO LO ENCONTRO

							t_list *conjuntoDeBloquesDelArchivo = obtenerElListadoDeBloquesCorrespondientesAlArchivo(posicion);

							log_info(logPokeDexServer, "FUSE_READ - conjuntoDeBloquesDelArchivo: %i\n",conjuntoDeBloquesDelArchivo->elements_count);

							//sending notification to client about the file found
							int fileFound = EXIT_SUCCESS;
							sendMessage(&serverData->socketClient, &fileFound , sizeof(fileFound));

							int cantBloquesParaEnviar = size / OSADA_BLOCK_SIZE;
							int bloqueOffset = offset / OSADA_BLOCK_SIZE;

							log_info(logPokeDexServer, "FUSE_READ - cantBloquesParaEnviar: %d\n",cantBloquesParaEnviar);
							log_info(logPokeDexServer, "FUSE_READ - bloqueOffset: %d", bloqueOffset);

							if (bloqueOffset <= conjuntoDeBloquesDelArchivo->elements_count){//checking that the block requested is WITHIN the file block
								char *bloqueDeDatos = malloc(size);
								memset(bloqueDeDatos,0,size);

								int ultimoBloqueParaEnviar = bloqueOffset + cantBloquesParaEnviar;
								log_info(logPokeDexServer, "ultimoBloqueParaEnviar: %d", ultimoBloqueParaEnviar);
								int i, j = 0;

								int messageSize = size;
								int offset;
								for (i = bloqueOffset; i < ultimoBloqueParaEnviar ; i++) { //we need to send the number of blocks requested (by the size) from the offset received
									int bloque2 = list_get(conjuntoDeBloquesDelArchivo, i);
									bloque2 *= 64;

									offset = (j * OSADA_BLOCK_SIZE);

									pthread_mutex_lock(&OSADAmutex);
									memcpy(bloqueDeDatos + offset, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );
									pthread_mutex_unlock(&OSADAmutex);
//									log_info(logPokeDexServer, "bloqueDeDatos: %s", bloqueDeDatos);

									//bloqueDeDatos[OSADA_BLOCK_SIZE] = '\0';
									//log_info(logPokeDexServer, "messageSize: %i\n", messageSize);

									j++;
								}

								sendMessage(&serverData->socketClient, &messageSize , sizeof(messageSize));
								sendMessage(&serverData->socketClient, bloqueDeDatos , messageSize);
								log_info(logPokeDexServer, "j: %d", j);
								free(bloqueDeDatos);
							}

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

					//get padre from path received
					int posBloquePadre = obtener_Nuevo_padre(path);
					log_info(logPokeDexServer,"FUSE_READDIR - posBloquePadre: %i\n", posBloquePadre);
					printf("FUSE_READDIR - posBloquePadre: %i\n", posBloquePadre);

					lista = crearArbolAPartirDelPadre(posBloquePadre);

					log_info(logPokeDexServer,"FUSE_READDIR - lista->elements_count: %i\n",lista->elements_count);
					printf("FUSE_READDIR - lista->elements_count: %i\n",lista->elements_count);

					int messageSize = 0;
					char *mensajeOsada = serializeListaBloques(lista, &messageSize);
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

					int posArchivo = -666;
					int bloquePadre = obtener_bloque_padre_NUEVO(path, &posArchivo);

					int elementCount;
					char *mensajeOsada = malloc(sizeof(elementCount));
					int messageSize = sizeof(elementCount);

					if (posArchivo != -666){
						printf("FUSE_GETATTR - Paso el buscarArchivo: \n");
						pthread_mutex_lock(&TABLA_DE_ARCHIVOSmutex);
						osada_file bloqueArchivo = TABLA_DE_ARCHIVOS[posArchivo];
						printf("FUSE_GETATTR - File Name: %s\n", bloqueArchivo.fname);

						elementCount = 1;
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain only 1 OSADA_FILE
						mensajeOsada = serializeBloque(&bloqueArchivo, mensajeOsada, &messageSize);
						pthread_mutex_unlock(&TABLA_DE_ARCHIVOSmutex);
						log_info(logPokeDexServer, "FUSE_GETATTR - !=666 elementCount: %i\n",elementCount);

					}else{
						elementCount = 0;
						log_info(logPokeDexServer, "FUSE_GETATTR - ==-666 - elementCount: %i\n",elementCount);
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain 0 OSADA_FILE because the file was not found
					}
					log_info(logPokeDexServer, "FUSE_GETATTR - messageSize: %i\n",messageSize);

					sendMessage(&serverData->socketClient, mensajeOsada , messageSize);

					log_info(logPokeDexServer, "HIZO SEND\n");

					break;
				}
				case FUSE_RENAME:{
					log_info(logPokeDexServer, "-------Processing FUSE_RENAME message");
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

					sobreescribirNombre(path, newPath);

					int exitCode = EXIT_SUCCESS; //voy a retornar siempre ok
					sendMessage(&serverData->socketClient, &exitCode , sizeof(exitCode));

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
