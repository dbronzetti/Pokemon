/*
 ============================================================================
 Name        : Mapa.c
 ============================================================================
 */

#include "Mapa_Refactor.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	mapa = string_new();
	char *pokedex = string_new();
	pthread_t serverThread;
	pthread_t detectorDeadlocks;
	pthread_t hiloSignal;
	pthread_t planificador;
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
	pthread_mutex_init(&metadataMutex, NULL);
	sem_init(&processNextMessageSem, 0, SEM_INIT_VALUE); //pshared = 0, then the semaphore is shared between the threads of a process
	sem_init(&borradoDePersonajesSem, 0, SEM_INIT_VALUE);

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

	logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	rutaMetadata = string_from_format("%s/Mapas/%s/metadata.dat", pokedex,
			mapa);
	char* rutaPokenest = string_from_format("%s/Mapas/%s/Pokenest/", pokedex,
			mapa);

	log_info(logMapa, "Directorio de la metadata del mapa '%s': '%s'\n", mapa,
			rutaMetadata);

	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa, logMapa);
	recorrerdirDePokenest(rutaPokenest);

	log_info(logMapa, "Bienvenido al mapa");

	sleep(2);
	system("clear"); //Espera 2 segundos y borra to_do

	dibujarMapa();

	pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);
	pthread_create(&planificador, NULL, (void*) planificar, NULL);
//	pthread_create(&detectorDeadlocks, NULL, (void*) detectarDeadlocks, NULL);
	pthread_create(&hiloSignal, NULL, (void*) recibirSignal, NULL);

	pthread_join(serverThread, NULL);
	pthread_join(planificador, NULL);
	pthread_join(hiloSignal, NULL);
//	pthread_join(detectorDeadlocks, NULL);

	pthread_mutex_destroy(&setEntrenadoresMutex);
	pthread_mutex_destroy(&colaDeListosMutex);
	pthread_mutex_destroy(&colaDeBloqueadosMutex);
	pthread_mutex_destroy(&setFDmutex);
	pthread_mutex_destroy(&itemsMutex);
	pthread_mutex_destroy(&listaDePokenestMutex);
	pthread_mutex_destroy(&setRecibirMsj);
	pthread_mutex_destroy(&metadataMutex);

	return 0;

}

//*************************************************************************//
//************************* Funciones Auxiliares **************************//
//*************************************************************************//

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

			} else { //estamos leyendo un archvo pokemon.
				char* rutaDelPokemon = string_from_format("%s/%s",
						rutaDeUnaPokenest, ditPokemones->d_name);

				t_pokemones* pokemon = malloc(sizeof(pokemon));
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

	void mapearIdYTipo(t_pokemones* pokemon) {
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
						pthread_mutex_lock(&setRecibirMsj);
						newClients(&socketServer, &master, &fdmax);
						pthread_mutex_unlock(&setRecibirMsj);

					} else {
						// we got some data from a client
						//Create thread attribute detached
//						pthread_attr_t processMessageThreadAttr;
//						pthread_attr_init(&processMessageThreadAttr);
//						pthread_attr_setdetachstate(&processMessageThreadAttr, PTHREAD_CREATE_DETACHED);

						//Create thread for checking new connections in server socket
						pthread_t processMessageThread;
						t_serverData *serverData = malloc(sizeof(t_serverData));
						memcpy(&serverData->socketServer, &socketServer,
								sizeof(serverData->socketServer));
						memcpy(&serverData->socketClient, &i,
								sizeof(serverData->socketClient));

						serverData->masterFD = &master;
						log_info(logMapa, "298");
						pthread_create(&processMessageThread, NULL,
								(void*) processMessageReceived, serverData);
						pthread_join(processMessageThread, NULL);
						//Destroy thread attribute
						log_info(logMapa, "302");
//						pthread_attr_destroy(&processMessageThreadAttr);
						log_info(logMapa, "304");
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

		serverData->masterFD = master;

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

		pthread_mutex_lock(&setFDmutex);
		FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
		pthread_mutex_unlock(&setFDmutex);

		close(serverData->socketClient);
		free(serverData);

	} else {

		switch ((int) message->process) {
		case ENTRENADOR: {
			log_info(logMapa, "Message from '%s': %s",
					getProcessString(message->process), message->message);
			log_info(logMapa, "Ha ingresado un nuevo ENTRENADOR");

			exitCode = sendClientAcceptation(&serverData->socketClient);

			sem_post(&processNextMessageSem); //enable message reception for new trainers

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

			pthread_mutex_lock(&setFDmutex);
			FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
			pthread_mutex_unlock(&setFDmutex);

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
	int receivedBytes = receiveMessage(&serverData->socketClient, &messageSize,
			sizeof(messageSize));

	bool buscarPorSocket(t_entrenador* entrenador) {
		if ((entrenador != NULL && entrenador->socket != NULL)
				&& (serverData != NULL && serverData->socketClient != NULL)) {
			return (entrenador->socket == serverData->socketClient);
		} else {
			printf("Fallo variable NULL _funcBuscarPokemon\n");
			printf("variable 1 %d\n",
					(entrenador != NULL && entrenador->socket != NULL));
			printf("variable 2 %d\n",
					(serverData != NULL && serverData->socketClient != NULL));
		}
		return false;
	}

	if (receivedBytes <= 0) {
		// got error or connection closed by client
		if (receivedBytes == 0) {
			// connection closed
			pthread_mutex_lock(&setEntrenadoresMutex);
			int existeElEntrenador = list_any_satisfy(listaDeEntrenadores,
					(void*) buscarPorSocket);
			pthread_mutex_unlock(&setEntrenadoresMutex);

			if (existeElEntrenador) {

				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				int enTurno = entrenador->estaEnTurno;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				if (enTurno == 0) { //si no esta en turno lo borramos normal
					char simboloDelEntrenador = entrenador->simbolo;

					log_error(logMapa,
							"Trainer: '%c' wants to  disconnect in socket: %d",
							simboloDelEntrenador, serverData->socketClient);
					log_error(logMapa, "Deleting trainer: '%c'",
							simboloDelEntrenador);

					eliminarEntrenador(simboloDelEntrenador);

					log_error(logMapa, "Trainer: '%c' deleted SUCCESSFUL",
							simboloDelEntrenador);

					int socketFD = serverData->socketClient;

					pthread_mutex_lock(&setFDmutex);
					FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
					pthread_mutex_unlock(&setFDmutex);

					close(serverData->socketClient); // bye!

					log_error(logMapa, "Socket: %d  disconected SUCCESSFUL",
							socketFD);

				} else {

					sem_wait(&borradoDePersonajesSem);
					log_error(logMapa,
							"Trainer '%c' wants to disconnect in socket: '%d' BUT IS PLAYING!!",
							entrenador->simbolo, entrenador->socket);

					//this wait is because the trainer has to be disconnected
					sem_post(&processNextMessageSem); //enable message processing

					pthread_mutex_lock(&setEntrenadoresMutex);
					int socketFD = entrenador->socket;
					entrenador->accion = DESCONECTAR;
					entrenador->socket = -1;
					entrenador->pokemonD = '0';
					pthread_mutex_unlock(&setEntrenadoresMutex);

					pthread_mutex_lock(&setFDmutex);
					FD_CLR(serverData->socketClient, serverData->masterFD); // remove from master set
					pthread_mutex_unlock(&setFDmutex);

					close(serverData->socketClient); // bye!

					log_error(logMapa, "Socket: %d  disconected SUCCESSFUL",
							socketFD);
				}
			}

		}

	} else {
		//Receive message using the size read before
		char *messageRcv = malloc(messageSize);
		receivedBytes = receiveMessage(&serverData->socketClient, messageRcv,
				messageSize);

		//starting handshake with client connected
		t_Mensaje *message = malloc(sizeof(t_Mensaje));
		deserializeClientMessage(message, messageRcv);

		//Now it's checked that the client is not down
		if (receivedBytes > 0) {

			sem_wait(&processNextMessageSem); //enable message processing

			switch (message->tipo) {
			case NUEVO: {
				log_info(logMapa, "Creating new trainer: %s", message->mensaje);
				crearEntrenadorYDibujar(message->mensaje[0],
						serverData->socketClient);
				log_info(logMapa, "Trainer:%s created SUCCESSFUL",
						message->mensaje);
				break;
			}
			case CONOCER: {
				char id_pokemon = message->mensaje[0];

				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->pokemonD = id_pokemon;
				entrenador->accion = CONOCER;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa,
						"Trainer: '%c' want to know the position of: '%s'",
						entrenador->simbolo, message->mensaje);

				break;
			}
			case IR: {
				log_info(logMapa, "526");
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				log_info(logMapa, "527");
				entrenador->accion = IR;
				log_info(logMapa, "528");
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa, "Trainer want: '%c' to go to: '%s'",
						entrenador->simbolo, message->mensaje);

				break;
			}
			case CAPTURAR: {
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->accion = CAPTURAR;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa, "Trainer: '%c' want to capture: '%s'",
						entrenador->simbolo, message->mensaje);

				break;
			}
			default: {
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenador = list_find(listaDeEntrenadores,
						(void*) buscarPorSocket);
				entrenador->accion = ERROR;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_error(logMapa,
						"Message from the trainer %c came wrong: message: %c type: %d",
						entrenador->simbolo, message->mensaje, message->tipo);

				break;
			}
			}
		}

		free(message->mensaje);
		free(message);
		free(messageRcv);
	}

	log_info(logMapa, "fin de la funcion");
}

