/*
 ============================================================================
 Name        : PokeDex_Servidor.c
 ============================================================================
 */

#include "PokeDex_Servidor.h"
//joel
int main(int argc, char **argv) {
	char *logFile = NULL;
	pthread_t serverThread;

	int archivoID = obtenerIDDelArchivo("/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Servidor/Debug/challenge.bin");
	int tamanioDelArchivo = setearTamanioDelArchivo(archivoID);

	inicializarOSADA(archivoID);
	obtenerHeader();\

	setearConstantesDePosicionDeOsada();

	obtenerBitmap();
    obtenerTablaDeArchivos();
    obtenerTablaDeAsignacion();


	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	for (i = 0; i < argc; i++) {

		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	logPokeDexServer = log_create(logFile, "POKEDEX_SERVER", 0, LOG_LEVEL_TRACE);

	puts("Soy el PokeDex Servidor"); /* prints Soy el PokeDex Servidor */

	//getting environment variable for listening
	PORT = atoi(getenv("POKEPORT"));

	pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);

	pthread_join(serverThread, NULL);

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
	memcpy(&serverData->socketServer, parameter,
			sizeof(serverData->socketServer));

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
	/*t_list* lista2 = list_create();
	osada_file *tablaDeArchivo2= malloc(64);*/
	while(1){
		//0) Receive FUSE Operation
		enum_FUSEOperations FUSEOperation;
		int receivedBytes = receiveMessage(&serverData->socketClient, &FUSEOperation, sizeof(enum_FUSEOperations));

		if ( receivedBytes > 0 ){

			log_info(logPokeDexServer, "Processing POKEDEX_CLIENTE message received");

			switch (FUSEOperation){
				case FUSE_WRITE:{
						log_info(logPokeDexServer, "Processing FUSE_WRITE message");

						int pathLength = 0;
						//1) Receive path length
						receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
						log_info(logPokeDexServer, "FUSE_WRITE - Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
						char *path = malloc(pathLength);
						//2) Receive path
						receiveMessage(&serverData->socketClient, path, pathLength);
						log_info(logPokeDexServer, "FUSE_WRITE - Message path received : %s\n",path);
						//3) Content size
						int contentSize = 0;
						receiveMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
						log_info(logPokeDexServer, "FUSE_WRITE - Content size: %d", contentSize);
						char *content = malloc(contentSize);
						//4) Content path
						receiveMessage(&serverData->socketClient, content, contentSize);
						log_info(logPokeDexServer, "FUSE_WRITE -Message content received : %s\n",content);

						crearUnArchivo(content, contentSize, path);
						log_info(logPokeDexServer, "FUSE_WRITE - TERMINO DE CREAR\n");

						sendMessage(&serverData->socketClient, &contentSize, sizeof(contentSize));
						break;
				}
				case FUSE_CREATE:{
					log_info(logPokeDexServer, "Processing FUSE_CREATE message");
					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					//get padre from path received for passing it to escribirEnLaTablaDeArchivos
					int posBloquePadre = obtener_bloque_padre(path);

					log_info(logPokeDexServer, "escribirEnLaTablaDeArchivos");
					escribirEnLaTablaDeArchivos(posBloquePadre, 0, path, 666);

					int exitCode = EXIT_SUCCESS;
					sendMessage(&serverData->socketClient, &exitCode, sizeof(exitCode));

					break;
				}
				case FUSE_MKDIR:{
					log_info(logPokeDexServer, "Processing FUSE_READ message");

					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					crearUnArchivo("hola mundo\n", 128,"hola.txt\0");

					sendMessage(&serverData->socketClient, "bien\0" , strlen("bien\0"));
					break;
				}
				case FUSE_READ:{
					log_info(logPokeDexServer, "Processing FUSE_READ message");

					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message path received : %s\n",path);

					osada_block_pointer posicion = buscarArchivo(path);
					t_list *conjuntoDeBloquesDelArchivo = crearPosicionesDeBloquesParaUnArchivo(posicion);
					//verContenidoDeArchivo(conjuntoDeBloquesDelArchivo);

					int i;
					//char *contenido = malloc(OSADA_BLOCK_SIZE * conjuntoDeBloquesDelArchivo->elements_count + 1);
					char *string = string_new();
					log_info(logPokeDexServer, "ENTRA EN EL FOR DE BLOQUES\n");
					memcpy(string, &conjuntoDeBloquesDelArchivo->elements_count, sizeof(int));
					char *bloqueDeDatos = malloc(OSADA_BLOCK_SIZE);

					for (i = 0; i < conjuntoDeBloquesDelArchivo->elements_count; i++) {

						int bloque2 = list_get(conjuntoDeBloquesDelArchivo, i);
						bloque2 *= 64;
						memcpy(bloqueDeDatos, &OSADA[DATA_BLOCKS+bloque2], OSADA_BLOCK_SIZE );
						log_info(logPokeDexServer, "bloqueDeDatos: %s\n", bloqueDeDatos);

						bloqueDeDatos[OSADA_BLOCK_SIZE] = '\0';
						string_append(&string, bloqueDeDatos);
					}

					free(bloqueDeDatos);

					string_append(&string, "\0");
					log_info(logPokeDexServer, "string to be sent for file '%s': %s\n", path, string);

					int messageSize = strlen(string) + 1; //+1 due to /0
					sendMessage(&serverData->socketClient, &messageSize , sizeof(messageSize));
					sendMessage(&serverData->socketClient, string , messageSize);

					break;
				}
				case FUSE_READDIR:{
					log_info(logPokeDexServer, "Processing FUSE_READDIR message");
					int pathLength = 0;
					//1) Receive path length
					receiveMessage(&serverData->socketClient, &pathLength, sizeof(pathLength));
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					//get padre from path received for passing it to crearArbolAPartirDelPadre
					int posBloquePadre = obtener_bloque_padre(path);
					lista = crearArbolAPartirDelPadre(posBloquePadre);
					log_info(logPokeDexServer,"lista->elements_count: %i\n",lista->elements_count);

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
					log_info(logPokeDexServer, "Message size received in socket cliente '%d': %d", serverData->socketClient, pathLength);
					char *path = malloc(pathLength);
					//2) Receive path
					receiveMessage(&serverData->socketClient, path, pathLength);
					log_info(logPokeDexServer, "Message size received : %s\n",path);

					int posArchivo = obtener_bloque_archivo(path);

					int elementCount;
					char *mensajeOsada = malloc(sizeof(elementCount));
					int messageSize = sizeof(elementCount);

					if (posArchivo != -666){
						osada_file bloqueArchivo = TABLA_DE_ARCHIVOS[posArchivo];

						printf("Paso el buscarArchivo: \n");
						printf("File Name: %s\n",bloqueArchivo.fname);
						elementCount = 1;
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain only 1 OSADA_FILE
						mensajeOsada = serializeBloque(&bloqueArchivo, mensajeOsada, &messageSize);

					}else{
						elementCount = 0;
						memcpy(mensajeOsada, &elementCount, sizeof(elementCount));//this will tell to PokeDexCliente that the message is going to contain 0 OSADA_FILE because the file was not found
					}

					sendMessage(&serverData->socketClient, mensajeOsada , messageSize);

					log_info(logPokeDexServer, "HIZO SEND\n");

					break;
				}
				default:{
					log_error(logPokeDexServer,"Invalid operation received '%d'", FUSEOperation);
					close(serverData->socketClient);
					free(serverData);
					break;
				}
			}

		}else if (receivedBytes == 0 ){
			//The client is down when bytes received are 0
			log_error(logPokeDexServer,"The client went down while receiving! - Please check the client '%d' is down!", serverData->socketClient);
			close(serverData->socketClient);
			free(serverData);
			break;
		}else{
			log_error(logPokeDexServer, "Error - No able to received - Error receiving from socket '%d', with error: %d",serverData->socketClient,errno);
			close(serverData->socketClient);
			free(serverData);
			break;
		}
	}
}
