/*
 ============================================================================
 Name        : cliente.c
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
#include "cmhmr_fuse.h"


static struct fuse_operations hello_oper = {
		.getattr = hello_getattr,
		.readdir = hello_readdir,
		.open = hello_open,
		.read = hello_read,
};

struct fuse_opt fuse_options[] = {
		// Este es un parametro definido por nosotros
		CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};

#define SOCK_PATH "/home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/cmhmr-osada/Debug/echo_socket"

int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	// Si se paso el parametro --welcome-msg
	// el campo welcome_msg deberia tener el
	// valor pasado
	if( runtime_options.welcome_msg != NULL ){
		printf("%s\n", runtime_options.welcome_msg);
	}

	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	//return fuse_main(args.argc, args.argv, &hello_oper, NULL);
	 return fuse_main(argc, argv, &hello_oper, NULL);

}


void conectarmeAlServidor(){
    int s, t, len;
    struct sockaddr_un remote;
    char str[100];


    if ((s=socket(AF_UNIX, SOCK_STREAM, 0))== -11){
    	perror("socket");
    	exit(1);
    }

    printf("Intentando conectarse....\n");
    remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len)==1){
		perror("connect");
		exit(1);
	}

	printf("Conectado.\n");

	while(printf("> "), fgets(str, 100, stdin), !feof(stdin)) {
				if (send(s, str, strlen(str), 0) == -1) {
	                perror("send");
	                exit(1);
	            }
	            if ((t=recv(s, str, 100, 0)) > 0) {
	                str[t] = '\0';
	                printf("echo> %s", str);
	            } else {
	                if (t < 0) perror("recv");
	                else printf("Servidor cerrando conexión\n");
	                exit(1);
	            }
	        }
	        close(s);

}
/*
int main(void) {
	const char *a[2];
	a[0] = "./cmhmr-fuse";
	a[1] = "/tmp/example";
	crearFuse(2, a);
	//conectarmeAlServidor();
	return EXIT_SUCCESS;
}
*/
