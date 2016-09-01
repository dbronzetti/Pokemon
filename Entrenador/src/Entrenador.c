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

	int exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

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

	logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	exitCode = connectTo(MAPA, &socketMapa);
	if (exitCode == EXIT_SUCCESS) {
		log_info(logEntrenador, "ENTRENADOR connected to MAPA successfully\n");
		puts("Se ha conectado correctamente");
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
	int port = 0;
	char *ip = string_new();

	switch (processToConnect) {
	case MAPA: {
		string_append(&ip, "127.0.0.1"); //Esto obvio que despues lo va a levantar de la pokedex pero lo hice estatico para probar
		port = 5000;
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