void crearEntrenadorYDibujar(char simbolo, int socket) {
	t_entrenador* nuevoEntrenador = malloc(sizeof(t_entrenador));

	nuevoEntrenador->simbolo = simbolo;
	nuevoEntrenador->timeIngreso = time(0);
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
	nuevoEntrenador->estaEnTurno = 0;
	nuevoEntrenador->tiempoBloqueado = 0;
	nuevoEntrenador->cantDeadLock = 0;

	//In function CrearPersonaje there is a list_add to items
	pthread_mutex_lock(&itemsMutex);
	CrearPersonaje(items, simbolo, nuevoEntrenador->pos_x,
			nuevoEntrenador->pos_y);
	pthread_mutex_unlock(&itemsMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);
	list_add(listaDeEntrenadores, nuevoEntrenador);
	pthread_mutex_unlock(&setEntrenadoresMutex);

	pthread_mutex_lock(&itemsMutex);
	nivel_gui_dibujar(items, mapa);
	pthread_mutex_unlock(&itemsMutex);

	pthread_mutex_lock(&colaDeListosMutex);
	queue_push(colaDeListos, nuevoEntrenador);
	pthread_mutex_unlock(&colaDeListosMutex);
}

void eliminarEntrenador(char simbolo) {
	log_info(logMapa, "The trainer '%c' is going to be eliminated", simbolo);

	pthread_mutex_lock(&itemsMutex);
	BorrarItem(items, simbolo);
	nivel_gui_dibujar(items, mapa);
	pthread_mutex_unlock(&itemsMutex);

	bool igualarACaracterCondicion(t_entrenador *entrenador) {
		return simbolo == entrenador->simbolo;
	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	t_entrenador* entrenador = list_remove_by_condition(listaDeEntrenadores,
			(void*) igualarACaracterCondicion);

	// Al Sacarlo de la lista debemos calcular tiempos en cola.
	saleColaBloqueados(entrenador);

	t_list* pokemones = entrenador->listaDePokemonesCapturados;
	pthread_mutex_unlock(&setEntrenadoresMutex);

	devolverPokemones(pokemones);

	//After returning all pokemons to Map it's needed to reset all the trainers
	//in order to let them capture the new pokemons available in case it is possible
	pthread_mutex_lock(&colaDeBloqueadosMutex);
	while (queue_size(colaDeBloqueados) > 0) {//se limpia la cola de bloqueados completamente
		t_entrenador* entrenadorBloqueado = queue_pop(colaDeBloqueados);

		pthread_mutex_lock(&setEntrenadoresMutex);
		entrenadorBloqueado->estaBloqueado = 0;
		entrenador->accion = SIN_MENSAJE;
		saleColaBloqueados(entrenadorBloqueado);
		pthread_mutex_unlock(&setEntrenadoresMutex);

		pthread_mutex_lock(&colaDeListosMutex);
		queue_push(colaDeListos, entrenadorBloqueado);
		pthread_mutex_unlock(&colaDeListosMutex);
	}
	pthread_mutex_unlock(&colaDeBloqueadosMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);
	free(entrenador);
	pthread_mutex_unlock(&setEntrenadoresMutex);

}

void devolverPokemones(t_list* pokemones) {
	int i;
	for (i = 0; i < list_size(pokemones); i++) {
		t_pokemones* pokemon = list_get(pokemones, i);

		bool _buscarPokenest(t_pokenest* pokenest) {
			if ((pokenest != NULL && pokenest->metadata.id != NULL)
					&& (pokemon != NULL && pokemon->id != NULL)) {
				return pokenest->metadata.id == pokemon->id;
			} else {
				printf("Fallo variable NULL _funcBuscarPokemon\n");
				printf("variable 1 %d\n",
						(pokenest != NULL && pokenest->metadata.id != NULL));
				printf("variable 2 %d\n",
						(pokemon != NULL && pokemon->id != NULL));
			}
			return false;

		}

		pthread_mutex_lock(&listaDePokenestMutex);
		t_pokenest* pokenest = list_find(listaDePokenest,
				(void*) _buscarPokenest);
		list_add(pokenest->listaDePokemones, pokemon);
		pthread_mutex_unlock(&listaDePokenestMutex);

		pthread_mutex_lock(&itemsMutex);
		sumarRecurso(items, pokemon->id);
		nivel_gui_dibujar(items, mapa);
		pthread_mutex_unlock(&itemsMutex);
	}

	list_destroy(pokemones);//destruyo unicamente la lista, NO los elementos ya que esos se utilizan en a lista de pokenest
}

void sumarRecurso(t_list* items, char id) { //defino esta funcion porque no esta en la libreria
	ITEM_NIVEL* item = _search_item_by_id(items, id);

	if (item != NULL) {
		item->quantity = item->quantity + 1;
	} else {
		printf("WARN: Item %c no existente\n", id);
	}
}

void planificar() {
	char *prevPlanificador = "";
	char *planificador;
	while (1) {
		pthread_mutex_lock(&metadataMutex);
		planificador = metadataMapa.algoritmo;
		pthread_mutex_unlock(&metadataMutex);

		if (strcmp(planificador, prevPlanificador) != 0) {
			log_info(logMapa, "Planificador %s", planificador);
			//TODO informar estado de las colas
		}

		if (strcmp(planificador, "RR") == 0) {
			planificarRR();
		} else {
			planificarSRDF();
		}

		prevPlanificador = planificador;

	}
}

void planificarRR() {

	pthread_mutex_lock(&colaDeListosMutex);
	int tamanioColaListos = queue_size(colaDeListos);
	pthread_mutex_unlock(&colaDeListosMutex);

	if (tamanioColaListos > 0) {

		pthread_mutex_lock(&colaDeListosMutex);
		t_entrenador* entrenador = queue_pop(colaDeListos);
		pthread_mutex_unlock(&colaDeListosMutex);

		if (noSeBorro(entrenador)) { //hacemos un if por si sacamos un entrenador que se desconecto
			pthread_mutex_lock(&setEntrenadoresMutex);
			entrenador->estaEnTurno = 1;
			pthread_mutex_unlock(&setEntrenadoresMutex);

			log_info(logMapa, "Begins the turn of trainer: %c",
					entrenador->simbolo);

			pthread_mutex_lock(&metadataMutex);
			int quantum = metadataMapa.quantum;
			int retardoTurno = (metadataMapa.retardo * 1000);
			pthread_mutex_unlock(&metadataMutex);

			usleep(retardoTurno); //Retardo entre turnos antes de asignar entrenador

			int i = 0;
			while (i < quantum) {

				log_info(logMapa, "Quantum #%d of trainer : '%c'", i,
						entrenador->simbolo);

				ejecutarAccionEntrenador(entrenador, &i);

				i++;
			}

			if (noSeBorro(entrenador)) {

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
					entrenador->timeIngresoBloq = time(0);
					queue_push(colaDeBloqueados, entrenador); //y aca lo mandamos a la cola de bloqueados.
					pthread_mutex_unlock(&colaDeBloqueadosMutex);
				}
			}
		}
	}
}

