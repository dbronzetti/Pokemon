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
	listaDeEntrenadores = list_create();
	semaforo_wait = 1;
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

	char* rutaMetadata = string_from_format("%s/Mapas/%s/metadata.dat", pokedex, mapa);

	char* rutaPokenest = string_from_format("%s/Mapas/%s/Pokenest/", pokedex, mapa);


	logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	log_info(logMapa, "Directorio de la metadata del mapa '%s': '%s'\n", mapa,	rutaMetadata);

	log_info(logMapa, "@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa);
	log_info(logMapa, "Tiempo de checkeo de deadlock: %d\n",metadataMapa.tiempoChequeoDeadlock);
	log_info(logMapa, "Batalla: %d\n", metadataMapa.batalla);
	log_info(logMapa, "Algoritmo: %s\n", metadataMapa.algoritmo);
	log_info(logMapa, "Quantum: %d\n", metadataMapa.quantum);
	log_info(logMapa, "Retardo: %d\n", metadataMapa.retardo);
	log_info(logMapa, "IP: %s\n", metadataMapa.ip);
	log_info(logMapa, "Puerto: %d\n", metadataMapa.puerto);
	log_info(logMapa, "@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	recorrerdirDePokenest(rutaPokenest);

	log_info(logMapa, "Bienvenido al mapa");

	sleep(2);
	system("clear"); //Espera 2 segundos y borra todo

	dibujarMapa();

	pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);
	pthread_join(serverThread, NULL);

	return 0;

}

void startServerProg() {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	int socketServer;// listening socket descriptor
	fd_set master; // master file descriptor list
	fd_set read_fds;// temp file descriptor list for select()
	int fdmax;// maximum file descriptor number

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	exitCode = openSelectServerConnection(metadataMapa.puerto, &socketServer);
	log_info(logMapa, "SocketServer: %d", socketServer);

	//If exitCode == 0 the server connection is opened and listening
	if (exitCode == 0) {
		log_info(logMapa, "the server is opened");

		exitCode = listen(socketServer, SOMAXCONN);

		if (exitCode < 0) {
			log_error(logMapa, "Failed to listen server Port.");
			return;
		}

		// add the listener to the master set
		FD_SET(socketServer, &master);

		// keep track of the biggest file descriptor
		fdmax = socketServer; // so far, it's this one

		while (1) {

			read_fds = master; // copy it
			if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(4);
			}

			int i;

			// run through the existing connections looking for data to read
			for(i = 0; i <= fdmax; i++) {
				if (FD_ISSET(i, &read_fds)) { // we got one!!
					if (i == socketServer) {
						// handle new connections
						newClients(&socketServer, &master, &fdmax);

					} else {
						// handle data from a client
						//Receive message size
						if (semaforo_wait == 1){
							semaforo_wait = 0;
							// we got some data from a client
							//Create thread attribute detached
							pthread_attr_t processMessageThreadAttr;
							pthread_attr_init(&processMessageThreadAttr);
							pthread_attr_setdetachstate(&processMessageThreadAttr, PTHREAD_CREATE_DETACHED);

							//Create thread for checking new connections in server socket
							pthread_t processMessageThread;
							t_serverData *serverData = malloc(sizeof(t_serverData));
							memcpy(&serverData->socketServer, &socketServer ,sizeof(serverData->socketServer));
							memcpy(&serverData->socketClient, &i ,sizeof(serverData->socketClient));

							pthread_create(&processMessageThread, &processMessageThreadAttr, (void*) processMessageReceived, serverData);
							pthread_join(processMessageThread, NULL);
							//Destroy thread attribute
							pthread_attr_destroy(&processMessageThreadAttr);

						}
					} // END handle data from client
				} // END got new incoming connection
			} // END looping through file descriptors
		}
	}

}



