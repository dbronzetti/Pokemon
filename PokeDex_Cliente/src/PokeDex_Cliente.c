/*
 ============================================================================
 Name        : PokeDex_Cliente.c
 ============================================================================
 */

#include "PokeDex_Cliente.h"

int main(int argc, char **argv) {
	char *logFile = NULL;
	char *disco = string_new();
	int socketPokeServer = 0;
	int exitCode = EXIT_FAILURE; //por default EXIT_FAILURE

	assert(("ERROR - NOT arguments passed", argc > 1)); // Verifies if was passed at least 1 parameter, if DONT FAILS

	//get parameter
	int i;
	for (i = 0; i < argc; i++) {

		//check disk file parameter
		if (strcmp(argv[i], "-d") == 0) {
			disco = argv[i + 1];
			printf("El disco a trabajar: '%s'\n", disco);
		}

		//check log file parameter
		if (strcmp(argv[i], "-l") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	logPokeCliente = log_create(logFile, "POKEDEX_CLIENTE", 0, LOG_LEVEL_TRACE);

	//getting environment variable for connecting to server
	IP_SERVER = getenv("POKEIP");
	PORT = atoi(getenv("POKEPORT"));

	exitCode = connectTo(POKEDEX_SERVIDOR, &socketPokeServer);
	if (exitCode == EXIT_SUCCESS) {
		log_info(logPokeCliente, "POKEDEX_CLIENTE connected to POKEDEX_SERVIDOR successfully\n");

	} else {
		log_error(logPokeCliente,"No server available - shutting down proces!!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int connectTo(enum_processes processToConnect, int *socketClient) {
	int exitcode = EXIT_FAILURE; //DEFAULT VALUE
	int port;
	char *ip = string_new();

	switch (processToConnect) {
		case POKEDEX_SERVIDOR: {
			string_append(&ip, IP_SERVER);
			port = PORT;
			break;
		}
		default: {
			log_info(logPokeCliente, "Process '%s' NOT VALID to be connected.\n",getProcessString(processToConnect));
			break;
		}
	}
	exitcode = openClientConnection(ip, port, socketClient);

	//If exitCode == 0 the client could connect to the server
	if (exitcode == EXIT_SUCCESS) {

		// ***1) Send handshake
		exitcode = sendClientHandShake(socketClient, POKEDEX_CLIENTE);

		if (exitcode == EXIT_SUCCESS) {

			// ***2)Receive handshake response
			//Receive message size
			int messageSize = 0;
			char *messageRcv = malloc(sizeof(messageSize));
			int receivedBytes = receiveMessage(socketClient, messageRcv,sizeof(messageSize));

			if (receivedBytes > 0) {
				//Receive message using the size read before
				memcpy(&messageSize, messageRcv, sizeof(int));
				messageRcv = realloc(messageRcv, messageSize);
				receivedBytes = receiveMessage(socketClient, messageRcv,messageSize);

				//starting handshake with client connected
				t_MessageGenericHandshake *message = malloc(sizeof(t_MessageGenericHandshake));
				deserializeHandShake(message, messageRcv);

				free(messageRcv);

				switch (message->process) {
				case ACCEPTED: {
					log_info(logPokeCliente, "Conectado a POKE SERVER - Messsage: %s\n",message->message);
					break;
				}
				default: {
					log_error(logPokeCliente,"Process couldn't connect to SERVER - Not able to connect to server %s. Please check if it's down.\n",ip);
					close(*socketClient);
					break;
				}
				}
			} else if (receivedBytes == 0) {
				//The client is down when bytes received are 0
				log_error(logPokeCliente,"The client went down while receiving! - Please check the client '%d' is down!\n",	*socketClient);
				close(*socketClient);
			} else {
				log_error(logPokeCliente,"Error - No able to received - Error receiving from socket '%d', with error: %d\n",*socketClient, errno);
				close(*socketClient);
			}
		}

	} else {
		log_error(logPokeCliente,"I'm not able to connect to the server! - My socket is: '%d'\n",*socketClient);
		close(*socketClient);
	}

	return exitcode;
}



