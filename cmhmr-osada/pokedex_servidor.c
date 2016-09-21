/*
 ============================================================================
 Name        : servidor.c
 Author      : joel
 Version     :
 Copyright   : joel es bueno
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

    #define SOCK_PATH "echo_socket"

	void crearServidor() {
		  int s, s2, t, len;
		        struct sockaddr_un local, remote;
		        char str[100];

		        //CREO UN SOCKET Y VALIDO SI FUE TODO BIEN
		        if ((s = socket(AF_UNIX, SOCK_STREAM, 0))== -1)
		        {
		            perror("socket");
		            exit(1);
		        }

		        //EL SOCKET LO LIGO A UNA DIRECCION DE DOMINIO
		        local.sun_family = AF_UNIX;
		        strcpy(local.sun_path, SOCK_PATH);
		        unlink(local.sun_path);
		        len = strlen(local.sun_path) + sizeof(local.sun_family);
		        if (bind(s, (struct sockaddr *)&local, len) == -1){
		            perror("bind");
		            exit(1);
		        }

		        //ESCUCHA HASTA 5 CLIENTES, Y LO PONE EN COLA
		        if (listen(s, 5) == -1) {
		        	perror("listen");
		            exit(1);
		        }

		        for(;;) {
		            int done, n;
		            printf("esperando una conexión...\n");
		            t = sizeof(remote);

		            //ACEPTO EL CLIENTE
		            if((s2 = accept(s, (struct sokaddr *)&remote, &t)) == -1) {
		                perror("accept");
		                exit(1);
		            }

		            printf("Conectado.\n");
		            done = 0;


		            do {
		                //recibe
		            	n = recv(s2, str, 100, 0);
		                if (n <= 0) {
		                    if (n < 0) perror("recv");
		                    done = 1;
		                }
		                if (!done)
		                	//envie
		                    if (send(s2, str, n, 0) < 0) {
		                        perror("send");
		                        done = 1;
		                    }
		            } while (!done);

		            close(s2);
		}

	}

    int main(void)
    {
    	crearServidor();
        return EXIT_SUCCESS;
    }
