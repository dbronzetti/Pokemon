/*
 ============================================================================
 Name        : Mapa.c
 ============================================================================
 */

#include "Mapa.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	mapa = string_new();
	char *pokedex = string_new();
	pthread_t serverThread;
	pthread_t planificador;
	pthread_t detectorDeadlocks;
	listaDeEntrenadores = list_create();
	listaDePokenest = list_create();
	colaDeBloqueados = queue_create();
	colaDeListos = queue_create();

	//initializing mutexes
	pthread_mutex_init(&setEntrenadoresMutex, NULL);
	pthread_mutex_init(&colaDeListosMutex, NULL);
	pthread_mutex_init(&colaDeBloqueadosMutex, NULL);
	pthread_mutex_init(&setFDmutex, NULL);
	pthread_mutex_init(&itemsMutex, NULL);
	pthread_mutex_init(&listaDePokenestMutex, NULL);
	pthread_mutex_init(&setRecibirMsj, NULL);
	pthread_mutex_init(&borradoDeEntrenadores, NULL);

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

	logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	log_info(logMapa, "Directorio de la metadata del mapa '%s': '%s'\n", mapa,
			rutaMetadata);

	log_info(logMapa, "@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa);
	log_info(logMapa, "Tiempo de checkeo de deadlock: %d\n",
			metadataMapa.tiempoChequeoDeadlock);
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
	pthread_create(&planificador, NULL, (void*) planificar, NULL);
	pthread_create(&detectorDeadlocks, NULL, (void*) detectarDeadlocks, NULL);

	pthread_join(serverThread, NULL);
	pthread_join(planificador, NULL);
	pthread_join(detectorDeadlocks, NULL);
	return 0;

}

void startServerProg() {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure
	int socketServer; // listening socket descriptor
	fd_set master; // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number

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
			if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(4);
			}

			int i;

			// run through the existing connections looking for data to read
			for (i = 0; i <= fdmax; i++) {
				if (FD_ISSET(i, &read_fds)) { // we got one!!
					if (i == socketServer) {
						// handle new connections
						newClients(&socketServer, &master, &fdmax);
					} else {
						// handle data from a client
						//Receive message size
						pthread_mutex_lock(&setRecibirMsj);
						// we got some data from a client
						//Create thread attribute detached
						pthread_attr_t processMessageThreadAttr;
						pthread_attr_init(&processMessageThreadAttr);
						pthread_attr_setdetachstate(&processMessageThreadAttr,
						PTHREAD_CREATE_DETACHED);

						//Create thread for checking new connections in server socket
						pthread_t processMessageThread;
						t_serverData *serverData = malloc(sizeof(t_serverData));
						memcpy(&serverData->socketServer, &socketServer,
								sizeof(serverData->socketServer));
						memcpy(&serverData->socketClient, &i,
								sizeof(serverData->socketClient));

						serverData->masterFD = &master;

						pthread_create(&processMessageThread,
								&processMessageThreadAttr,
								(void*) processMessageReceived, serverData);
						pthread_join(processMessageThread, NULL);
						//Destroy thread attribute
						pthread_attr_destroy(&processMessageThreadAttr);
						pthread_mutex_unlock(&setRecibirMsj);

//						}
					} // END handle data from client
				} // END got new incoming connection
			} // END looping through file descriptors
		}
	}

}

