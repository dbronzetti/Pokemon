/*
 ============================================================================
 Name        : Mapa.c
 ============================================================================
 */

#include "Mapa.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *mapa = string_new();
	char *pokedex = string_new();
	pthread_t serverThread;

	listaDePokenest = list_create();

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

	char* rutaMetadata = string_from_format("%s/Mapas/%s/metadata.dat", pokedex,
			mapa);

	char* rutaPokenest = string_from_format("%s/Mapas/%s/Pokenest/", pokedex,
			mapa);

	printf("Directorio de la metadata del mapa '%s': '%s'\n", mapa,
			rutaMetadata);

	logMapa = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa);
	printf("Tiempo de checkeo de deadlock: %d\n",
			metadataMapa.tiempoChequeoDeadlock);
	printf("Batalla: %d\n", metadataMapa.batalla);
	printf("Algoritmo: %s\n", metadataMapa.algoritmo);
	printf("Quantum: %d\n", metadataMapa.quantum);
	printf("Retardo: %d\n", metadataMapa.retardo);
	printf("IP: %s\n", metadataMapa.ip);
	printf("Puerto: %d\n", metadataMapa.puerto);
	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	recorrerdirDePokenest(rutaPokenest);

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

int recorrerdirDePokenest(char* rutaDirPokenest) {

	int i;

	if ((dipPokenest = opendir(rutaDirPokenest)) == NULL) {
		log_error(logMapa, "Error al abrir el directorio donde se encuentran las pokenests.");
		return -1;
	}

	while ((ditPokenest = readdir(dipPokenest)) != NULL) {
		i++;

		if ((strcmp(ditPokenest->d_name, ".") != 0)
				&& (strcmp(ditPokenest->d_name, "..") != 0)) {

			char* rutaPokemon = string_from_format("%s%s", rutaDirPokenest,
					ditPokenest->d_name);
			recorrerCadaPokenest(rutaPokemon);

		}

	}

	if (closedir(dipPokenest) == -1) {
		log_error(logMapa, "Error al cerrar el directorio donde se encuentran las pokenest.");
		return -1;
	}

	return 0;

}

int recorrerCadaPokenest(char* rutaDeUnaPokenest) {

	int i;
	int b = 0;

	t_pokenest* pokenest = malloc(sizeof(t_pokenest));
	pokenest->pokemon = malloc((sizeof(int)));

	if ((dipPokemones = opendir(rutaDeUnaPokenest)) == NULL) {
		log_error(logMapa, "Error al abrir el directorio %s", rutaDeUnaPokenest);
		return -1;
	}

	while ((ditPokemones = readdir(dipPokemones)) != NULL) {
		i++;

		if ((strcmp(ditPokemones->d_name, ".") != 0)
				&& (strcmp(ditPokemones->d_name, "..") != 0)) {
			if ((strcmp(ditPokemones->d_name, "metadata.dat") == 0)) {

				char* rutaMetadataPokenest = string_from_format(
						"%s/metadata.dat", rutaDeUnaPokenest);
				pokenest->metadata = crearArchivoMetadataPokenest(rutaMetadataPokenest);

			}

			else{

				char* rutaDelPokemon = string_from_format("%s/%s",rutaDeUnaPokenest, ditPokemones->d_name);

				pokenest->pokemon[b] = levantarNivelDelPokemon(rutaDelPokemon);

				b++;

			}

		}

	}

	if (closedir(dipPokemones) == -1) {
		log_error(logMapa, "Error al cerrar el directorio %s", rutaDeUnaPokenest);
		return -1;
	}

	list_add(listaDePokenest, pokenest);

	return 0;

}

t_metadataPokenest crearArchivoMetadataPokenest(char* rutaMetadataPokenest) {
	t_config* metadata;
	metadata = config_create(rutaMetadataPokenest);
	t_metadataPokenest metadataPokenest;

	metadataPokenest.tipo = config_get_string_value(metadata, "Tipo");
	metadataPokenest.id = config_get_string_value(metadata, "Identificador");

	char* posicionSinParsear = config_get_string_value(metadata, "Posicion");

	char** posicionYaParseadas = string_split(posicionSinParsear, ";");

	metadataPokenest.pos_x = atoi(posicionYaParseadas[0]);
	metadataPokenest.pos_y = atoi(posicionYaParseadas[1]);

	free(metadata);

	return metadataPokenest;
}

int levantarNivelDelPokemon(char* rutaDelPokemon){

	t_config* metadata;
	metadata = config_create(rutaDelPokemon);
	return config_get_int_value(metadata, "Nivel");

	free(metadata);
}
