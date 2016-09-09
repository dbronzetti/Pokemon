/*
 ============================================================================
 Name        : Mapa.c
 ============================================================================
 */

#include "Mapa.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *mapa = string_new();
	;
	char *pokedex = string_new();
	pthread_t serverThread;

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	for (i = 0; i < argc; i++) {
		//chekea el nick del mapa
		if (strcmp(argv[i], "-m") == 0) {
			mapa = argv[i + 1];
			printf("Nombre del mapa: '%s'\n", mapa);
		}
		//chekea la carpeta del pokedex
		if (strcmp(argv[i], "-p") == 0) {
			pokedex = argv[i + 1];
			printf("Directorio Pokedex: '%s'\n", pokedex);
		}
		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	char* rutaMetadata = string_from_format("%s/Mapas/%s/metadata.dat",
			pokedex, mapa);

	printf("Directorio de la metadata del mapa '%s': '%s'\n", mapa,
			rutaMetadata);

	logMapa = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa);
	printf("Tiempo de checkeo de deadlock: %d\n", metadataMapa.tiempoChequeoDeadlock);
	printf("Batalla: %d\n", metadataMapa.batalla);
	printf("Algoritmo: %s\n", metadataMapa.algoritmo);
	printf("Quantum: %d\n", metadataMapa.quantum);
	printf("Retardo: %d\n", metadataMapa.retardo);
	printf("IP: %s\n", metadataMapa.ip);
	printf("Puerto: %d\n", metadataMapa.puerto);
	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	puts("Bienvenido al mapa");
	pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);

	pthread_join(serverThread, NULL);

	return 0;

}

void startServerProg() {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	int socketServer;

	exitCode = openServerConnection(metadataMapa.puerto, &socketServer);
	log_info(logMapa, "SocketServer: %d", socketServer);

	//If exitCode == 0 the server connection is opened and listening
	if (exitCode == 0) {
		log_info(logMapa, "the server is opened");

		exitCode = listen(socketServer, SOMAXCONN);

		if (exitCode < 0) {
			log_error(logMapa, "Failed to listen server Port.");
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

	exitCode = acceptClientConnection(&serverData->socketServer,
			&serverData->socketClient);

	if (exitCode == EXIT_FAILURE) {
		log_warning(logMapa,
				"There was detected an attempt of wrong connection");
		close(serverData->socketClient);
		free(serverData);
	} else {

		log_info(logMapa, "The was received a connection in socket: %d.",
				serverData->socketClient);
		//Create thread attribute detached
		pthread_attr_t handShakeThreadAttr;
		pthread_attr_init(&handShakeThreadAttr);
		pthread_attr_setdetachstate(&handShakeThreadAttr,
		PTHREAD_CREATE_DETACHED);

		//Create thread for checking new connections in server socket
		pthread_t handShakeThread;
		pthread_create(&handShakeThread, &handShakeThreadAttr,
				(void*) handShake, serverData);

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
	int receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,
			sizeof(messageSize));

	//Receive message using the size read before
	memcpy(&messageSize, messageRcv, sizeof(int));
	messageRcv = realloc(messageRcv, messageSize);
	receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,
			messageSize);

	//starting handshake with client connected
	t_MessageGenericHandshake *message = malloc(
			sizeof(t_MessageGenericHandshake));
	deserializeHandShake(message, messageRcv);

	//Now it's checked that the client is not down
	if (receivedBytes == 0) {
		log_error(logMapa,
				"The client went down while handshaking! - Please check the client '%d' is down!",
				serverData->socketClient);
		close(serverData->socketClient);
		free(serverData);
	} else {
		switch ((int) message->process) {

		case ENTRENADOR: {
			log_info(logMapa, "Message from '%s': %s",
					getProcessString(message->process), message->message);
			puts("Ha ingresado un nuevo entrenador");
			exitCode = sendClientAcceptation(&serverData->socketClient);

			if (exitCode == EXIT_SUCCESS) {

				//Create thread attribute detached
				pthread_attr_t processMessageThreadAttr;
				pthread_attr_init(&processMessageThreadAttr);
				pthread_attr_setdetachstate(&processMessageThreadAttr,
				PTHREAD_CREATE_DETACHED);

				//Destroy thread attribute
				pthread_attr_destroy(&processMessageThreadAttr);
			}

			break;
		}
		default: {
			log_error(logMapa,
					"Process not allowed to connect - Invalid process '%s' tried to connect to MAPA",
					getProcessString(message->process));
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