void newClients(int *socketServer, fd_set *master, int *fdmax) {
	int exitCode = EXIT_FAILURE; //DEFAULT Failure

	t_serverData *serverData = malloc(sizeof(t_serverData));
	memcpy(&serverData->socketServer, socketServer,
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

		pthread_mutex_lock(&setFDmutex);
		FD_SET(serverData->socketClient, master); // add to master set
		pthread_mutex_unlock(&setFDmutex);

		if (serverData->socketClient > *fdmax) {    // keep track of the max
			*fdmax = serverData->socketClient;
		}

		handShake(serverData);

	}    // END handshakes

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
			log_info(logMapa, "Ha ingresado un nuevo ENTRENADOR");
			exitCode = sendClientAcceptation(&serverData->socketClient);

			if (exitCode == EXIT_SUCCESS) {
				log_info(logMapa,
						"The client '%d' has received SUCCESSFULLY the handshake message",
						serverData->socketClient);
			}
			break;
		}
		case POKEDEX_CLIENTE: {
			log_info(logMapa, "Message from '%s': %s",
					getProcessString(message->process), message->message);
			log_info(logMapa, "Ha ingresado un nuevo POKEDEX_CLIENTE");
			exitCode = sendClientAcceptation(&serverData->socketClient);

			if (exitCode == EXIT_SUCCESS) {
				log_info(logMapa,
						"The client '%d' has received SUCCESSFULLY the handshake message",
						serverData->socketClient);
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

void processMessageReceived(void *parameter) {
	t_serverData *serverData = (t_serverData*) parameter;

	//Receive message size
	int messageSize = 0;
	char *messageRcv = malloc(sizeof(messageSize));
	int receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,
			sizeof(messageSize));

	bool buscarPorSocket(t_entrenador* entrenador) {
		return (entrenador->socket == serverData->socketClient);
	}

	if (receivedBytes <= 0) {
		// got error or connection closed by client
		if (receivedBytes == 0) {
			// connection closed
			pthread_mutex_lock(&borradoDeEntrenadores);
			pthread_mutex_lock(&setEntrenadoresMutex);
			if (list_any_satisfy(listaDeEntrenadores,
					(void*) buscarPorSocket)) {

				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);

				if (entrenador->estaEnTurno == false) { //si no esta en turno lo borramos normal

					char simboloDelEntrenador = entrenador->simbolo;

					pthread_mutex_unlock(&setEntrenadoresMutex);
					log_error(logMapa,
							"Trainer: '%c' want to  disconnect in socket: %d",
							simboloDelEntrenador, serverData->socketClient); //disculpen mi ingles is very dificcul

					log_info(logMapa, "Deleting trainer: '%c'",
							simboloDelEntrenador);
					eliminarEntrenador(simboloDelEntrenador);
					log_info(logMapa, "Trainer: '%c' deleted SUCCESSFUL",
							simboloDelEntrenador);

					int socketFD = serverData->socketClient;
					close(serverData->socketClient); // bye!
					free(messageRcv);

					pthread_mutex_lock(&setFDmutex);
					FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
					pthread_mutex_unlock(&setFDmutex);
					log_error(logMapa, "Socket: %d  disconected SUCCESSFUL",
							socketFD);
				} else {

					log_error(logMapa,
							"Trainer '%c' want to disconnect in socket: '%d' on the turn",
							entrenador->simbolo, entrenador->socket);
					int socketFD = entrenador->socket;
					entrenador->accion = DESCONECTAR;
					entrenador->socket = -1;
					entrenador->pokemonD = '0';
					pthread_mutex_unlock(&setEntrenadoresMutex);

					close(serverData->socketClient); // bye!
					free(messageRcv);

					pthread_mutex_lock(&setFDmutex);
					FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
					log_error(logMapa, "Socket: %d  disconected SUCCESSFUL",
							socketFD);
					pthread_mutex_unlock(&setFDmutex);
				}
			}

			pthread_mutex_unlock(&setEntrenadoresMutex);
			pthread_mutex_unlock(&borradoDeEntrenadores);
		} else {
//			perror("recv");
		}

	} else {
		//Receive message using the size read before
		memcpy(&messageSize, messageRcv, sizeof(int));
		messageRcv = realloc(messageRcv, messageSize);
		receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,
				messageSize);

		//starting handshake with client connected
		t_Mensaje *message = malloc(sizeof(t_Mensaje));
		deserializeClientMessage(message, messageRcv);

		//Now it's checked that the client is not down
		if (receivedBytes == 0) {
			log_error(logMapa,
					"The client went down while sending message! - Please check the client '%d' is down!",
					serverData->socketClient); //disculpen mi ingles is very dificcul
			close(serverData->socketClient);
			free(serverData);
		} else {

			switch (message->tipo) {
			case NUEVO: {
				log_info(logMapa, "Creating new trainer: %s", message->mensaje);
				crearEntrenadorYDibujar(message->mensaje[0],
						serverData->socketClient);
				log_info(logMapa, "Trainer:%s created SUCCESSFUL",
						message->mensaje);
				break;
			}

			case DESCONECTAR: {
				log_info(logMapa, "Deleting trainer: '%s'", message->mensaje);
				eliminarEntrenador(message->mensaje[0]);
				log_info(logMapa, "Trainer: '%s' deleted SUCCESSFUL",
						message->mensaje);

				close(serverData->socketClient); // bye!

				pthread_mutex_lock(&setFDmutex);
				FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
				pthread_mutex_unlock(&setFDmutex);
				break;
			}

			case CONOCER: {
				char id_pokemon = message->mensaje[0];

				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->pokemonD = id_pokemon;
				entrenador->accion = CONOCER;

				log_info(logMapa,
						"Trainer: '%c' want to know the position of: '%s'",
						entrenador->simbolo, message->mensaje);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				break;
			}

			case IR: {

				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->accion = IR;
				log_info(logMapa, "Trainer want: '%c' to go to: '%s'",
						entrenador->simbolo, message->mensaje);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				break;
			}

			case CAPTURAR: {
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->accion = CAPTURAR;
				log_info(logMapa, "Trainer: '%c' want to capture: '%s'",
						entrenador->simbolo, message->mensaje);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				break;
			}

			default: {
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->accion = ERROR;
				log_error(logMapa, "Message from the trainer %c came wrong",
						entrenador->simbolo);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				break;
			}
			}
		}

		free(message->mensaje);
		free(message);
		free(messageRcv);
	}
}