void ejecutarAccionEntrenador(t_entrenador* entrenador, int* quantum) {

	int estaEnAccion = 1;

//	pthread_mutex_lock(&setEntrenadoresMutex);
//	if (entrenador->posD_x != -1) {
//		if ((entrenador->posD_x == entrenador->pos_x) && (entrenador->posD_y == entrenador->pos_y)) { //si se encuentra en la posicion deseada le avisamos que llego y asi comienza su movimiento
//			log_info(logMapa, "Trainer: '%c' came to the pokenest", entrenador->simbolo);
//			entrenador->accion = LLEGO;
//		}else if (entrenador->seEstaMoviendo) { // le pedimos que se mueva
//			log_info(logMapa, "Trainer: '%c' has not yet reached his position",entrenador->simbolo);
//			entrenador->accion = MOVETE;
//		}
//	}
//	pthread_mutex_unlock(&setEntrenadoresMutex);

	evaluarEstadoEntrenador(entrenador);
	time_t tiempo1 = time(0);
	log_info(logMapa, "809");
	while (estaEnAccion) { //una accion que puede llevar acabo el usuario dentro del turno

		time_t tiempo2 = time(0);
		double segsSinResponder = difftime(tiempo2, tiempo1);

		if (segsSinResponder > 10) {
			log_info(logMapa,
					"Trainer '%c' doesn't respond, will be disconnected from %s",
					entrenador->simbolo, mapa);
			pthread_mutex_lock(&setEntrenadoresMutex);
			entrenador->accion = DESCONECTAR;
			pthread_mutex_unlock(&setEntrenadoresMutex);
		}

		pthread_mutex_lock(&setEntrenadoresMutex);
		int siNoTieneMensaje = entrenador->accion != SIN_MENSAJE;
//		log_info(logMapa,"sin msj");
		pthread_mutex_unlock(&setEntrenadoresMutex);

		if (siNoTieneMensaje) { //este if verifica que el entrenador respondio :D

			pthread_mutex_lock(&setEntrenadoresMutex);
			int accionDelEntrenador = entrenador->accion;
			pthread_mutex_unlock(&setEntrenadoresMutex);

			switch (accionDelEntrenador) {
//				case NUEVO: {
//					pthread_mutex_lock(&setEntrenadoresMutex);
//					entrenador->accion = SIN_MENSAJE;
//					sendClientMessage(&entrenador->socket, "LIBRE!", LIBRE);
//					pthread_mutex_unlock(&setEntrenadoresMutex);
//
//					sem_post(&processNextMessageSem);//enable message reception
//
//					estaEnAccion = 0;
//					break;
//				}

			case CONOCER: {
				pthread_mutex_lock(&setEntrenadoresMutex);
				char idPokemon = entrenador->pokemonD;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				bool buscarPokenestPorId1(t_pokenest* pokenestParam) {
					if ((pokenestParam != NULL
							&& pokenestParam->metadata.id != NULL)
							&& (idPokemon != NULL)) {
						return (pokenestParam->metadata.id == idPokemon); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario
					} else {
						printf("Fallo variable NULL buscarPokenestPorId1\n");
						printf("variable 1 %d\n",
								(pokenestParam != NULL
										&& pokenestParam->metadata.id != NULL));
						printf("variable 2 %d\n", (idPokemon != NULL));
					}
					return false;

				}

				pthread_mutex_lock(&listaDePokenestMutex);
				bool existeLaPokenest = existePokenest(idPokemon);
				pthread_mutex_unlock(&listaDePokenestMutex);

				if (existeLaPokenest) {
					pthread_mutex_lock(&listaDePokenestMutex);
					t_pokenest* pokenestEncontrada = list_find(listaDePokenest,
							(void*) buscarPokenestPorId1);
					int posX = pokenestEncontrada->metadata.pos_x;
					int posY = pokenestEncontrada->metadata.pos_y;
					pthread_mutex_unlock(&listaDePokenestMutex);

					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenador->posD_x = posX;
					entrenador->posD_y = posY;
					entrenador->accion = SIN_MENSAJE;
					entrenador->seEstaMoviendo = 1;
					char* mensajeAEnviar = convertirPosicionesAString(posX,
							posY);
					sendClientMessage(&entrenador->socket, mensajeAEnviar,
							CONOCER);
					pthread_mutex_unlock(&setEntrenadoresMutex);

					log_info(logMapa,
							"Map send the position to the trainer: '%c'",
							entrenador->simbolo);

				} else {
					sendClientMessage(&entrenador->socket,
							"The id of pokemon came wrong", ERROR_CONOCER);
				}

				sem_post(&processNextMessageSem); //enable message reception

				estaEnAccion = 0;
				break;
			}
//				case MOVETE: {
//
//					pthread_mutex_lock(&setEntrenadoresMutex);
//					entrenador->accion = SIN_MENSAJE;
//					sendClientMessage(&entrenador->socket, "MOVETE!", MOVETE);
//					pthread_mutex_unlock(&setEntrenadoresMutex);
//
//					sem_post(&processNextMessageSem);//enable message reception
//
//					estaEnAccion = 0;
//					break;
//				}
			case IR: {

				pthread_mutex_lock(&setEntrenadoresMutex);
				moverEntrenador(&entrenador->pos_x, &entrenador->pos_y,
						entrenador->posD_x, entrenador->posD_y,
						&entrenador->seMovioEnX); //mueve el entrenador de a 1 til.
				pthread_mutex_unlock(&setEntrenadoresMutex);

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
//				case LLEGO: {
//					pthread_mutex_lock(&setEntrenadoresMutex);
//					entrenador->seEstaMoviendo = 0; // si ya llego a la posicion no se esta moviendo mas
//					entrenador->accion = SIN_MENSAJE;
//					sendClientMessage(&entrenador->socket, "LLEGO!", LLEGO);
//					pthread_mutex_unlock(&setEntrenadoresMutex);
//
//					sem_post(&processNextMessageSem);//enable message reception
//
//					estaEnAccion = 0;
//					break;
//				}
			case CAPTURAR: {
				bool buscarPokenestPorId(t_pokenest* pokenestParam) {
					if ((pokenestParam != NULL
							&& pokenestParam->metadata.id != NULL)
							&& (entrenador != NULL
									&& entrenador->pokemonD != NULL)) {
						return (pokenestParam->metadata.id
								== entrenador->pokemonD); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario
					} else {
						printf("Fallo variable NULL buscarPokenestPorId1\n");
						printf("variable 1 %d\n",
								(pokenestParam != NULL
										&& pokenestParam->metadata.id != NULL));
						printf("variable 2 %d\n",
								(entrenador != NULL
										&& entrenador->pokemonD != NULL));
					}
					return false;
				}

				pthread_mutex_lock(&listaDePokenestMutex);
				t_pokenest* pokenestEncontrada = list_find(listaDePokenest,
						(void*) buscarPokenestPorId);
				int cantidadPokemones = list_size(
						pokenestEncontrada->listaDePokemones);
				pthread_mutex_unlock(&listaDePokenestMutex);

				if (cantidadPokemones > 0) {
					pthread_mutex_lock(&listaDePokenestMutex);
					t_pokemones* pokemon = list_remove(
							pokenestEncontrada->listaDePokemones, 0); //saca al primero que encuentra
					pthread_mutex_unlock(&listaDePokenestMutex);

					log_info(logMapa,
							"Trainer: '%c' capture the pokemon: '%s' SUCCESSFUL",
							entrenador->simbolo, pokemon->nombre);

					pthread_mutex_lock(&setEntrenadoresMutex);
					list_add(entrenador->listaDePokemonesCapturados, pokemon);

					pthread_mutex_lock(&itemsMutex);
					restarRecurso(items, entrenador->pokemonD);
					nivel_gui_dibujar(items, mapa);
					pthread_mutex_unlock(&itemsMutex);
					entrenador->pokemonD = '/';
					entrenador->posD_x = -1;
					entrenador->posD_y = -1;
					entrenador->accion = SIN_MENSAJE;
					; // se lo configura como libre para que el proximo turno solicite nuevo pokemon al entrenador
					char* msjAEnviar = pokemon->nombre;
					sendClientMessage(&entrenador->socket, msjAEnviar,
							CAPTURADO);
					pthread_mutex_unlock(&setEntrenadoresMutex);

					sem_post(&processNextMessageSem); //enable message reception

				} else {
					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenador->estaBloqueado = 1;
					entrenador->accion = SIN_MENSAJE;
					pthread_mutex_unlock(&setEntrenadoresMutex);

					log_info(logMapa,
							"Trainer: '%c' couldn't capture the pokemon: '%c'",
							entrenador->simbolo, entrenador->pokemonD);
				}

				pthread_mutex_lock(&metadataMutex);
				*quantum = metadataMapa.quantum; // le asignamos to_do el quantum al contador asi sale
				pthread_mutex_unlock(&metadataMutex);
				estaEnAccion = 0;

				break;
			}
//				case LIBRE: {
//					pthread_mutex_lock(&setEntrenadoresMutex);
//					entrenador->accion = SIN_MENSAJE;
//					sendClientMessage(&entrenador->socket, "LIBRE!", LIBRE);
//					pthread_mutex_unlock(&setEntrenadoresMutex);
//
//					sem_post(&processNextMessageSem);//enable message reception
//
//					estaEnAccion = 0;
//					break;
//				}
			case DESCONECTAR: { //por si se desconecta en medio del turno

				pthread_mutex_lock(&setEntrenadoresMutex);
				char simbolo = entrenador->simbolo; // guardamos el simbolo en esta variable para poder logearlo
				pthread_mutex_unlock(&setEntrenadoresMutex);

				log_info(logMapa, "Deleting trainer: '%c'", simbolo);

				eliminarEntrenador(simbolo);

				log_info(logMapa, "Trainer: '%c' deleted SUCCESSFUL", simbolo);

				pthread_mutex_lock(&metadataMutex);
				*quantum = metadataMapa.quantum; //asi lo deja de planificar.
				pthread_mutex_unlock(&metadataMutex);
				sem_post(&borradoDePersonajesSem);
				estaEnAccion = 0;
				break;
			}

			case ERROR: {
				pthread_mutex_lock(&metadataMutex);
				*quantum = *quantum - 1; // si llego mal el msj le restamos 1 al contador asi no pierde una accion en caso de que este en RR
				pthread_mutex_unlock(&metadataMutex);

				pthread_mutex_lock(&setEntrenadoresMutex);
				entrenador->accion = SIN_MENSAJE;
				pthread_mutex_unlock(&setEntrenadoresMutex);

				estaEnAccion = 0;
				break;
			}

			} // switch (accionDelEntrenador)
		} // if (siNoTieneMensaje)
	} // while (estaEnAccion)
}

bool existePokenest(char idPokemon) {

	bool buscarPokenestPorId1(t_pokenest* pokenestParam) {
		if ((pokenestParam != NULL && pokenestParam->metadata.id != NULL)
				&& (idPokemon != NULL)) {
			return (pokenestParam->metadata.id == idPokemon); //comparo si el identificador del pokemon es igual al pokemon que desea el usuario		}
		} else {
			printf("Fallo variable NULL \n");
			printf("variable 1 %d\n", pokenestParam->metadata.id != NULL);
			printf("variable 2 %d\n", idPokemon != NULL);
		}
		return false;
	}

	return list_any_satisfy(listaDePokenest, (void*) buscarPokenestPorId1);

}

char* convertirPosicionesAString(int posX, int posY) {
	char* posXstr = string_itoa(posX);
	char* posYstr = string_itoa(posY);

	return string_from_format("%s,%s", posXstr, posYstr);
}

void moverEntrenador(int* pos_x, int* pos_y, int posD_x, int posD_y,
		int* seMovioEnX) { //le puse un millon de parametros para poder usarla despues como contador de distancia
	if (*seMovioEnX) { //si ya se movio en x

		if (*pos_y == posD_y) { //y ademas esta paralelo (en y) a su pokenest
			if (*pos_x > posD_x) { //si esta por arriba le restamos uno.
				*pos_x = *pos_x - 1;     // se vuelve a mover en x nomas.
			} else {
				*pos_x = *pos_x + 1; //si esta por abajo le sumamos uno
			}

		} else {   //sino lo movemos en Y.
			if (*pos_y > posD_y) { //si esta por arriba le restamos uno
				*pos_y = *pos_y - 1;
			} else {
				*pos_y = *pos_y + 1;
			}

			*seMovioEnX = 0; //y le seteamos el flag en 0.
		}

	} else {

		if (*pos_x == posD_x) { //si  se movio en X pero esta paralelo (en y) a su pokenest
			if (*pos_y > posD_y) { //si esta por arriba le restamos uno.
				*pos_y = *pos_y - 1;     // se vuelve a mover en y nomas.
			} else {
				*pos_y = *pos_y + 1; //si esta por abajo le sumamos uno
			}

		} else { //sino lo movemos en X

			if (*pos_x > posD_x) { //si esta por arriba le restamos uno
				*pos_x = *pos_x - 1;
			} else {
				*pos_x = *pos_x + 1;
			}

			*seMovioEnX = 1; //y le seteamos el flag en 1.
		} // if (*pos_x == posD_x)
	} // if (*seMovioEnX)

}

void planificarSRDF() {
	pthread_mutex_lock(&colaDeListosMutex);
	int tamanioColaListos = queue_size(colaDeListos);
	pthread_mutex_unlock(&colaDeListosMutex);

	if (tamanioColaListos) {

		ordenarColaEntrenadores();

		pthread_mutex_lock(&colaDeListosMutex);
		t_entrenador* entrenador = queue_pop(colaDeListos);
		pthread_mutex_unlock(&colaDeListosMutex);

		pthread_mutex_lock(&setEntrenadoresMutex);
		entrenador->estaEnTurno = 1;
		pthread_mutex_unlock(&setEntrenadoresMutex);

		pthread_mutex_lock(&metadataMutex);
		int retardoTurno = (metadataMapa.retardo * 1000);
		pthread_mutex_unlock(&metadataMutex);
		if (noSeBorro(entrenador)) {

			pthread_mutex_lock(&setEntrenadoresMutex);
			entrenador->estaEnTurno = 1;
			if (entrenadorAnterior != entrenador->simbolo) //si cambio de entrenador => se corto su rafaga => se retarda
				usleep(retardoTurno);

			pthread_mutex_unlock(&setEntrenadoresMutex);
			int a = 0;
			ejecutarAccionEntrenador(entrenador, &a); //0 it's not needed for this planificador

		}
		if (noSeBorro(entrenador)) {

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
				entrenador->timeIngresoBloq = time(0); // Empezamos a contar el tiempo en la cola.
				queue_push(colaDeBloqueados, entrenador); //y aca lo mandamos a la cola de bloqueados.
				pthread_mutex_unlock(&colaDeBloqueadosMutex);
			}
			entrenadorAnterior = entrenador->simbolo;
		}

	}
}

