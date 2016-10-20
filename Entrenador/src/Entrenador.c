/*
 ============================================================================
 Name        : Entrenador.c
 ============================================================================
 */

#include "Entrenador.h"

int socketMapa = 0;
int llegoMsj = 0;
t_queue* colaDeObjetivos;
enum_messages turno;
char* posicionPokenest;

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *entrenador = string_new();
	char *pokedex = string_new();
	pthread_t hiloSignal; //un hio para detectar la signals que se le envia
	pthread_t hiloEscuchar; //un hilo para escuchar los msjs del server
	int ganoMapa;

	int exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

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

	char* rutaMetadata = string_from_format("%s/Entrenadores/%s/metadata.dat",
			pokedex, entrenador);

	printf("Directorio de la metadata del entranador '%s': '%s'\n", entrenador,
			rutaMetadata);

	logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	crearArchivoMetadata(rutaMetadata);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	pthread_create(&hiloSignal, NULL, (void*) recibirSignal, NULL);


	i = 0;
	for (i = 0; i < queue_size(metadataEntrenador.hojaDeViaje); i++) {
		char* mapaActual = queue_pop(metadataEntrenador.hojaDeViaje);
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

			pthread_join(hiloSignal, NULL);
			pthread_join(hiloEscuchar, NULL);


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
		signal(SIGINT, desconectarse);
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

	sendClientMessage(&socketMapa, metadataEntrenador.simbolo, DESCONECTAR);
	log_info(logEntrenador, "Se desconecto del mapa y el proceso se cerrara");
	sleep(1); //dormimos un segundo para darle tiempo al mapa de cerrar el socket correctamente y no joder el select.
	close(socketMapa);
	exit(0);
}

void jugar() {

	while (queue_size(colaDeObjetivos)) //mientras queden objetivos se sigue jugando en el mapa
	{
		char* objetivoActual;
		if (llegoMsj!=0) {
			switch (turno) {
			case LIBRE: { //si es un turno libre, le pedimos conocer la posicion de la pokenest
				log_info(logEntrenador, "Trainer send the id of the pokenest");
				objetivoActual = queue_pop(colaDeObjetivos);
				sendClientMessage(&socketMapa, objetivoActual, CONOCER);
				llegoMsj = 0;
				break;
			}

			case CONOCER: {	// no se hace nada...
				log_info(logEntrenador, "Trainer receive the position");
		//		sendClientMessage(&socketMapa, posicionPokenest, IR);
				llegoMsj = 0;
				break;
			}

			case LLEGO: {	// si llegamos le pedimos que lo capture
				log_info(logEntrenador, "Trainer ask to Mapa capture the pokemon");
				sendClientMessage(&socketMapa, objetivoActual, CAPTURAR);
				llegoMsj = 0;
				break;
			}

			case MOVETE: {	// si estamos yendo a la pokenest le pedimos seguir moviendonos
				log_info(logEntrenador, "Trainer ask to mapa to move to the pokenest");
				sendClientMessage(&socketMapa, objetivoActual, IR);
				llegoMsj = 0;
				break;
			}

			}

		}

	}

	desconectarse();
}

t_queue* parsearObjetivos(char** objetivos) {

	int i = 0;
	t_queue* colaDeObjetivos = queue_create();
	while (objetivos[i] != '\0') { //el bucle dura hasta que se lee todo el array
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
			receivedBytes = receiveMessage(&socketMapa, messageRcv, messageSize);

			//starting handshake with client connected
			t_Mensaje *message = malloc(sizeof(t_Mensaje));
			deserializeClientMessage(message, messageRcv);

			free(messageRcv);

			switch (message->tipo) {
			case LIBRE: { //msj que envia el mapa para indicar que comenzo un turno libre
				log_info(logEntrenador, "New action begins", message->mensaje);
				llegoMsj = 1;
				turno = LIBRE;
				break;
			}

			case LLEGO: { //msj que envia el mapa para indicar que llego a la pokenest
				log_info(logEntrenador,
						"New action begins: trainer came to pokenest: %s\n",
						message->mensaje);
				llegoMsj = 1;
				turno = LLEGO;
				break;
			}

			case CONOCER: { //msj que envia el mapa para indicando la posicion de la pokenest
				log_info(logEntrenador,
						"New action begins: the position of the pokenest is: %s\n",
						message->mensaje);
				llegoMsj = 1;
				turno = CONOCER;
				break;
			}

			case MOVETE: { //msj que envia el mapa para indicando que el pokemon ha sido capturado
				log_info(logEntrenador,
						"New action begins: trainer has not yet reached his position: %s\n",
						message->mensaje);
				llegoMsj = 1;
				turno = MOVETE;
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
		} else {
			log_error(logEntrenador,
					"Error - No able to received - Error receiving from socket '%d', with error: %d\n",
					socketMapa, errno);
			close(socketMapa);
		}
	}
}

