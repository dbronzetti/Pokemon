/*
 ============================================================================
 Name        : Entrenador.c
 ============================================================================
 */

#include "Entrenador.h"

int socketMapa = 0;

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *entrenador = string_new();
	pokedex = string_new();
	pthread_mutex_init(&turnoMutex, NULL);
	pthread_mutex_init(&pokemonCapturadoMutex, NULL);
	colaDeRutasDePokemones = queue_create();
	colaDeRutasDeMapas = queue_create();

	exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	for (i = 0; i < argc; i++) {
		//chekea el nick del entrenador
		if (strcmp(argv[i], "-e") == 0) {
			entrenador = argv[i + 1];
			printf("Nombre del entrenador: '%s'\n", entrenador);
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

	rutaMetadata = string_from_format("%s/Entrenadores/%s/metadata.dat",
			pokedex, entrenador);

	rutaDirDeBill = string_from_format("%s/Entrenadores/%s/Dir de Bill",
			pokedex, entrenador);

	rutaMedallas = string_from_format("%s/Entrenadores/%s/medallas", pokedex,
			entrenador);

	printf("Directorio de la metadata del entranador '%s': '%s'\n", entrenador,
			rutaMetadata);

	logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	crearArchivoMetadata(rutaMetadata);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	pthread_create(&hiloSignal, NULL, (void*) recibirSignal, NULL);

	i = 0;
	for (i = 0; i < queue_size(metadataEntrenador.hojaDeViaje); i++) {
		mapaActual = queue_pop(metadataEntrenador.hojaDeViaje);
		char** objetivosActuales = queue_pop(metadataEntrenador.obj); // un string con los objetivos separados por coma.

		colaDeObjetivos = parsearObjetivos(objetivosActuales); // la cola de objetivos actuales donde cada elemento es un char

		char* rutaMetadataMapa = string_from_format("%s/Mapas/%s/metadata.dat",
				pokedex, mapaActual);

		crearArchivoMetadataDelMapa(rutaMetadataMapa, &metadataMapa);

		exitCode = connectTo(MAPA, &socketMapa);

		if (exitCode == EXIT_SUCCESS) {
			log_info(logEntrenador,
					"ENTRENADOR connected to MAPA successfully\n");
			printf("Se ha conectado correctamente al mapa: %s\n", mapaActual);
			sendClientMessage(&socketMapa, metadataEntrenador.simbolo, NUEVO);
			pthread_create(&hiloEscuchar, NULL, (void*) recibirMsjs, NULL);

			jugar();

		} else {
			log_error(logEntrenador,
					"No server available - shutting down proces!!\n");
			return EXIT_FAILURE;
		}

	}
	return 0;
}

int connectTo(enum_processes processToConnect, int *socketClient) {
	int exitcode = EXIT_FAILURE; //DEFAULT VALUE
	int port;
	char *ip = string_new();

	switch (processToConnect) {
	case MAPA: {
		string_append(&ip, metadataMapa.ip);
		port = metadataMapa.puerto;
		break;
	}
	default: {
		log_info(logEntrenador, "Process '%s' NOT VALID to be connected.\n",
				getProcessString(processToConnect));
		break;
	}
	}
	exitcode = openClientConnection(ip, port, socketClient);

	//If exitCode == 0 the client could connect to the server
	if (exitcode == EXIT_SUCCESS) {

		// ***1) Send handshake
		exitcode = sendClientHandShake(socketClient, ENTRENADOR);

		if (exitcode == EXIT_SUCCESS) {

			// ***2)Receive handshake response
			//Receive message size
			int messageSize = 0;
			char *messageRcv = malloc(sizeof(messageSize));
			int receivedBytes = receiveMessage(socketClient, messageRcv,
					sizeof(messageSize));

			if (receivedBytes > 0) {
				//Receive message using the size read before
				memcpy(&messageSize, messageRcv, sizeof(int));
				messageRcv = realloc(messageRcv, messageSize);
				receivedBytes = receiveMessage(socketClient, messageRcv,
						messageSize);

				//starting handshake with client connected
				t_MessageGenericHandshake *message = malloc(
						sizeof(t_MessageGenericHandshake));
				deserializeHandShake(message, messageRcv);

				free(messageRcv);

				switch (message->process) {
				case ACCEPTED: {
					log_info(logEntrenador, "Conectado a MAPA - Messsage: %s\n",
							message->message);
					puts("Conectado al mapa!");
					break;
				}
				default: {
					log_error(logEntrenador,
							"Process couldn't connect to SERVER - Not able to connect to server %s. Please check if it's down.\n",
							ip);
					close(*socketClient);
					break;
				}
				}
			} else if (receivedBytes == 0) {
				//The client is down when bytes received are 0
				log_error(logEntrenador,
						"The client went down while receiving! - Please check the client '%d' is down!\n",
						*socketClient);
				close(*socketClient);
			} else {
				log_error(logEntrenador,
						"Error - No able to received - Error receiving from socket '%d', with error: %d\n",
						*socketClient, errno);
				close(*socketClient);
			}
		}

	} else {
		log_error(logEntrenador,
				"I'm not able to connect to the server! - My socket is: '%d'\n",
				*socketClient);
		close(*socketClient);
	}

	return exitcode;
}

void crearArchivoMetadata(char *rutaMetadata) {
	t_config* metadata;
	int i = 0;
	metadataEntrenador.hojaDeViaje = queue_create();
	metadataEntrenador.obj = queue_create();
	char** hojaDeViaje; //Creo un array auxiliar para poder encolar los objetivos de cada mapa sin desapilar la hojaDeViaje

	metadata = config_create(rutaMetadata);

	metadataEntrenador.nombre = config_get_string_value(metadata, "nombre");
	printf("Nombre: %s\n", metadataEntrenador.nombre);

	metadataEntrenador.simbolo = config_get_string_value(metadata, "simbolo");
	printf("Simbolo: %s\n", metadataEntrenador.simbolo);

	hojaDeViaje = config_get_array_value(metadata, "hojaDeViaje");
	while (hojaDeViaje[i] != NULL) {
		queue_push(metadataEntrenador.hojaDeViaje, hojaDeViaje[i]);
		i++;
	}
	printf("Mapas a recorrer: ");
	imprimirArray(hojaDeViaje);

	i = 0;
	while (hojaDeViaje[i] != NULL) {
		char* obj = string_from_format("obj[%s]", hojaDeViaje[i]);

		queue_push(metadataEntrenador.obj,
				config_get_array_value(metadata, obj)); //pushea en la cola un array de strings
		printf("Dentro del mapa %s debe atrapar: ", hojaDeViaje[i]);
		imprimirArray(config_get_array_value(metadata, obj));
		i++;
	}

	metadataEntrenador.vidas = config_get_int_value(metadata, "vidas");
	printf("Cantidad de vidas: %d\n", metadataEntrenador.vidas);

	metadataEntrenador.reintentos = config_get_int_value(metadata,
			"reintentos");
	printf("Cantidad de reintentos: %d\n", metadataEntrenador.reintentos);

}

void imprimirArray(char** array) {
	int i = 0;
	while (array[i] != NULL) {
		if (i == 0) {
			printf("[%s", array[i]);
			printf(", ");
		} else if (i == (strlen((char*) array) / sizeof(char*)) - 1) {
			printf("%s]", array[i]);
		} else {
			printf("%s", array[i]);
			printf(", ");
		}
		i++;
	}
	printf(" \n");
}

void recibirSignal() {
	while (1) {
		signal(SIGUSR1, sumarVida);
		signal(SIGTERM, restarVida);
		signal(SIGINT, cerrarEntrenador);
	}
}

void sumarVida() {
	metadataEntrenador.vidas++;
	log_info(logEntrenador,
			"Ha aumentado en 1 la vida del entrenador, ahora es de '%d'\n",
			metadataEntrenador.vidas);
}

void restarVida() {
	metadataEntrenador.vidas--;
	log_info(logEntrenador,
			"Ha disminuido en 1 la vida del entrenador, ahora es de '%d'\n",
			metadataEntrenador.vidas);
}

void desconectarse() {
	send(socketMapa, 0, 0, 0); //mandamos 0 bytes para que nos desconecte :)
	close(socketMapa);
}

void jugar() {
	char* objetivoActual;
	t_queue* colaDeObjetivos_M; //esta es la cola de objetivos modificables
	colaDeObjetivos_M = queue_create();
	colaDeObjetivos_M = colaDeObjetivos;

	while ((queue_size(colaDeObjetivos_M) > 0) || (objetivoActual != NULL)) //mientras queden objetivos y no se haya capturado el ultimo pokemon se sigue jugando en el mapa
	{

		pthread_mutex_lock(&turnoMutex);
		if (turno != SIN_MENSAJE) {
			switch (turno) {
			case LIBRE: { //si es un turno libre, le pedimos conocer la posicion de la pokenest
				log_info(logEntrenador, "Trainer send the id of the pokenest");
				objetivoActual = queue_pop(colaDeObjetivos_M);
				sendClientMessage(&socketMapa, objetivoActual, CONOCER);
				log_info(logEntrenador, "Conocer: Se manda: %s",
						objetivoActual);

				break;
			}

			case CONOCER: {	// no se hace nada...
				log_info(logEntrenador, "Trainer receive the position");
				//		sendClientMessage(&socketMapa, posicionPokenest, IR);

				break;
			}

			case LLEGO: {	// si llegamos le pedimos que lo capture
				log_info(logEntrenador,
						"Trainer ask to Mapa capture the pokemon");
				sendClientMessage(&socketMapa, objetivoActual, CAPTURAR);

				break;
			}

			case MOVETE: {// si estamos yendo a la pokenest le pedimos seguir moviendonos
				log_info(logEntrenador,
						"Trainer ask to mapa to move to the pokenest '%s'",
						objetivoActual);
				sendClientMessage(&socketMapa, objetivoActual, IR);
//				log_info(logEntrenador, "IR: Se manda: %s", objetivoActual);

				break;
			}

			case CAPTURADO: { // si se capturo ok hay que copiar el archivito metadata en el dir de bill
				char* rutaMetadataPokemon = string_from_format(
						"%s/metadata%s.dat", rutaDirDeBill, pokemonCapturado); //armamos la ruta de donde se va a copiar el archivo metadata del pokemon capturado :)

				char* rutaMedataPokemonMapa = string_from_format(
						"%s/Mapas/%s/Pokenest/%s/metadata.dat", pokedex,
						mapaActual, pokemonCapturado); //armamos la ruta del archivo metadata que vamos a copiar
				puts(rutaMedataPokemonMapa);

				log_info(logEntrenador, "Trainer has captured: %s",
						pokemonCapturado);
				copiarArchivos(rutaMedataPokemonMapa, rutaMetadataPokemon);
				queue_push(colaDeRutasDePokemones, rutaMetadataPokemon);
				free(rutaMetadataPokemon);
				free(rutaMedataPokemonMapa);
				objetivoActual = NULL;
				break;
			}

			case ERROR_CONOCER: { //si hubo un error cuando queria conocer la pos de la pokenest se vuelva a mandar :)
				log_info(logEntrenador, "Trainer send the id of the pokenest");
				sendClientMessage(&socketMapa, objetivoActual, CONOCER);
				log_info(logEntrenador, "Conocer: Se manda: %s",
						objetivoActual);

				break;
			}

			case MATAR: { //el mapa mando a matar al entrenador
				log_info(logEntrenador, "Killing trainer after Map kick");
				restarVida();
				if (metadataEntrenador.vidas > 0) {
					reconectarse();
					colaDeObjetivos_M = colaDeObjetivos; // la cola se rellena
				}

				else {
					puts("No posee mas vidas desea reintentar?");
				}
				break;
			}

			}
			turno = SIN_MENSAJE;

		}
		pthread_mutex_unlock(&turnoMutex);

	}

	yoYaGane(mapaActual);

}

t_queue* parsearObjetivos(char** objetivos) {

	int i = 0;
	t_queue* colaDeObjetivos = queue_create();
	while (objetivos[i] != '\0') { //el bucle dura hasta que se lee to_do el array
		char* unObjetivo = objetivos[i];

		queue_push(colaDeObjetivos, unObjetivo);
		i++;
	}
	return colaDeObjetivos;

}

void recibirMsjs() {
	while (1) {

		int messageSize = 0;
		char *messageRcv = malloc(sizeof(messageSize));
		int receivedBytes = receiveMessage(&socketMapa, messageRcv,
				sizeof(messageSize));

		if (receivedBytes > 0) {
			//Receive message using the size read before
			memcpy(&messageSize, messageRcv, sizeof(int));
			messageRcv = realloc(messageRcv, messageSize);
			receivedBytes = receiveMessage(&socketMapa, messageRcv,
					messageSize);

			//starting handshake with client connected
			t_Mensaje *message = malloc(sizeof(t_Mensaje));
			deserializeClientMessage(message, messageRcv);

			free(messageRcv);

			switch (message->tipo) {
			case LIBRE: { //msj que envia el mapa para indicar que comenzo un turno libre
				log_info(logEntrenador, "New action begins", message->mensaje);
				pthread_mutex_lock(&turnoMutex);
				turno = LIBRE;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}

			case LLEGO: { //msj que envia el mapa para indicar que llego a la pokenest
				log_info(logEntrenador,
						"New action begins: trainer came to pokenest: %s\n",
						message->mensaje);
				pthread_mutex_lock(&turnoMutex);
				turno = LLEGO;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}

			case CONOCER: { //msj que envia el mapa para indicando la posicion de la pokenest
				log_info(logEntrenador,
						"New action begins: the position of the pokenest is: %s\n",
						message->mensaje);
				pthread_mutex_lock(&turnoMutex);
				turno = CONOCER;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}

			case MOVETE: { //msj que envia el mapa para indicando que el pokemon ha sido capturado
				log_info(logEntrenador,
						"New action begins: trainer has not yet reached his position: %s\n",
						message->mensaje);
				pthread_mutex_lock(&turnoMutex);
				turno = MOVETE;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}

			case CAPTURADO: { //msj que envia si fue capturado OK !!
				log_info(logEntrenador,
						"Trainer captured the pokemon SUCCESSFUL");
				pthread_mutex_lock(&pokemonCapturadoMutex);
				puts(message->mensaje);
				pokemonCapturado = message->mensaje;
				pthread_mutex_unlock(&pokemonCapturadoMutex);

				pthread_mutex_lock(&turnoMutex);
				turno = CAPTURADO;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}
			case MATAR: { //msj que envia si fue capturado OK !!
				log_info(logEntrenador,
						"The trainer has been killed by the Map");
				pthread_mutex_lock(&turnoMutex);
				turno = MATAR;
				pthread_mutex_unlock(&turnoMutex);
				break;
			}
			default: {
				log_error(logEntrenador,
						"Process couldn't connect to SERVER - Not able to connect to server %s. Please check if it's down.\n",
						metadataMapa.ip);
				close(socketMapa);
				break;
			}
			}
		} else if (receivedBytes == 0) {
			//The client is down when bytes received are 0
			log_error(logEntrenador,
					"The client went down while receiving! - Please check the client '%d' is down!\n",
					socketMapa);
			close(socketMapa);
			break;
		} else {
			log_error(logEntrenador,
					"Error - No able to received - Error receiving from socket '%d', with error: %d\n",
					socketMapa, errno);
			close(socketMapa);
			break;
		}
	}
}

void copiarArchivos(char* archivoOrigen, char* archivoDestino) {
	char * archivoOrigenSinBlancos=  str_replace(archivoOrigen," ","\\ ");

	char * archivoDestinoSinBlancos= str_replace(archivoDestino," ", "\\ ");

	char *command  = string_from_format("cp %s %s", archivoOrigenSinBlancos,archivoDestinoSinBlancos);

	printf("----------------------------------------\n");
	printf("%s \n",command);
	printf("----------------------------------------\n");
	system(command);
//
//	FILE *fp_org, *fp_dest;
//	char c;
//
//	if (!(fp_org = fopen(archivoOrigen, "rt"))
//			|| !(fp_dest = fopen(archivoDestino, "wt"))) {
//		perror("Error de apertura de ficheros");
//		exit(EXIT_FAILURE);
//	}
//
//	while ((c = fgetc(fp_org)) != EOF && !ferror(fp_org) && !ferror(fp_dest))
//		fputc(c, fp_dest);
//
//	fclose(fp_org);
//	fclose(fp_dest);

}


char* str_replace(const char *strbuf, const char *strold, const char *strnew) {
	char *strret, *p = NULL;
	char *posnews, *posold;
	size_t szold = strlen(strold);
	size_t sznew = strlen(strnew);
	size_t n = 1;

	if (!strbuf)
		return NULL;
	if (!strold || !strnew || !(p = strstr(strbuf, strold)))
		return strdup(strbuf);

	while (n > 0) {
		if (!(p = strstr(p + 1, strold)))
			break;
		n++;
	}

	strret = (char*) malloc(strlen(strbuf) - (n * szold) + (n * sznew) + 1);

	p = strstr(strbuf, strold);

	strncpy(strret, strbuf, (p - strbuf));
	strret[p - strbuf] = 0;
	posold = p + szold;
	posnews = strret + (p - strbuf);
	strcpy(posnews, strnew);
	posnews += sznew;

	while (n > 0) {
		if (!(p = strstr(posold, strold)))
			break;
		strncpy(posnews, posold, p - posold);
		posnews[p - posold] = 0;
		posnews += (p - posold);
		strcpy(posnews, strnew);
		posnews += sznew;
		posold = p + szold;
	}

	strcpy(posnews, posold);
	return strret;
}

void borrarArchivos(char* rutaDeleted) { //se borran todos los archivos que se pasa por la cola

	char * archivoOrigenSinBlancos=  str_replace(rutaDeleted," ","\\ ");

	char *command  = string_from_format("rm -f %s/*", archivoOrigenSinBlancos);

	printf("----------------------------------------\n");
	printf("%s \n",command);
	printf("----------------------------------------\n");
	system(command);

}

void cerrarEntrenador() {
	borrarArchivos(rutaDirDeBill);
	borrarArchivos(rutaMedallas);
	exit(0);
}

void yoYaGane() {
	char* nombreMedalla = "medalla*.*";

	char* rutaMedallaMapa = string_from_format("%s/Mapas/%s/%s", pokedex,
			mapaActual, nombreMedalla);

	char* rutaMedallaEntrenador = string_from_format("%s/", rutaMedallas);

	copiarArchivos(rutaMedallaMapa, rutaMedallaEntrenador);
	borrarArchivos(rutaDirDeBill);
	desconectarse();
	pthread_cancel(hiloEscuchar);

}

int reconectarse() {
	borrarArchivos(rutaDirDeBill);
	desconectarse();
	pthread_cancel(hiloEscuchar);
	exitCode = connectTo(MAPA, &socketMapa);

	if (exitCode == EXIT_SUCCESS) {
		log_info(logEntrenador, "ENTRENADOR connected to MAPA successfully\n");
		printf("Se ha conectado correctamente al mapa: %s\n", mapaActual);
		sendClientMessage(&socketMapa, metadataEntrenador.simbolo, NUEVO);
		pthread_create(&hiloEscuchar, NULL, (void*) recibirMsjs, NULL);

	} else {
		log_error(logEntrenador,
				"No server available - shutting down proces!!\n");
		return EXIT_FAILURE;
	}

	return 0;
}
