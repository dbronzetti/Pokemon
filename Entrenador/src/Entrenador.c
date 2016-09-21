/*
 ============================================================================
 Name        : Entrenador.c
 ============================================================================
 */

#include "Entrenador.h"

int socketMapa = 0;
int asd;

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *entrenador = string_new();
	char *pokedex = string_new();

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

	char* rutaMetadata = string_from_format("%s/Entrenadores/%s/metadata.dat",pokedex, entrenador);


	printf("Directorio de la metadata del entranador '%s': '%s'\n", entrenador,
			rutaMetadata);

	logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	crearArchivoMetadata(rutaMetadata);

	puts("@@@@@@@@@@@@@@@@@@@METADATA@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	char* mapaActual = queue_pop(metadataEntrenador.hojaDeViaje);

	char* rutaMetadataMapa = string_from_format("%s/Mapas/%s/metadata.dat",pokedex, mapaActual);

	crearArchivoMetadataDelMapa(rutaMetadataMapa, &metadataMapa);

	exitCode = connectTo(MAPA, &socketMapa);
	if (exitCode == EXIT_SUCCESS) {
		log_info(logEntrenador, "ENTRENADOR connected to MAPA successfully\n");
		printf("Se ha conectado correctamente al mapa: %s", mapaActual);
		while (1)
			scanf("%d", asd);
	} else {
		log_error(logEntrenador,
				"No server available - shutting down proces!!\n");
		return EXIT_FAILURE;
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

	hojaDeViaje = config_get_array_value(metadata,
			"hojaDeViaje");
	while (hojaDeViaje[i] != NULL) {
		queue_push(metadataEntrenador.hojaDeViaje, hojaDeViaje[i]);
		i++;
	}
	printf("Mapas a recorrer: ");
	imprimirArray(hojaDeViaje);

	i = 0;
	while (hojaDeViaje[i] != NULL) {
		char* obj = string_from_format("obj[%s]",
				hojaDeViaje[i]);
		queue_push(metadataEntrenador.obj,
				config_get_array_value(metadata, obj));
		printf("Dentro del mapa %s debe atrapar: ",
				hojaDeViaje[i]);
		imprimirArray(config_get_array_value(metadata, obj));
		i++;
	}

	metadataEntrenador.vidas = config_get_int_value(metadata, "vidas");
	printf("Cantidad de vidas: %d\n", metadataEntrenador.vidas);

	metadataEntrenador.reintentos = config_get_int_value(metadata, "reintentos");
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