int recorrerdirDePokenest(char* rutaDirPokenest) {

	int i;

	if ((dipPokenest = opendir(rutaDirPokenest)) == NULL) {
		log_error(logMapa, "Error trying to open dir of pokenests.");
		return -1;
	}

	while ((ditPokenest = readdir(dipPokenest)) != NULL) {
		i++;

		if ((strcmp(ditPokenest->d_name, ".") != 0)
				&& (strcmp(ditPokenest->d_name, "..") != 0)) {

			char* rutaPokemon = string_from_format("%s%s", rutaDirPokenest,
					ditPokenest->d_name);
			char* nombrePokenest = ditPokenest->d_name;

			recorrerCadaPokenest(rutaPokemon, nombrePokenest);

		}

	}

	if (closedir(dipPokenest) == -1) {
		log_error(logMapa, "Error trying to close the dir of pokenest.");
		return -1;
	}

	return 0;

}

int recorrerCadaPokenest(char* rutaDeUnaPokenest, char* nombreDelaPokenest) {

	int i;

	t_pokenest* pokenest = malloc(sizeof(t_pokenest));
	pokenest->pokemon = malloc((sizeof(int)));
	pokenest->listaDePokemones = list_create();

	if ((dipPokemones = opendir(rutaDeUnaPokenest)) == NULL) {
		log_error(logMapa, "Error trying to open the dir %s",
				rutaDeUnaPokenest);
		return -1;
	}

	while ((ditPokemones = readdir(dipPokemones)) != NULL) {
		i++;

		if ((strcmp(ditPokemones->d_name, ".") != 0)
				&& (strcmp(ditPokemones->d_name, "..") != 0)) {
			if ((strcmp(ditPokemones->d_name, "metadata.dat") == 0)) { //estamos leyendo el archivo metadata

				char* rutaMetadataPokenest = string_from_format(
						"%s/metadata.dat", rutaDeUnaPokenest);
				pokenest->metadata = crearArchivoMetadataPokenest(
						rutaMetadataPokenest, nombreDelaPokenest);

			}

			else { //estamos leyendo un archvo pokemon.

				char* rutaDelPokemon = string_from_format("%s/%s",
						rutaDeUnaPokenest, ditPokemones->d_name);

				t_pokemon* pokemon = malloc(sizeof(pokemon));
				pokemon->nivel = levantarNivelDelPokemon(rutaDelPokemon);
				list_add(pokenest->listaDePokemones, pokemon);
			}

		}

	}

	if (closedir(dipPokemones) == -1) {
		log_error(logMapa, "Error trying to close the dir %s",
				rutaDeUnaPokenest);
		return -1;
	}

	void mapearIdYTipo(t_pokemon* pokemon) {
		pokemon->id = pokenest->metadata.id;
		pokemon->tipo = pokenest->metadata.tipo;
		pokemon->nombre = pokenest->metadata.nombrePokenest;
	}

	pokenest->listaDePokemones = list_map(pokenest->listaDePokemones,
			(void*) mapearIdYTipo);

	//not needed to lock this list because there is only one execution thread at this point
	list_add(listaDePokenest, pokenest);
	log_info(logMapa, "Pokenest de %s y hay %d",
			pokenest->metadata.nombrePokenest,
			list_size(pokenest->listaDePokemones)); //todo: QUIERO BORRAR ESTO PERO ME TIRA UN SEG FAULT, AYUDA DAMI :(

	char simbolo = pokenest->metadata.id;

	bool buscarPorSimbolo(t_pokenest* pokenestParam) {
		return (pokenestParam->metadata.id == simbolo);
	}

	return 0;

}

t_metadataPokenest crearArchivoMetadataPokenest(char* rutaMetadataPokenest,
		const char* nombreDeLaPokenest) {
	t_config* metadata;
	metadata = config_create(rutaMetadataPokenest);
	char *nombresito = string_new();
	t_metadataPokenest metadataPokenest;

	string_append(&nombresito, nombreDeLaPokenest);

	metadataPokenest.nombrePokenest = nombresito;
	metadataPokenest.tipo = config_get_string_value(metadata, "Tipo");
	metadataPokenest.id = config_get_string_value(metadata, "Identificador")[0];

	char* posicionSinParsear = config_get_string_value(metadata, "Posicion");

	char** posicionYaParseadas = string_split(posicionSinParsear, ";");

	metadataPokenest.pos_x = atoi(posicionYaParseadas[0]);
	metadataPokenest.pos_y = atoi(posicionYaParseadas[1]);

	free(metadata);

	return metadataPokenest;
}

int levantarNivelDelPokemon(char* rutaDelPokemon) {

	t_config* metadata;
	metadata = config_create(rutaDelPokemon);
	return config_get_int_value(metadata, "Nivel");

	free(metadata);
}

void dibujarMapa() {
	int rows, cols;
	items = list_create();

	nivel_gui_inicializar();

	int ultimoElemento = list_size(listaDePokenest);
	int i = 0;

	nivel_gui_get_area_nivel(&rows, &cols);

	for (i = 0; i < ultimoElemento; i++) {
		t_pokenest* pokenest = list_get(listaDePokenest, i);
		int cantidadDePokemones = list_size(pokenest->listaDePokemones);
		CrearCaja(items, pokenest->metadata.id, pokenest->metadata.pos_x,
				pokenest->metadata.pos_y, cantidadDePokemones);
	}

	nivel_gui_dibujar(items, mapa);

}

