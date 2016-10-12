/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr


#include <fuse.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <commons/string.h>

int* pmap_pikachu;
int* pmap_squirtle;
int* pmap_bulbasaur;
struct stat pikachuStat;
struct stat squirtleStat;
struct stat bulbasaurStat;

char *ruta;
char *contenido;
char** carpetas;


static int ejemplo_getattr(const char *path, struct stat *stbuf) {
	int res = 0;
	int i = 0;
	memset(stbuf, 0, sizeof(struct stat));

	//strcat('/', ruta);
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		} else {
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 1;
		//res = -ENOENT;
	}


	return res;
}

static int ejemplo_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {

	int res = 0;
	int i=0;

	if (strcmp(path, "/") == 0) {
		filler(buf, "pikachu2", NULL, 0);
		for (i=0; i<=2;i++){

			filler(buf, carpetas[i], NULL, 0);
		}
		filler(buf, "pepito.txt", NULL, 0);
	} else {
		res = -ENOENT;
	}

	return res;
}


static int ejemplo_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	//char *rutaCompleta=malloc((strlen("/pepito.txt")+1));
	//memcpy(rutaCompleta, "/", strlen("/")+1);

	//printf("ruta!!!!!: %s\n", ruta);
	//strcat(rutaCompleta, ruta);
	//printf("rutaCompleta!!!!!: %s\n", rutaCompleta);
	/*
	if (strcmp(path, rutaCompleta) == 0) {
		//memcpy(buf,"Hola\n",size);
		memcpy(buf, contenido, size);
	}
	*/
	if (strcmp(path, "/pepito.txt") == 0) {
			memcpy(buf,"Hola\n",size);
		}
	return size;
}

static struct fuse_operations ejemplo_oper = {
		.getattr = ejemplo_getattr,
		.readdir = ejemplo_readdir,
		.read = ejemplo_read,
};



int main(int argc , char *argv[])
{

    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    ruta = malloc((strlen("pepito.txt")+1));
    contenido = malloc((strlen("holitas\n")+1));
    char** substrings;


    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Conectado\n");

    //keep communicating with server
    while(1)
    {

    	strcpy(message, "kk");
		if( send(sock , message , strlen(message) , 0) < 0)
		{
			puts("Send failed");
			//return 1;
		}


        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break; //por el while
        }

       // puts("Server reply :");
        puts(server_reply);
        substrings = string_split(server_reply, "|");
        carpetas = string_split(substrings[0], ",");

        printf("substrings[0]: %s\n", substrings[0]);
        printf("carpetas[0]: %s\n", carpetas[0]);
        printf("carpetas[2]: %s\n", carpetas[2]);
    	//strcpy(ruta, substrings[0]);
        printf("substrings[1]: %s\n", substrings[1]);
    	//strcpy(contenido, substrings[1]);
    	printf("copi\n");

        //if(strcmp(ruta,"pepito.txt")==0){
        	printf("PEPITO ES IGUAL\n");
        	close(sock);
        	printf("llego pepito\n");
        	return fuse_main(argc, argv, &ejemplo_oper, NULL );
        //}


    }

    //close(sock);
    printf("termino\n");
	//return fuse_main(argc, argv, &ejemplo_oper, NULL );

   // return 0;
}