void newClients(int *socketServer, fd_set *master, int *fdmax) {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure

	t_serverData *serverData = malloc(sizeof(t_serverData));
	memcpy(&serverData->socketServer, socketServer,sizeof(serverData->socketServer));

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
		log_warning(logMapa,"There was detected an attempt of wrong connection");
		close(serverData->socketClient);
		free(serverData);
	} else {

		log_info(logMapa, "The was received a connection in socket: %d.", serverData->socketClient);

		FD_SET(serverData->socketClient, master); // add to master set

		if (serverData->socketClient > *fdmax) {    // keep track of the max
			*fdmax = serverData->socketClient;
		}

		handShake(serverData);

	}// END handshakes

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
		log_error(logMapa,"The client went down while handshaking! - Please check the client '%d' is down!",serverData->socketClient);
		close(serverData->socketClient);
		free(serverData);
	} else {
		switch ((int) message->process) {
			case ENTRENADOR: {
				log_info(logMapa, "Message from '%s': %s",getProcessString(message->process), message->message);
				log_info(logMapa, "Ha ingresado un nuevo ENTRENADOR");
				exitCode = sendClientAcceptation(&serverData->socketClient);

				if (exitCode == EXIT_SUCCESS) {
					log_info(logMapa, "The client '%d' has received SUCCESSFULLY the handshake message",serverData->socketClient);
				}
				break;
			}
			case POKEDEX_CLIENTE: {
				log_info(logMapa, "Message from '%s': %s",	getProcessString(message->process), message->message);
				log_info(logMapa, "Ha ingresado un nuevo POKEDEX_CLIENTE");
				exitCode = sendClientAcceptation(&serverData->socketClient);

				if (exitCode == EXIT_SUCCESS) {
					log_info(logMapa, "The client '%d' has received SUCCESSFULLY the handshake message",serverData->socketClient);
				}
				break;
			}
			default: {
				log_error(logMapa, "Process not allowed to connect - Invalid process '%s' tried to connect to MAPA",	getProcessString(message->process));
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

void processMessageReceived (void *parameter){
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
	t_Mensaje *message = malloc(sizeof(t_Mensaje));
	deserializeClientMessage(message, messageRcv);

	//Now it's checked that the client is not down
	if (receivedBytes == 0) {
		log_error(logMapa,"The client went down while sending message! - Please check the client '%d' is down!",serverData->socketClient); //disculpen mi ingles is very dificcul
		close(serverData->socketClient);
		free(serverData);
	} else {
		switch (message->tipo) {
			case NUEVO: {
				log_info(logMapa, "Creating new user: %s",message->mensaje);
				crearEntrenadorYDibujar(message->mensaje[0], serverData->socketClient);
				semaforo_wait = 1;
				break;
			}
			default: {
				log_error(logMapa, "Message not allowed. Invalid message '%s' tried to send to MAPA",	getProcessString(message->tipo));
				close(serverData->socketClient);
				free(serverData);
				break;
			}
		}
	}

	free(message->mensaje);
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

	pokenest->pokemon[b] = -1; //Esto avisa que el array ya termino
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

void dibujarMapa(){
	int rows, cols;
	items = list_create();

	nivel_gui_inicializar();

	int ultimoElemento = list_size(listaDePokenest);
	int i = 0;

    nivel_gui_get_area_nivel(&rows, &cols);

    for (i=0 ; i<ultimoElemento; i++)
    {
    	t_pokenest* pokenest = list_get(listaDePokenest,i);
    	int cantidadDePokemones = 0;
    	while (pokenest->pokemon[cantidadDePokemones]!= -1)
    		cantidadDePokemones++;
	    CrearCaja(items, pokenest->metadata.id[0] , pokenest->metadata.pos_x, pokenest->metadata.pos_y, cantidadDePokemones);
    }

	nivel_gui_dibujar(items, "TEST");


}

void crearEntrenadorYDibujar(char simbolo, int socket){
	t_entrenador* nuevoEntrenador = malloc(sizeof(t_entrenador*));

	nuevoEntrenador->simbolo = simbolo;
	nuevoEntrenador->pos_x = 1; //por defecto se setea en el (1,1) creo que lo dijeron en la charla, por las dudas preguntar.
	nuevoEntrenador->pos_y = 1;
	nuevoEntrenador->posD_x = -1; //flag para representar que por el momento no busca ninguna ubicacion
	nuevoEntrenador->posD_y = -1;
	nuevoEntrenador->socket = socket;

	CrearPersonaje(items, simbolo, nuevoEntrenador->pos_x, nuevoEntrenador->pos_y);
	list_add(listaDeEntrenadores, nuevoEntrenador);
	nivel_gui_dibujar(items, "TEST");

}