void crearEntrenadorYDibujar(char simbolo, int socket) {
	t_entrenador* nuevoEntrenador = malloc(sizeof(t_entrenador));

	nuevoEntrenador->simbolo = simbolo;
	nuevoEntrenador->pos_x = 1; //por defecto se setea en el (1,1) creo que lo dijeron en la charla, por las dudas preguntar.
	nuevoEntrenador->pos_y = 1;
	nuevoEntrenador->posD_x = -1; //flag para representar que por el momento no busca ninguna ubicacion
	nuevoEntrenador->posD_y = -1;
	nuevoEntrenador->socket = socket;
	nuevoEntrenador->accion = NUEVO;
	nuevoEntrenador->pokemonD = '/'; //flag para representar que por el momento no busca ningun pokemon
	nuevoEntrenador->listaDePokemonesCapturados = list_create();
	nuevoEntrenador->seEstaMoviendo = 0;
	nuevoEntrenador->seMovioEnX = 0;
	nuevoEntrenador->estaBloqueado = 0;

	//In function CrearPersonaje there is a list_add to items
	pthread_mutex_lock(&itemsMutex);
	CrearPersonaje(items, simbolo, nuevoEntrenador->pos_x,
			nuevoEntrenador->pos_y);
	pthread_mutex_unlock(&itemsMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);
	list_add(listaDeEntrenadores, nuevoEntrenador);
	pthread_mutex_unlock(&setEntrenadoresMutex);

	pthread_mutex_lock(&colaDeListosMutex);
	queue_push(colaDeListos, nuevoEntrenador);
	pthread_mutex_unlock(&colaDeListosMutex);

	pthread_mutex_lock(&itemsMutex);
	nivel_gui_dibujar(items, mapa);
	pthread_mutex_unlock(&itemsMutex);

}

void eliminarEntrenador(char simbolo) {

	pthread_mutex_lock(&itemsMutex);
	BorrarItem(items, simbolo);
	nivel_gui_dibujar(items, mapa);
	pthread_mutex_unlock(&itemsMutex);

	bool igualarACaracterCondicion(t_entrenador *entrenador) {

		return simbolo == entrenador->simbolo;
	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	t_entrenador* entrenador = list_remove_by_condition(listaDeEntrenadores,(void*) igualarACaracterCondicion);
	t_list* pokemones = entrenador->listaDePokemonesCapturados;
	pthread_mutex_unlock(&setEntrenadoresMutex);

	devolverPokemones(pokemones);

	pthread_mutex_lock(&setEntrenadoresMutex);
	free(entrenador);
	pthread_mutex_unlock(&setEntrenadoresMutex);

}

void planificar() {

	while (1) {

		while (queue_size(colaDeListos) != 0) {
			int i;
			char simbolo;

			bool buscarPorSimbolo(t_entrenador* entrenadorDeLaLista) {
				return (entrenadorDeLaLista->simbolo == simbolo);
			}

			pthread_mutex_lock(&colaDeListosMutex);
			t_entrenador* entrenador = queue_pop(colaDeListos);
			pthread_mutex_unlock(&colaDeListosMutex);
			simbolo = entrenador->simbolo;
			pthread_mutex_lock(&setEntrenadoresMutex);
			if (list_any_satisfy(listaDeEntrenadores,
					(void*) buscarPorSimbolo)) { //hacemos un if por si sacamos un entrenador que se desconecto
				entrenador->estaEnTurno = 1;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa, "Begins the turn of trainer: %c",
						entrenador->simbolo);

				for (i = 0; i < metadataMapa.quantum; i++) {

					sleep(3);

					log_info(logMapa, "Action: %d of trainer : '%c'", i,
							entrenador->simbolo);

					pthread_mutex_lock(&setEntrenadoresMutex);
					if (entrenador->pokemonD == '/') { //sino busca ninguno por el momento le preguntamos cual quiera buscar (movimiento libre)
						sendClientMessage(&entrenador->socket, "ASD", LIBRE); //no hace falta enviar un string solo un enum, por eso pongo "ASD"
						log_info(logMapa, "Trainer: '%c' has free action ",
								entrenador->simbolo);
					}
					pthread_mutex_unlock(&setEntrenadoresMutex);

					ejecutarAccionEntrenador(entrenador, &i);

				}

				pthread_mutex_lock(&setEntrenadoresMutex);
				if (list_any_satisfy(listaDeEntrenadores,
						(void*) buscarPorSimbolo)) {
					pthread_mutex_unlock(&setEntrenadoresMutex);

					if (entrenador->estaBloqueado != 1) {

						log_info(logMapa,
								"End of the turn, trainer: %c goes to the end colaDeListos",
								entrenador->simbolo);
						pthread_mutex_lock(&colaDeListosMutex);
						entrenador->estaEnTurno = 0;
						queue_push(colaDeListos, entrenador); //y aca lo mandamos a la cola de listos.
						pthread_mutex_unlock(&colaDeListosMutex);
					} else {
						log_info(logMapa,
								"End of the turn, trainer: %c goes to colaDeBloqueados",
								entrenador->simbolo);
						pthread_mutex_lock(&colaDeBloqueadosMutex);
						entrenador->estaEnTurno = 0;
						queue_push(colaDeBloqueados, entrenador); //y aca lo mandamos a la cola de bloqueados.
						pthread_mutex_unlock(&colaDeBloqueadosMutex);
					}
				} else
					pthread_mutex_unlock(&setEntrenadoresMutex);
			} else
				pthread_mutex_unlock(&setEntrenadoresMutex);

//			pthread_mutex_lock(&colaDeBloqueadosMutex);
//			pthread_mutex_lock(&colaDeListosMutex);
//			if (queue_size(colaDeBloqueados) != 0
//					&& queue_size(colaDeListos) == 0) {
//
//				t_entrenador* entrenador = queue_pop(colaDeBloqueados); //desencolamos al primero que se bloqueo
//				queue_push(colaDeListos, entrenador); //y lo encolamos a la cola de listos
//
//				log_info(logMapa, "Trainer: %c goes to colaDeListos",
//						entrenador->simbolo);
//			}
//			pthread_mutex_unlock(&colaDeListosMutex);
//			pthread_mutex_unlock(&colaDeBloqueadosMutex);

		}

	}

}