void ordenarColaEntrenadores() {
	t_list* listAuxOrdenar = list_create();
	int i;
	char simbolo;

	//obtenemos todos los entrenadores y determinamos su distancia.
	pthread_mutex_lock(&colaDeListosMutex);
	int tamanioColaListos = queue_size(colaDeListos);
	pthread_mutex_unlock(&colaDeListosMutex);

	while (tamanioColaListos) {

		pthread_mutex_lock(&colaDeListosMutex);
		t_entrenador* entrenadorAux = queue_pop(colaDeListos);
		pthread_mutex_unlock(&colaDeListosMutex);

		if (noSeBorro(entrenadorAux)) {
			entrenadorAux->estaEnTurno = 1;
			calcularCantidadMovimientos(entrenadorAux);
			entrenadorAux->estaEnTurno = 0;
			pthread_mutex_lock(&setEntrenadoresMutex);
			list_add(listAuxOrdenar, entrenadorAux);
			pthread_mutex_unlock(&setEntrenadoresMutex);

		}
		tamanioColaListos--; //le resto uno al tamanio para que no quede en bucle infinito
	}

	bool entrenador_menor(t_entrenador *entrenadorA, t_entrenador *entrenadorB) {
		return entrenadorA->distancia < entrenadorB->distancia;
	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	list_sort(listAuxOrdenar, (void*) entrenador_menor);
	pthread_mutex_unlock(&setEntrenadoresMutex);

	//Volvemos a regenerar la Cola con los entrenadores ya ordenados.
	pthread_mutex_lock(&setEntrenadoresMutex);
	for (i = 0; i < list_size(listAuxOrdenar); i++) {
		t_entrenador* entrenadorAux = list_get(listAuxOrdenar, i);
		pthread_mutex_lock(&colaDeListosMutex);
		queue_push(colaDeListos, entrenadorAux);
		pthread_mutex_unlock(&colaDeListosMutex);
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

	list_destroy(listAuxOrdenar);

}

void calcularCantidadMovimientos(t_entrenador* entrenador) {

	//Si el PokemonD esta con este caracter / significa que es un nuevo entrenador o ya capturo a su Pokemon
	pthread_mutex_lock(&setEntrenadoresMutex);
	char pokemonDeseado = entrenador->pokemonD;
	pthread_mutex_unlock(&setEntrenadoresMutex);

	if (pokemonDeseado == '/') {
		int quatum = 0; // the quantum it's not needed
		ejecutarAccionEntrenador(entrenador, &quatum);
	}

	if(noSeBorro(entrenador)){

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
		//log_info(logMapa,"[DEBUG] pos en x:%d , pos deseada x: %d , pos en y:%d , pos deseada en y: %d", pos_x,posD_x,pos_y,posD_y);
		distancia++;
	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	entrenador->distancia = distancia;
	pthread_mutex_unlock(&setEntrenadoresMutex);

	log_info(logMapa, "Trainer: '%c' it's about %d actions of his pokenest",
			entrenador->simbolo, entrenador->distancia);
	}
}

void detectarDeadlocks() {

	while (1) {

		pthread_mutex_lock(&metadataMutex);
		int sleepDeadlock = metadataMapa.tiempoChequeoDeadlock;
		pthread_mutex_unlock(&metadataMutex);

		sleep(sleepDeadlock / 1000);

		log_info(logMapa, "Checking DEADLOCKS.....");

		pthread_mutex_lock(&colaDeBloqueadosMutex);
		int tamanioColaBloqueados = queue_size(colaDeBloqueados);
		pthread_mutex_unlock(&colaDeBloqueadosMutex);

		log_info(logMapa, "tamanioColaBloqueados %d", tamanioColaBloqueados);

		if (tamanioColaBloqueados > 1) { //si no hay mas de 1 bloqueado no hay deadlock.

			t_list* listaDeadlock = list_create();
			t_list* entrenadoresBloqueados = detectarInterbloqueo();

			pthread_mutex_lock(&colaDeBloqueadosMutex);
			while (queue_size(colaDeBloqueados) > 0) { //transformo la cola en una lista para poder manejarla mejor...
				//se limpia la cola de bloqueados completamente
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenadorBloqueado = queue_pop(colaDeBloqueados);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				bool _funcBuscarEntrenador(char *entrenador) {
					if ((entrenadorBloqueado != NULL
							&& entrenadorBloqueado->simbolo != NULL)
							&& (*entrenador != NULL)) {
						return *entrenador == entrenadorBloqueado->simbolo;
					} else {
						printf("Fallo variable NULL funcBuscarEntrenador\n");
						printf("variable 1 %d\n",
								entrenadorBloqueado->simbolo != NULL);
						printf("variable 2 %d\n", *entrenador != NULL);
					}
					return false;
				}

				// ME fijo si ese entrenador esta en mi lista de DeadLock
				pthread_mutex_lock(&setEntrenadoresMutex);
				char * entrenadorFlag = list_find(entrenadoresBloqueados,
						(void *) _funcBuscarEntrenador);
				pthread_mutex_unlock(&setEntrenadoresMutex);

				if (entrenadorFlag != NULL) {
					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenadorBloqueado->cantDeadLock++;
					list_add(listaDeadlock, entrenadorBloqueado);
					pthread_mutex_unlock(&setEntrenadoresMutex);
				} else {
					//Los devuelvo a la cola de Listo. TODOS los entrenadores no DEADLOCK vuelven a competir
					pthread_mutex_lock(&setEntrenadoresMutex);
					entrenadorBloqueado->estaBloqueado = 0;
					saleColaBloqueados(entrenadorBloqueado);
					pthread_mutex_unlock(&setEntrenadoresMutex);

					pthread_mutex_lock(&colaDeListosMutex);
					queue_push(colaDeListos, entrenadorBloqueado);
					pthread_mutex_unlock(&colaDeListosMutex);
				}
			}
			pthread_mutex_unlock(&colaDeBloqueadosMutex);

			//Ordenamos la lista segun acceso al Mapa.
			bool _entrenadorMasViejoEnMapa(t_entrenador* entrenador1,
					t_entrenador* entrenador2) {
				double tiempoDiferencia = difftime(entrenador1->timeIngreso,
						entrenador2->timeIngreso);

				if (tiempoDiferencia < 0) {
					return true;
				} else {
					return false;
				}
			}

			pthread_mutex_lock(&setEntrenadoresMutex);
			list_sort(listaDeadlock, (void*) _entrenadorMasViejoEnMapa);
			pthread_mutex_unlock(&setEntrenadoresMutex);

			pthread_mutex_lock(&metadataMutex);
			int modoBatalla = metadataMapa.batalla;
			pthread_mutex_unlock(&metadataMutex);

			if (modoBatalla == 1) { //batallas activadas

				//1) mandar pokemon 1 y 2 a pelear. El ganador debera pasar a la cola de listo y su flag "estaBloqueado" = 0.
				bool enBatalla = true;
				while (enBatalla) {

					//me voy a asegurar que siempre haya por lo menos 2 entrenadores para hacer pelear
					if (list_size(listaDeadlock) >= 2) {
						pthread_mutex_lock(&setEntrenadoresMutex);
						t_entrenador* entrenador1 = list_get(listaDeadlock, 0);	//saco primer entrenador
						t_entrenador* entrenador2 = list_get(listaDeadlock, 1);	//saco el segundo entrenador

						//obtengo sus pokemones mas fuertes
						t_pokemones* pokemon1 = dameTuMejorPokemon(entrenador1);
						t_pokemones* pokemon2 = dameTuMejorPokemon(entrenador2);
						pthread_mutex_unlock(&setEntrenadoresMutex);

						//Creamos una instancia de la Factory
						//La misma sirve para crear pokémons con solo el nombre y el nivel
						t_pkmn_factory* pokemon_factory = create_pkmn_factory();

						//Nótese que empieza con letra mayúscula y no debe tener errores de nombre
						t_pokemon * pokemonParaBatalla1 = create_pokemon(
								pokemon_factory, pokemon1->nombre,
								pokemon1->nivel);
						t_pokemon * pokemonParaBatalla2 = create_pokemon(
								pokemon_factory, pokemon2->nombre,
								pokemon2->nivel);

						log_info(logMapa, "\n========Batalla!========\n");
						log_info(logMapa,
								"Pokemon del entrenador '%c': %s[%s/%s] Nivel: %d",
								entrenador1->simbolo,
								pokemonParaBatalla1->species,
								pkmn_type_to_string(pokemonParaBatalla1->type),
								pkmn_type_to_string(
										pokemonParaBatalla1->second_type),
								pokemonParaBatalla1->level);
						log_info(logMapa,
								"Pokemon del entrenador '%c': %s[%s/%s] Nivel: %d",
								entrenador2->simbolo,
								pokemonParaBatalla2->species,
								pkmn_type_to_string(pokemonParaBatalla2->type),
								pkmn_type_to_string(
										pokemonParaBatalla2->second_type),
								pokemonParaBatalla2->level); //Función que sirve para ver el Tipo de Enum como un String

						//La batalla propiamente dicha
						t_pokemon * loser = pkmn_battle(pokemonParaBatalla1,
								pokemonParaBatalla2);

						//uso un puntero para identificar el ganador y luego trabajar sobre ese
						t_entrenador* entrenadorGanador;
						t_pokemones* pokemonGanador;
						if (string_equals_ignore_case(loser->species,
								pokemon1->nombre)) { //comparo el pokemon que perdio contra el pokemon de entrenador 1 obtenido antes
							pthread_mutex_lock(&setEntrenadoresMutex);
							entrenadorGanador = entrenador2;
							pthread_mutex_unlock(&setEntrenadoresMutex);
							pokemonGanador = pokemon2;
							log_info(logMapa,
									"El entrenador PERDEDOR fue '%c' con el pokemon '%s'",
									entrenador1->simbolo, loser->species);
						} else {
							pthread_mutex_lock(&setEntrenadoresMutex);
							entrenadorGanador = entrenador1;
							pthread_mutex_unlock(&setEntrenadoresMutex);
							pokemonGanador = pokemon1;
							log_info(logMapa,
									"El entrenador PERDEDOR fue '%c' con el pokemon '%s'",
									entrenador2->simbolo, loser->species);
						}

						log_info(logMapa,
								"El GANADOR el entrenador '%c' con el pokemon '%s'",
								entrenadorGanador->simbolo, pokemon1->nombre);

						//Devuelvo a la cola de Listo al entrenador contrario que fue quien gano la batalla
						pthread_mutex_lock(&colaDeListosMutex);
						pthread_mutex_lock(&setEntrenadoresMutex);
						entrenadorGanador->estaBloqueado = 0;
						saleColaBloqueados(entrenadorGanador);
						queue_push(colaDeListos, entrenadorGanador);
						pthread_mutex_unlock(&setEntrenadoresMutex);
						pthread_mutex_unlock(&colaDeListosMutex);

						bool _funcBuscarEntrenador(char* listElement) {
							if ((entrenadorGanador != NULL
									&& entrenadorGanador->simbolo != NULL)
									&& (*listElement != NULL)) {
								return (*listElement
										== entrenadorGanador->simbolo);
							} else {
								printf(
										"Fallo variable NULL _funcBuscarEntrenador\n");
								printf("variable 1 %d\n",
										entrenadorGanador->simbolo != NULL);
								printf("variable 2 %d\n", *listElement != NULL);
							}
							return false;

						}

						//Remuevo al entrenador ganador de la lista de deadlock
						list_remove_by_condition(listaDeadlock,
								(void*) _funcBuscarEntrenador);

						//Liberemos los recursos
						//Como el puntero loser apunta a alguno de los otros 2, no se lo libera
						free(pokemonParaBatalla1);
						free(pokemonParaBatalla2);

					} else {
						//2) Se debera iterar hasta que la lista tenga tamaño 1, ese entrenador va a ser el candidato a muerte
						enBatalla = false;
					}

				}						// while (enBatalla)

				//3) Se obtiene la victima y se procede a matarlo y devolver sus pokemones
				pthread_mutex_lock(&setEntrenadoresMutex);
				t_entrenador* entrenadorVictima = list_get(listaDeadlock, 0);//En este punto solo debe haber un entrenador en la lista de deadlock
				pthread_mutex_unlock(&setEntrenadoresMutex);
				log_info(logMapa,
						"El entrenador VICTIMA para resolver el deadlock es '%c'",
						entrenadorVictima->simbolo);
				log_info(logMapa,
						"Se le notifica al entrenador '%c' que se va a morir",
						entrenadorVictima->simbolo);
				matar(entrenadorVictima);//se envia mensaje al entrenador y devuelve sus pokemones al mapa

				list_destroy(listaDeadlock);
			}						// if (modoBatalla == 1)
		}						// if (tamanioColaBloqueados > 1)
	}						// while (1)
}

t_list* detectarInterbloqueo() {

	//1)Obtener la lista asignacion ( La cantidad de pokemon que tiene cada Entrenador)
	t_list* asignacion = list_create();
	cargarListaAsignacion(asignacion);

	//1.1)Obtener la lista solicitud ( La cantidad de pokemon que necesitan los entrenadores en este momento)
	t_list* solicitud = list_create();
	cargarListaSolicitud(solicitud);

	//2)Crear lista para entrenadores NO bloqueados
	t_list* entrenadoresDeadlock = list_create();
	cargarEntrenadores(entrenadoresDeadlock);

	//3) Buscar en lista de asignacion el entrenador que tenga TODOS 0 en su lista de pokemones y sacarlos
	quitarEntrenadoresSinAsignacion(asignacion, entrenadoresDeadlock);

	//4) Crear lista auxiliar con pokemones disponibles (pokemon y cantidad)
	t_list* pokemonesDisponibles = list_create();
	cargarCantidadPokemonesExistentes(pokemonesDisponibles);

	//4.1) Informamos el estado de todas las Listas
	iformarEstadosRecursos(asignacion,solicitud,pokemonesDisponibles);

	//5)Recorrer matriz asignacion buscando que tenga recursos <= a la lista creada previamente (pokemonesExistentes)
	bool notFound = true;
	while (notFound) {

		//** Find function by cantidad**//
		bool recursosMenores_aAsignados(t_pokemones_Asignacion* listElement) {

			//** Find function by pokemon**//
			bool _funcBuscarPokemon(t_pokemones_Asignacion *pokemonAux) {
				if ((pokemonAux != NULL && pokemonAux->pokemon_id != NULL)
						&& (listElement != NULL
								&& listElement->pokemon_id != NULL)) {
					log_info(logMapa, "pokemonAux %c listElement %c",
							pokemonAux->pokemon_id, listElement->pokemon_id);
					return pokemonAux->pokemon_id == listElement->pokemon_id;
				} else {
					printf("Fallo variable NULL _funcBuscarPokemon\n");
					printf("variable 1 %d\n",
							(pokemonAux != NULL
									&& pokemonAux->pokemon_id != NULL));
					printf("variable 2 %d\n",
							(listElement != NULL
									&& listElement->pokemon_id != NULL));
				}
				return false;
			}

			t_pokemones_Asignacion * pokemonDisponible = list_find(
					pokemonesDisponibles, (void*) _funcBuscarPokemon);

			if (pokemonDisponible != NULL) {
				return (listElement->cantidad <= pokemonDisponible->cantidad);
			} else {
				return false;
			}
		}

		int i;
		for (i = 0; i < list_size(solicitud); i++) {
			t_entrenador_Asignacion* entrenadorAux = list_get(solicitud, i);
			notFound = list_all_satisfy(entrenadorAux->pokemonesAsignados,
					(void*) recursosMenores_aAsignados);

			if (notFound) {
				//5.1) El entrenador tiene recursos <= a los asignados
				int j;
				for (j = 0; j < list_size(entrenadorAux->pokemonesAsignados);
						j++) {
					t_pokemones_Asignacion* pokemonAsignado = list_get(
							entrenadorAux->pokemonesAsignados, j);

					bool _funcBuscarPokemon(t_pokemones_Asignacion *listElement) {
						if ((listElement != NULL
								&& listElement->pokemon_id != NULL)
								&& (pokemonAsignado != NULL
										&& pokemonAsignado->pokemon_id != NULL)) {
							return listElement->pokemon_id
									== pokemonAsignado->pokemon_id;
						} else {
							printf("Fallo variable NULL _funcBuscarPokemon\n");
							printf("variable 1 %d\n",
									(listElement != NULL
											&& listElement->pokemon_id != NULL));
							printf("variable 2 %d\n",
									(pokemonAsignado != NULL
											&& pokemonAsignado->pokemon_id
													!= NULL));
						}
						return false;

					}

					t_pokemones_Asignacion * pokemonDisponible = list_find(
							pokemonesDisponibles, (void*) _funcBuscarPokemon);

					if (pokemonDisponible != NULL) { //verifying not null value to add
						pokemonDisponible->cantidad +=
								pokemonAsignado->cantidad;
					}

				}

				bool _funcBuscarEntrenador(char* listElement) {
					if ((entrenadorAux != NULL
							&& entrenadorAux->entrenador != NULL)
							&& (*listElement != NULL)) {
						return (*listElement == entrenadorAux->entrenador);
					} else {
						printf("Fallo variable NULL _funcBuscarEntrenador\n");
						printf("variable 1 %d\n",
								(entrenadorAux != NULL
										&& entrenadorAux->entrenador != NULL));
						printf("variable 2 %d\n", (*listElement != NULL));
					}
					return false;
				}

				//5.1) Remuevo el entrenador de la lista de entrenadoresNoBloqueados
				list_remove_and_destroy_by_condition(entrenadoresDeadlock,
						(void*) _funcBuscarEntrenador, (void*) free);
			}
		}

	}

	if (list_size(entrenadoresDeadlock) > 0) {
		log_info(logMapa, "Hay DEADLOCK!!");
		int i;
		for (i = 0; i < list_size(entrenadoresDeadlock); i++) {
			char* entrenador = list_get(entrenadoresDeadlock, i);
			log_info(logMapa, "---> Entrenador: '%c'", *entrenador);
		}
	}

	return entrenadoresDeadlock;

}

void cargarListaAsignacion(t_list *asignacion) {
	int i;
	//recorrer la lista de entrenadores
	t_list* pokemonesList = list_create();
	cargarPokemonesExistentes(pokemonesList);

	pthread_mutex_lock(&setEntrenadoresMutex);
	for (i = 0; i < list_size(listaDeEntrenadores); i++) {
		int j;
		t_entrenador_Asignacion* entrenador = malloc(
				sizeof(t_entrenador_Asignacion));
		t_entrenador* entrenadorAux = list_get(listaDeEntrenadores, i);
		entrenador->entrenador = entrenadorAux->simbolo;
		entrenador->pokemonesAsignados = list_create();

		cargarPokeNests(entrenador->pokemonesAsignados, pokemonesList);

		//Recorro la lista de Pokemones caputurados por este entrenador
		for (j = 0; j < list_size(entrenadorAux->listaDePokemonesCapturados);
				j++) {

			t_pokemones* pokemon = list_get(
					entrenadorAux->listaDePokemonesCapturados, j);

			bool _funcBuscarPokemon(t_pokemones_Asignacion *pokemonAux) {
				if ((pokemonAux != NULL && pokemonAux->pokemon_id != NULL)
						&& (pokemon != NULL && pokemon->id != NULL)) {
					return pokemonAux->pokemon_id == pokemon->id;
				} else {
					printf("Fallo variable NULL _funcBuscarPokemon\n");
					printf("variable 1 %d\n",
							(pokemonAux != NULL
									&& pokemonAux->pokemon_id != NULL));
					printf("variable 2 %d\n",
							(pokemon != NULL && pokemon->id != NULL));
				}
				return false;
			}

			//Determino si el Pokemon que llego ya estaba en  mi lista de Asignados.
			t_pokemones_Asignacion *recursoDisponible = list_find(
					entrenador->pokemonesAsignados,
					(void *) _funcBuscarPokemon);

			//Si existia sumo uno
			if (recursoDisponible != NULL) {
				recursoDisponible->cantidad++;
			}

		}

		list_add(asignacion, entrenador);
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);
}

void cargarPokemonesExistentes(t_list *pokemonesList) {
	int i;

	pthread_mutex_lock(&listaDePokenestMutex);
	for (i = 0; i < list_size(listaDePokenest); i++) {

		t_pokenest* pokenest = list_get(listaDePokenest, i);
		list_add(pokemonesList, &pokenest->metadata); //estaba agregando null a la lista

	}
	pthread_mutex_unlock(&listaDePokenestMutex);

}

void cargarPokeNests(t_list *pokemonesAsignados, t_list* pokemonesList) {
	int i;
	for (i = 0; i < list_size(pokemonesList); i++) {
		t_metadataPokenest* pokemon = list_get(pokemonesList, i);
		t_pokemones_Asignacion *recursoDisponible = malloc(
				sizeof(t_pokemones_Asignacion));

		recursoDisponible->cantidad = 0;
		recursoDisponible->pokemon_id = pokemon->id;
		list_add(pokemonesAsignados, recursoDisponible);

	}
}

void cargarListaSolicitud(t_list *solicitud) {
	int i;
	//recorrer la lista de entrenadores
	t_list* pokemonesList = list_create();
	cargarPokemonesExistentes(pokemonesList);

	pthread_mutex_lock(&setEntrenadoresMutex);
	for (i = 0; i < list_size(listaDeEntrenadores); i++) {
		t_entrenador_Asignacion* entrenador = malloc(
				sizeof(t_entrenador_Asignacion));
		t_entrenador* entrenadorAux = list_get(listaDeEntrenadores, i);
		entrenador->entrenador = entrenadorAux->simbolo;
		entrenador->pokemonesAsignados = list_create();

		cargarPokeNests(entrenador->pokemonesAsignados, pokemonesList);

		//busca el pokemon deseado por este entrenador en la lista de pokemones existentes en el mapa
		bool _funcBuscarPokemon(t_pokemones_Asignacion *pokemonAux) {
			if ((pokemonAux != NULL && pokemonAux->pokemon_id != NULL)
					&& (entrenadorAux != NULL && entrenadorAux->pokemonD != NULL)) {
				return pokemonAux->pokemon_id == entrenadorAux->pokemonD;
			} else {
				printf("Fallo variable NULL _funcBuscarPokemon\n");
				printf("variable 1 %d\n",
						(pokemonAux != NULL && pokemonAux->pokemon_id != NULL));
				printf("variable 2 %d\n",
						(entrenadorAux != NULL
								&& entrenadorAux->pokemonD != NULL));
			}
			return false;
		}

		t_pokemones_Asignacion *pokemonDeseado = list_find(
				entrenador->pokemonesAsignados, (void *) _funcBuscarPokemon);

		//le asigna 1 al pokemon deseado de la lista
		if (pokemonDeseado != NULL) {
			pokemonDeseado->cantidad = 1;
		}

		list_add(solicitud, entrenador);
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);
}

void cargarEntrenadores(t_list *entrenadores) {
	int i;

	pthread_mutex_lock(&setEntrenadoresMutex);
	for (i = 0; i < list_size(listaDeEntrenadores); i++) {
		t_entrenador* entrenadorAux = list_get(listaDeEntrenadores, i);

		if (entrenadorAux->estaBloqueado == 1) {
			char *entrenador = &entrenadorAux->simbolo;
			list_add(entrenadores, entrenador);
		}
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

}

void quitarEntrenadoresSinAsignacion(t_list *asignacion,
		t_list *entrenadoresNoBloqueados) {
	int flag;
	int i, j;

	for (i = 0; i < list_size(asignacion); i++) {
		flag = 0;
		t_entrenador_Asignacion* entrenadorAux = list_get(asignacion, i);

		for (j = 0; j < list_size(entrenadorAux->pokemonesAsignados); j++) {
			t_pokemones_Asignacion* auxPok = list_get(
					entrenadorAux->pokemonesAsignados, j);

			if (auxPok->cantidad > 0) {
				flag = 1;
			}
		}

		bool _funcBuscarEntrenador(char *entrenador) {
			if ((*entrenador != NULL)
					&& (entrenadorAux != NULL
							&& entrenadorAux->entrenador != NULL)) {
				return *entrenador == entrenadorAux->entrenador;
			} else {
				printf("Fallo variable NULL _funcBuscarEntrenador\n");
				printf("variable 1 %d\n", (*entrenador != NULL));
				printf("variable 2 %d\n",
						(entrenadorAux != NULL
								&& entrenadorAux->entrenador != NULL));
			}
			return false;
		}

		void _destroyElement(char *entrenador) {
			free(entrenador);
		}

		if (flag == 0) {
			list_remove_and_destroy_by_condition(entrenadoresNoBloqueados,
					(void *) _funcBuscarEntrenador, (void *) _destroyElement);
		}
	}
}

void cargarCantidadPokemonesExistentes(t_list *pokemonesList) {
	int i, j;

	pthread_mutex_lock(&listaDePokenestMutex);
	for (i = 0; i < list_size(listaDePokenest); i++) {

		t_pokenest* pokenest = list_get(listaDePokenest, i);

		for (j = 0; j < list_size(pokenest->listaDePokemones); j++) {
			t_pokemones* pokemon = list_get(pokenest->listaDePokemones, j);

			t_pokemones_Asignacion* pokAux = malloc(
					sizeof(t_pokemones_Asignacion));
			pokAux->pokemon_id = pokemon->id;
			pokAux->cantidad = 1;

			bool _funcBuscarPokemon(t_pokemones_Asignacion *element) {
				if ((element != NULL && element->pokemon_id != NULL)
						&& (pokAux != NULL && pokAux->pokemon_id != NULL)) {
					return element->pokemon_id == pokAux->pokemon_id;
				} else {
					printf("Fallo variable NULL _funcBuscarPokemon\n");
					printf("variable 1 %d\n",
							(element != NULL && element->pokemon_id != NULL));
					printf("variable 2 %d\n",
							(pokAux != NULL && pokAux->pokemon_id != NULL));
				}
				return false;

			}

			//Determino si el Pokemon es un nuevo pokemon o ya exisita.
			t_pokemones_Asignacion *recursoDisponible = list_find(pokemonesList,
					(void *) _funcBuscarPokemon);

			if (recursoDisponible == NULL) {
				list_add(pokemonesList, pokAux);
			} else {
				recursoDisponible->cantidad++;
			}
		}
	}
	pthread_mutex_unlock(&listaDePokenestMutex);

}

t_pokemones* dameTuMejorPokemon(t_entrenador* entrenador) {
	bool _pokemonMayor(t_pokemones* pokemon1, t_pokemones* pokemon2) {
		return pokemon1->nivel > pokemon2->nivel;
	}

	list_sort(entrenador->listaDePokemonesCapturados, (void*) _pokemonMayor);
	t_pokemones* pokemonGroso = list_get(entrenador->listaDePokemonesCapturados,
			0); //como ya esta ordenada la lista el primer pokemon va a ser el mas poronga.

	return pokemonGroso;
}

void matar(t_entrenador* entrenador) {

	pthread_mutex_lock(&setEntrenadoresMutex);
	int socketE = entrenador->socket;
	pthread_mutex_unlock(&setEntrenadoresMutex);

	eliminarEntrenador(entrenador->simbolo);

	sendClientMessage(&socketE, "MATAR!", MATAR); //le avisamos al entrenador que cago fuego
}

void recibirSignal() {
	while (1) {
		signal(SIGUSR2, reloadMetadata);
	}
}

void reloadMetadata() {
	pthread_mutex_lock(&metadataMutex);
	crearArchivoMetadataDelMapa(rutaMetadata, &metadataMapa, logMapa);
	pthread_mutex_unlock(&metadataMutex);
}

void evaluarEstadoEntrenador(t_entrenador* entrenador) {
	pthread_mutex_lock(&setEntrenadoresMutex);
	if (entrenador->posD_x != -1) {
		if (entrenador->posD_x == entrenador->pos_x
				&& entrenador->posD_y == entrenador->pos_y) { //si se encuentra en la posicion deseada le avisamos que llego y asi comienza su movimiento
			log_info(logMapa, "Trainer: '%c' came to the pokenest",
					entrenador->simbolo);
			sendClientMessage(&entrenador->socket, "LLEGO!", LLEGO);
			entrenador->seEstaMoviendo = 0; // si ya llego a la posicion no se esta moviendo mas
			sem_post(&processNextMessageSem); //enable message reception
		}
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);

	if (entrenador->seEstaMoviendo) { // le pedimos que se mueva
		log_info(logMapa, "Trainer: '%c' has not yet reached his position",
				entrenador->simbolo);
		sendClientMessage(&entrenador->socket, "MOVETE!", MOVETE);
		sem_post(&processNextMessageSem); //enable message reception
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);

	pthread_mutex_lock(&setEntrenadoresMutex);
	if (entrenador->pokemonD == '/') { //sino busca ninguno por el momento le preguntamos cual quiera buscar (movimiento libre)
		sendClientMessage(&entrenador->socket, "LIBRE!", LIBRE);
		log_info(logMapa, "Trainer: '%c' has free action ",
				entrenador->simbolo);
		sem_post(&processNextMessageSem); //enable message reception
	}
	pthread_mutex_unlock(&setEntrenadoresMutex);
}



bool noSeBorro(t_entrenador* entrenador){
	char simbolo;
	bool buscarPorSimbolo(t_entrenador* entrenadorDeLaLista) {
		if ((entrenadorDeLaLista != NULL
				&& entrenadorDeLaLista->simbolo != NULL)
				&& (simbolo != NULL)) {
			return (entrenadorDeLaLista->simbolo == simbolo);
		} else {
			printf("Fallo variable NULL buscarPorSimbolo\n");
			printf("variable 1 %d\n",
					(entrenadorDeLaLista != NULL
							&& entrenadorDeLaLista->simbolo != NULL));
			printf("variable 2 %d\n", (simbolo != NULL));
		}
		return false;

	}

	pthread_mutex_lock(&setEntrenadoresMutex);
	simbolo = entrenador->simbolo;
	bool noSeDesconectoNadie = list_any_satisfy(listaDeEntrenadores,
			(void*) buscarPorSimbolo);
	pthread_mutex_unlock(&setEntrenadoresMutex);

	return noSeDesconectoNadie;
}

void saleColaBloqueados(t_entrenador* entrenador){
	// Al Sacarlo de la lista debemos calcular tiempos en cola.
	time_t tiempo2 = time(0);
	entrenador->tiempoBloqueado = entrenador->tiempoBloqueado+(difftime(tiempo2, entrenador->timeIngresoBloq));
}

void iformarEstadosRecursos(t_list* asignacion,t_list* solicitud,t_list* pokemonesDisponibles){
	log_info(logMapa, "========================================");
	log_info(logMapa, "==   Estado de Recursos en DEADLOCK   ==");
	log_info(logMapa, "========================================");
	log_info(logMapa, " ");
	log_info(logMapa, "* Tabla De Asignacion");
	log_info(logMapa, "  -------------------");
	loguearEntrenadorAsignacion(asignacion);

	log_info(logMapa, " ");
	log_info(logMapa, "* Tabla De solicitud");
	log_info(logMapa, "  -------------------");
	loguearEntrenadorAsignacion(solicitud);

	log_info(logMapa, " ");
	log_info(logMapa, "* Tabla De Disponibilidad");
	log_info(logMapa, "  -----------------------");
	loguearPokemonesAsignacion(pokemonesDisponibles);

	log_info(logMapa, " ");
	log_info(logMapa, "========================================");

}

void loguearEntrenadorAsignacion(t_list* asignacion){
	int i;

	for (i = 0; i < list_size(asignacion); i++) {

		char* cabecera = string_new();
		char* lineaDato = string_new();

		t_entrenador_Asignacion* entrenadorAux = list_get(asignacion, i);

		string_append(&cabecera,"Entrenador             |");

		string_append(&lineaDato, string_from_format("%c", entrenadorAux->entrenador)  );
		//Para que todos comiencen la matriz en el mismo lugar

		int cantBlancos = 22 - string_length(entrenadorAux->entrenador);
		while(cantBlancos>0){
			string_append(&lineaDato," ");
			cantBlancos--;
		}
		string_append(&lineaDato,"|");

		int j;
		for (j = 0; j < list_size(entrenadorAux->pokemonesAsignados); j++) {
			t_pokemones_Asignacion* auxPok = list_get(entrenadorAux->pokemonesAsignados, j);
			if(i == 0){
				//Obtengo La cabecera para mostrar Solo una vez
				string_append(&cabecera, string_from_format("%c", auxPok->pokemon_id)  );
				string_append(&cabecera,"|");
			}
			string_append(&lineaDato,string_itoa(auxPok->cantidad));
			string_append(&lineaDato," |");
		}

		if(i == 0){
			//Solo si estoy en el primer debo armar la cabecera
			log_info(logMapa,cabecera);
		}
		log_info(logMapa,lineaDato);
	}
}

void loguearPokemonesAsignacion(t_list* asignacion){
	int i;
	char* cabecera = string_new();
	char* lineaDato = string_new();

	for (i = 0; i < list_size(asignacion); i++) {
		t_pokemones_Asignacion* auxPok = list_get(asignacion, i);
		string_append(&cabecera, string_from_format("%c", auxPok->pokemon_id)  );
		string_append(&cabecera," |");
		string_append(&lineaDato,string_itoa(auxPok->cantidad));
		string_append(&lineaDato," |");
	}

	log_info(logMapa,cabecera);
	log_info(logMapa,lineaDato);
}