void moverEntrenador(int* pos_x, int* pos_y, int posD_x, int posD_y,
		int* seMovioEnX) { //le puse un millon de parametros para poder usarla despues como contador de distancia
	if (*seMovioEnX) { //si ya se movio en x
		if (*pos_y == posD_y) { //y ademas esta paralelo (en y) a su pokenest
			if (*pos_x > posD_x) //si esta por arriba le restamos uno.
				*pos_x = *pos_x - 1;     // se vuelve a mover en x nomas.
			else
				*pos_x = *pos_x + 1; //si esta por abajo le sumamos uno

		} else {   //sino lo movemos en Y.
			if (*pos_y > posD_y) //si esta por arriba le restamos uno
				*pos_y = *pos_y - 1;

			else
				*pos_y = *pos_y + 1;

			*seMovioEnX = 0; //y le seteamos el flag en 0.
		}
	}

	else {
		if (*pos_x == posD_x) { //si  se movio en X pero esta paralelo (en y) a su pokenest
			if (*pos_y > posD_y) //si esta por arriba le restamos uno.
				*pos_y = *pos_y - 1;     // se vuelve a mover en y nomas.
			else
				*pos_y = *pos_y + 1; //si esta por abajo le sumamos uno

		} else { //sino lo movemos en X
			if (*pos_x > posD_x) //si esta por arriba le restamos uno
				*pos_x = *pos_x - 1;

			else
				*pos_x = *pos_x + 1;

			*seMovioEnX = 1; //y le seteamos el flag en 1.
		}

	}

}

char* convertirPosicionesAString(int posX, int posY) {
	char* posXstr = "a";
	char* posYstr = "b";
//	sprintf(posXstr, "%d", posX);
//	sprintf(posYstr, "%d", posY);

	return string_from_format("%s,%s", posXstr, posYstr);

}

void planificarSRDF() {
	while (1) {

		if (queue_size(colaDeListos)) {
			ordenarColaEntrenadores();

			pthread_mutex_lock(&colaDeListosMutex);
			t_entrenador* entrenador = queue_pop(colaDeListos);
			pthread_mutex_unlock(&colaDeListosMutex);

			//TODO: CORTAR ACCION CON X TIEMPO DE NO CONTESTAR!!!!

			sleep(3);

			ejecutarAccionEntrenador(entrenador, 0); //0 it's not needed for this planificador

			pthread_mutex_lock(&colaDeListosMutex);
			if (entrenador->estaBloqueado != 1)
				queue_push(colaDeListos, entrenador);
			else
				queue_push(colaDeBloqueados, entrenador);
			pthread_mutex_unlock(&colaDeListosMutex);

		}
	}
}

void ordenarColaEntrenadores() {
	t_list* listAuxOrdenar = list_create();
	int i;
	//@TODO: Poner un semaforo que bloque la Cola de Listos.

	//obtenemos todos los entrenadores y determinamos su distancia.
	while (queue_size(colaDeListos)) {
		t_entrenador* entrenadorAux = queue_pop(colaDeListos);
		calcularCantidadMovimientos(entrenadorAux);
		list_add(listAuxOrdenar, entrenadorAux);
	}
	bool entrenador_menor(t_entrenador *entrenadorA, t_entrenador *entrenadorB) {
		return entrenadorA->distancia < entrenadorB->distancia;
	}
	list_sort(listAuxOrdenar, (void*) entrenador_menor);
	//Volvemos a regenerar la Cola con los entrenadores ya ordenados.
	for (i = 0; i < list_size(listAuxOrdenar); i++) {
		t_entrenador* entrenadorAux = list_get(listAuxOrdenar, i);
		queue_push(colaDeListos, entrenadorAux);
	}

	list_destroy(listAuxOrdenar);
}

void calcularCantidadMovimientos(t_entrenador* entrenador) {

	//Si el PokemonD esta con este caracter / significa que es un nuevo entrenador o ya capturo a su Pokemon
	if (entrenador->pokemonD == '/') { //sino busca ninguno por el momento le preguntamos cual quiera buscar (movimiento libre)
		int estaEnAccion = 1;
		sendClientMessage(&entrenador->socket, "ASD", LIBRE); //no hace falta enviar un string solo un enum, por eso pongo "ASD"
		log_info(logMapa, "Trainer: '%c' has free action ",
				entrenador->simbolo);
		while (estaEnAccion) {

			if (entrenador->accion != SIN_MENSAJE) { //este if verifica que el entrenador respondio :D

				switch (entrenador->accion) {

				case CONOCER: {

					char idPokemon = entrenador->pokemonD;

					bool buscarPokenestPorId1(t_pokenest* pokenestParam) {
						return (pokenestParam->metadata.id == idPokemon); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario
					}

					pthread_mutex_lock(&listaDePokenestMutex);
					t_pokenest* pokenestEncontrada = list_find(listaDePokenest,
							(void*) buscarPokenestPorId1);
					int posX = pokenestEncontrada->metadata.pos_x;
					int posY = pokenestEncontrada->metadata.pos_y;
					pthread_mutex_unlock(&listaDePokenestMutex);

					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenador->posD_x = posX;
					entrenador->posD_y = posY;
					char* mensajeAEnviar = convertirPosicionesAString(posX,
							posY);
					entrenador->accion = SIN_MENSAJE;
					entrenador->seEstaMoviendo = 1;
					sendClientMessage(&entrenador->socket, mensajeAEnviar,
							CONOCER);
					pthread_mutex_unlock(&setEntrenadoresMutex);

					log_info(logMapa,
							"Map send the position to the trainer: '%c'",
							entrenador->simbolo);
					estaEnAccion = 0;
					break;
				}
				}
			}
		}
	}

	//@TODO: Esto hay que cambiarlo por un contador de movimientos acorde al mapa.
	pthread_mutex_lock(&setEntrenadoresMutex);
	int pos_x = entrenador->pos_x;
	int pos_y = entrenador->pos_y;
	int posD_x = entrenador->posD_x;
	int posD_y = entrenador->posD_y;
	int seMovioEnX = entrenador->seMovioEnX;
	pthread_mutex_unlock(&setEntrenadoresMutex);
	int distancia = 0;

	while (((pos_x == posD_x) && (pos_y == posD_y)) != 1) { //mientras no haya llegado que siga contando
		moverEntrenador(&pos_x, &pos_y, posD_x, posD_y, &seMovioEnX);
//		log_info(logMapa,"[DEBUG] pos en x:%d , pos deseada x: %d , pos en y:%d , pos deseada en y: %d", pos_x,posD_x,pos_y,posD_y);
		distancia++;
	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	entrenador->distancia = distancia;
	log_info(logMapa, "Trainer: '%c' it's about %d actions of his pokenest",
			entrenador->simbolo, entrenador->distancia);
	pthread_mutex_unlock(&setEntrenadoresMutex);

}

void ejecutarAccionEntrenador(t_entrenador* entrenador, int* i) {

	int estaEnAccion = 1;
	pthread_mutex_lock(&setEntrenadoresMutex);
	if (entrenador->posD_x != -1) {
		if (entrenador->posD_x == entrenador->pos_x
				&& entrenador->posD_y == entrenador->pos_y) { //si se encuentra en la posicion deseada le avisamos que llego y asi comienza su movimiento
			sendClientMessage(&entrenador->socket, "cualquier cosa", LLEGO);

			entrenador->seEstaMoviendo = 0; // si ya llego a la posicion no se esta moviendo mas

			log_info(logMapa, "Trainer: '%c' came to the pokenest",
					entrenador->simbolo);
		}
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);

	if (entrenador->seEstaMoviendo) { // le pedimos que se mueva
		log_info(logMapa, "Trainer: '%c' has not yet reached his position",
				entrenador->simbolo);
		sendClientMessage(&entrenador->socket, "cualquiercosa", MOVETE);

	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

	while (estaEnAccion) { //una accion que puede llevar acabo el usuario dentro del turno

		if (entrenador->accion != SIN_MENSAJE) { //este if verifica que el entrenador respondio :D

			switch (entrenador->accion) {

			case CONOCER: {

				char idPokemon = entrenador->pokemonD;

				bool buscarPokenestPorId1(t_pokenest* pokenestParam) {
					return (pokenestParam->metadata.id == idPokemon); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario
				}

				pthread_mutex_lock(&listaDePokenestMutex);
				t_pokenest* pokenestEncontrada = list_find(listaDePokenest,
						(void*) buscarPokenestPorId1);
				int posX = pokenestEncontrada->metadata.pos_x;
				int posY = pokenestEncontrada->metadata.pos_y;
				pthread_mutex_unlock(&listaDePokenestMutex);

				pthread_mutex_lock(&setEntrenadoresMutex);
				entrenador->posD_x = posX;
				entrenador->posD_y = posY;
				char* mensajeAEnviar = convertirPosicionesAString(posX, posY);
				entrenador->accion = SIN_MENSAJE;
				entrenador->seEstaMoviendo = 1;
				sendClientMessage(&entrenador->socket, mensajeAEnviar, CONOCER);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa, "Map send the position to the trainer: '%c'",
						entrenador->simbolo);
				estaEnAccion = 0;
				break;
			}

			case IR: {

				moverEntrenador(&entrenador->pos_x, &entrenador->pos_y,
						entrenador->posD_x, entrenador->posD_y,
						&entrenador->seMovioEnX); //mueve el entrenador de a 1 til.
				pthread_mutex_lock(&itemsMutex);
				MoverPersonaje(items, entrenador->simbolo, entrenador->pos_x,
						entrenador->pos_y);
				nivel_gui_dibujar(items, mapa);
				pthread_mutex_unlock(&itemsMutex);
				estaEnAccion = 0;
				pthread_mutex_lock(&setEntrenadoresMutex);
				entrenador->accion = SIN_MENSAJE;
				pthread_mutex_unlock(&setEntrenadoresMutex);
				log_info(logMapa, "Trainer: '%c', moves to x: %d, y: %d ",
						entrenador->simbolo, entrenador->pos_x,
						entrenador->pos_y);
				break;
			}

			case CAPTURAR: {
				bool buscarPokenestPorId(t_pokenest* pokenestParam) {
					return (pokenestParam->metadata.id == entrenador->pokemonD); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario
				}

				bool unaFuncionDeMierdaQueDevuelveTrue(t_pokemon* pokemon) {
					return TRUE;
				}

				pthread_mutex_lock(&listaDePokenestMutex);
				t_pokenest* pokenestEncontrada = list_find(listaDePokenest,
						(void*) buscarPokenestPorId);

				bool hayPokemones = list_size(
						pokenestEncontrada->listaDePokemones);

				if (hayPokemones) {
					t_pokemon* pokemon = list_remove_by_condition(
							pokenestEncontrada->listaDePokemones,
							(void*) unaFuncionDeMierdaQueDevuelveTrue); //saca al primero que encuentra

					pthread_mutex_unlock(&listaDePokenestMutex);

					pthread_mutex_lock(&setEntrenadoresMutex);
					list_add(entrenador->listaDePokemonesCapturados, pokemon);

					log_info(logMapa,
							"Trainer: '%c' capture the pokemon: '%s' SUCCESSFUL",
							entrenador->simbolo, pokemon->nombre);

					pthread_mutex_lock(&itemsMutex);
					restarRecurso(items, entrenador->pokemonD);
					nivel_gui_dibujar(items, mapa);
					pthread_mutex_unlock(&itemsMutex);

					entrenador->pokemonD = '/';
					entrenador->posD_x = -1;
					entrenador->posD_y = -1;
					entrenador->accion = SIN_MENSAJE;

					char* msjAEnviar = pokemon->nombre;
					sendClientMessage(&entrenador->socket, msjAEnviar,
							CAPTURADO);

					pthread_mutex_unlock(&setEntrenadoresMutex);

					//TODO agregar if teniendo en cuenta el algoritmo de planificacion

				}

				else {
					pthread_mutex_unlock(&listaDePokenestMutex);
					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenador->estaBloqueado = 1;
					log_info(logMapa,
							"Trainer: '%c' couldn't capture the pokemon: '%c'",
							entrenador->simbolo, entrenador->pokemonD);
					pthread_mutex_unlock(&setEntrenadoresMutex);
				}
				*i = metadataMapa.quantum; // le asignamos todo el quantum al contador asi sale
				estaEnAccion = 0;
				break;
			}

			case DESCONECTAR: { //por si se desconecta en medio del turno
				char simbolo = entrenador->simbolo; // guardamos el simbolo en esta variable para poder logearlo
				log_info(logMapa, "Deleting trainer: '%c'", simbolo);
				eliminarEntrenador(simbolo);
				log_info(logMapa, "Trainer: '%c' deleted SUCCESSFUL", simbolo);
				//TODO agregar if teniendo en cuenta el algoritmo de planificacion
				*i = metadataMapa.quantum; //asi lo deja de planificar.
				estaEnAccion = 0;
				break;
			}

			case ERROR: {
				*i = *i-1; // si llego mal el msj le restamos 1 al contador asi no pierde una accion en caso de que este en RR
				estaEnAccion = 0;
				break;
			}
			}
		}
	}
	//				} else pthread_mutex_unlock(&setEntrenadoresMutex);

}

void detectarDeadlocks() {
	while (1) {
		sleep(10);
		if (queue_size(colaDeBloqueados) > 1) { //si no hay mas de 1 bloqueado no hay deadlock.
			pthread_mutex_lock(&setEntrenadoresMutex);
			t_list* listaDeBloqueados = list_create();


			while(colaDeBloqueados){ //transformo la cola en una lista para poder manejarla mejor...
			t_entrenador* entrenadorBloqueado = queue_pop(colaDeBloqueados);
			list_add(listaDeBloqueados,entrenadorBloqueado);
			}

			int i;
			int a;
			int tamanioDeLaListaDeBloqueados = list_size(listaDeBloqueados);
			for (i = 0; i < tamanioDeLaListaDeBloqueados; i++) {

				t_entrenador* entrenador1 = list_get(listaDeBloqueados, i); // sacamos un entrenador

				t_queue* colaDeDeadlocks = queue_create();

				queue_push(colaDeDeadlocks,entrenador1); //lo ponemos tentativamente no se sabe si esta en deadlock realmente (por ahora).
				for (a = 0; a < tamanioDeLaListaDeBloqueados; a++) {
					t_entrenador* entrenador2 = list_get(listaDeBloqueados, a); // sacamos otro :)
					bool _tieneAlPokemonDeEntrenador1(t_pokemon* pokemonParam) {
						return pokemonParam->id == entrenador1->pokemonD;
					}

					bool _tieneAlPokemonDeEntrenador2(t_pokemon* pokemonParam) {
						return pokemonParam->id == entrenador2->pokemonD;
					}

					bool siNoSonIguales = entrenador1->simbolo
							!= entrenador2->simbolo;

					bool siElEntrenador2TieneUnPokemonQueQuiereEntrenador1 =
							list_any_satisfy(
									entrenador2->listaDePokemonesCapturados,
									(void*) _tieneAlPokemonDeEntrenador1);


					bool siElEntrenador1TieneUnPokemonQueQuiereEntrenador2 =
							list_any_satisfy(
									entrenador1->listaDePokemonesCapturados,
									(void*) _tieneAlPokemonDeEntrenador2);;

					if (siNoSonIguales
							&& siElEntrenador2TieneUnPokemonQueQuiereEntrenador1
							&& siElEntrenador1TieneUnPokemonQueQuiereEntrenador2) {

						log_info(logMapa,
								"The trainer '%c' is in deadlock with the trainer '%c'",
								entrenador1->simbolo, entrenador2->simbolo);
						queue_push(colaDeDeadlocks,entrenador2);

					}

				}

				if(queue_size(colaDeDeadlocks) >1){ //como lo habiamos puesto tentativamente verifacmos que sea mayor que >1
					//resolverDeadlocks(colaDeDeadlocks);
				}

			}

			list_destroy(listaDeBloqueados);
			pthread_mutex_unlock(&setEntrenadoresMutex);
		}

	}

}

void resolverDeadlocks(t_queue* colaDeDeadlocks){
	while(queue_size(colaDeDeadlocks) > 1){//cuando quede uno solo matamos a ese.
		t_entrenador* entrenador1 = queue_pop(colaDeDeadlocks);
		t_entrenador* entrenador2 = queue_pop(colaDeDeadlocks);
		t_pokemon* pokemon1 = dameTuMejorPokemon(entrenador1);
		t_pokemon* pokemon2 = dameTuMejorPokemon(entrenador2);

		t_pokemon* pokemonPerdedor;// = algortimoDeBatalla(pokemon1,pokemon2);

		if (pokemonPerdedor == pokemon1){
			queue_push(colaDeDeadlocks,entrenador2); // si perdio el entrenador 1 reencolamos al 2.
			queue_push(colaDeBloqueados,entrenador1); //y encolamos al 1 en la cola de bloqueados
		}

		else{
			queue_push(colaDeDeadlocks,entrenador1);
			queue_push(colaDeBloqueados,entrenador2);
		}

	}

	t_entrenador* entrenadorVictima = queue_pop(colaDeDeadlocks);
	matar(entrenadorVictima);

}

t_pokemon* dameTuMejorPokemon(t_entrenador* entrenador){
		bool _pokemonMayor(t_pokemon* pokemon1, t_pokemon* pokemon2) {
		return pokemon1->nivel > pokemon2->nivel;
		}

		list_sort(entrenador->listaDePokemonesCapturados,(void*)_pokemonMayor);
		t_pokemon* pokemonGroso = list_get(entrenador->listaDePokemonesCapturados,0); //como ya esta ordenada la lista el primer pokemon va a ser el mas poronga.
		return pokemonGroso;

}


void matar(t_entrenador* entrenador){
	sendClientMessage(&entrenador->socket, "cualquiercosa", MATAR); //le avisamos al entrenador que cago fuego

}


void sumarRecurso(t_list* items, char id) { //defino esta funcion porque no esta en la libreria
    ITEM_NIVEL* item = _search_item_by_id(items, id);

    if (item != NULL) {
        item->quantity = item->quantity + 1;
    } else {
        printf("WARN: Item %c no existente\n", id);
    }
}

void devolverPokemones(t_list* pokemones){
	int i;
		for(i=0;i<list_size(pokemones);i++){
			t_pokemon* pokemon = list_get(pokemones,i);
			bool _buscarPokenest(t_pokenest* pokenest){
				return pokenest->metadata.id == pokemon->id;
			}

			t_pokenest* pokenest = list_find(listaDePokenest,(void*) _buscarPokenest);

			list_add(pokenest->listaDePokemones, pokemon);

			pthread_mutex_lock(&itemsMutex);
			sumarRecurso(items,pokemon->id);
			nivel_gui_dibujar(items, mapa);
			pthread_mutex_unlock(&itemsMutex);
		}
}
