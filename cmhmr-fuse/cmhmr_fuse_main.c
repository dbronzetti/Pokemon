/*
 [Fuse nueva Version] Filesystem
*/

#include <fuse/fuse.h>
#include <fuse/fuse_common.h>
#include <fuse/fuse_compat.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux

/* para pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#define PATHLEN_MAX 1024


//#include "cache.c"

static char initial_working_dir[PATHLEN_MAX+1] ={ '\0' };
static char cached_mountpoint[PATHLEN_MAX+1] ={ '\0' };
static int save_dir;

#ifdef NEVER

static char *local=".";
static inline const char *rel(const char *path)
{
  if(strcmp(path,"/")==0)
    return local;
  else
    return (path+1);
}
#endif

const char *full(const char *path) /* Anade un punto de montaje */;//Esto da un warning no importa!
const char *full(const char *path) /* Anade un punto de montaje */
{

  char *ep, *buff;

  buff = strdup(path+1); if (buff == NULL) exit(1);
  //strdup: Esta función devuelve una String compuesta de caracteres repetidos.
  //El carácter que conforma la cadena es el primer carácter del argumento Character y
  //se duplica tantas veces como indique Number.

  ep = buff + strlen(buff) - 1; if (*ep == '/') *ep = '\0'; /* Puede que esto no suceda pero ante la duda mejor prevenia */

  if (*buff == '\0') strcpy(buff, "."); /* La magia! */

  return buff;
}

static int fuse_getattr(const char *path, struct stat *stbuf)
{
    int res;

	path = full(path);
	printf("[Fuse] getattr(%s)\n", path);
    res = lstat(path, stbuf);

    /*lstat; Estas  funciones  devuelven información del fichero especificado. No se
    necesitan derechos de acceso al fichero para conseguir  la  información
    pero  sí  se  necesitan derechos de búsqueda para todos los directorios
    del camino al fichero.

    lstat examina el fichero al que apunta file_name y llena buf y trate  de  un  enlace
    simbólico,  en  cuyo  caso se examina el enlace mismo, no el fichero al
    que hace referencia. */

    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_access(const char *path, int mask)
{
    int res;

    path = full(path);
	printf("[Fuse] access(%s)\n", path);
    res = access(path, mask);

	/* Access:  La función de acceso comprueba si el archivo llamado por nombre de archivo se puede acceder
	 * de la manera especificada por el argumento de la forma.
	 * Mask : El argumento de cómo puede ser o bien el operador OR de las banderas
	 * R_OK, W_OK, X_OK, o la prueba de la existencia F_OK.

	   Esta función utiliza el usuario real y los ID de grupo del proceso actual, en lugar de los
	   identificadores efectivos, para comprobar si hay permiso de acceso. Como resultado, si se utiliza
	   la función de un programa setuid o setgid (ver cómo el cambio de Persona), que proporciona
	   información relativa al usuario que en realidad se acabó el programa.

		El valor de retorno es 0 si está permitido el acceso, y -1 en caso contrario.
	 * */

    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_readlink(const char *path, char *buf, size_t size)
{
    int res;

    path = full(path);
	printf("[Fuse] readlink(%s)\n", path);
    res = readlink(path, buf, size - 1);

    /* Readlink  pone  los  contenidos del enlace simbólico camino en el búfer
       buf, que tiene de tamaño tambuf.  readlink no añade un carácter  NUL  a
       buf.    Truncará   los   contenidos   (hasta  una  longitud  de  tambuf
       caracteres), en caso de que el búfer sea demasiado pequeño para  alojar
       t odo el contenido.

       La  llamada  devuelve  el  número  de caracteres puestos en el búfer si
       acaba con éxito, o un  -1  si  ocurre  un  error,  poniendo  el  código
       correspondiente de error en errno.

     * */

    if (res == -1)
        return -errno;

    buf[res] = '\0';
    return 0;
}


static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    path = full(path);
    printf("[Fuse] readdir(%s)\n", path);
    dp = opendir(path);

    /* La  función  opendir()  abre  un flujo de directorio correspondiente al
       directorio nombre, y devuelve un puntero al  flujo  de  directorio.  El
       flujo se sitúa en la primera entrada del directorio.

       La  función opendir() devuelve un puntero al flujo de directorio o NULL
       si ocurre un error.

     * */

    if (dp == NULL)
        return -errno;


    /* Readdir()  devuelve un puntero a una estructura dirent que
       representa la siguiente entrada de directorio en el flujo de directorio
       al que apunte dir.  Devuelve NULL cuando alcanza el fin-de-fichero o si
       hay un error.
       Los datos devueltos por readdir() pueden ser sobreescritos por llamadas
       posteriores a readdir() para el mismo flujo de directorio.

       Por cada entrada del directorio se debe llamar a la función filler().
     * */

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        /*	Es una forma rapida de llenar un espacio de memoria con un valor char.
			En vez de usar un loop tipo for o while, invocas a la funcion y listo. */
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    /*Esto no necesita mucha explicacion jeje */
    return 0;
}

static int fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    path = full(path);
    printf("[Fuse] mknod(%s)\n", path);

    /*
     * Para comprobar el tipo de fichero
    	if (S_ISLNK (datosFichero.st_mode)) es cierto si el fichero es un link simbólico.
    	if (S_ISREG (datosFichero.st_mode)) es cierto si el fichero es un fichero normal (REGular)
    	if (S_ISDIR (datosFichero.st_mode)) es cierto si el fichero es un directorio.
     * */

    if (S_ISREG(mode)) {
        res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (res >= 0)
            res = close(res);
    } else if (S_ISFIFO(mode)) // prueba si es un PIPE o un archivo especial FIFO
        res = mkfifo(path, mode);

    	/*mkfifo crea FIFOs (también llamados  "tuberías  con  nombre")  con  los
       nombres de fichero especificados.
       Un  "FIFO"  es  un  tipo  de  fichero  especial  que permite a procesos
       independientes comunicarse.  Un  proceso  abre  el  fichero  FIFO  para
       escribir,  y  otro  para leer, tras lo cual los datos pueden fluir como
       con las tuberías sin nombre usuales en shells o donde sea.
       OPCIONES (-m modo, --mode=modo): Establece  los  permisos  de los FIFOs
       creados a modo, que puede
	   ser simbólico como en chmod(1) y emplea el  modo  predeterminado
	   como punto de partida.
    	*/

    else
        res = mknod(path, mode, rdev);

    	/*
       mknod crea un  FIFO  (tubería  con  nombre),  un  fichero  especial  de
       bloques,   o   un   fichero  especial  de  caracteres,  con  el  nombre
       especificado.
       La orden mknod es la que crea ficheros de este tipo.

       El argumento que sigue  a  nombre  especifica  el  tipo  de  fichero  a
       construir:
              p      para un FIFO
              b      para un fichero especial de bloques (con búfer)
              c      para un fichero especial de caracteres (sin búferes)
		OPCIONES (-m modo, --mode=modo) Establece  los  permisos  de los ficheros
		creados a modo, que es simbólico como en chmod(1) y emplea el modo predeterminado como
        punto de partida.
    	 * */

    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_mkdir(const char *path, mode_t mode)
{
    int res;

    path = full(path);
    printf("[Fuse] mkdir(%s)\n", path);
    res = mkdir(path, mode);

    /*Mkdir crea directorios con los nombres especificados.
       De forma predeterminada, los permisos de los  directorios  creados  son
       0777 (`a+rwx') menos los bits puestos a 1 en la umask.
	   OPCIONES (-m modo, --mode=modo)
              Establece  los  permisos  de los directorios creados a modo, que
              puede ser simbólico como en chmod(1) y entonces emplea  el  modo
              predeterminado como el punto de partida.
       -p, --parents
              Crea  los  directorios  padre  que  falten  para  cada argumento
              directorio.  Los permisos para los directorios padre se ponen  a
              la umask modificada por `u+rwx'.  No hace caso de argumentos que
              correspondan  a  directorios  existentes.  (Así,  si  existe  un
              directorio  /a,  entonces `mkdir /a' es un error, pero `mkdir -p
              /a' no lo es.)
       --verbose
              Muestra un mensaje para cada directorio creado. Esto es más útil
              con --parents.
     */
    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_unlink(const char *path)
{
    int res;

    path = full(path);
    printf("[Fuse] unlink(%s)\n", path);
    res = unlink(path);

    /* Unlink borra un nombre del sistema de ficheros. Si dicho nombre era  el
       último  enlace a un fichero, y ningún proceso tiene el fichero abierto,
       el fichero  es  borrado  y  el  espacio  que  ocupaba  vuelve  a  estar
       disponible.
       Si  el  nombre  era  el  último enlace a un fichero, pero algún proceso
       sigue teniendo el fichero abierto, el fichero seguirá existiendo  hasta
       que el último descriptor de fichero referente a él sea cerrado.
       Si  el  nombre  hacía  referencia  a  un enlace simbólico, el enlace es
       eliminado.
       Si el nombre hacía referencia a  un  socket,  fifo  o  dispositivo,  el
       nombre  es  eliminado,  pero  los procesos que tengan el objeto abierto
       pueden continuar usándolo.
   	   VALOR DEVUELTO En caso de éxito, se devuelve cero. En caso de  error,
   	   se devuelve  -1 y se establece el errno apropiado.
     * */

    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_rmdir(const char *path)
{
    int res;

    path = full(path);
    printf("[Fuse] rmdir(%s)\n", path);
    res = rmdir(path);

    /* Rmdir borra directorios vacíos.
       Si un argumento directorio no se refiere a un  directorio  existente  y
       vacío, es un error.
     * */

    if (res == -1)
        return -errno;

    return 0;
}


static int fuse_rename(const char *from, const char *to)
{
    int res;

    from = full(from);
    to = full(to);
    printf("[Fuse] rename(%s, %s)\n", from, to);
    res = rename(from, to);
    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_link(const char *from, const char *to)
{
    int res;

    from = full(from);
	to = full(to);
	printf("[Fuse] link(%s, %s)\n", from, to);
    res = link(from, to);

    /*La función de enlace hace un nuevo enlace al archivo existente denominado
      por nombre_antiguo, bajo el nuevo nombre nuevo.
	  Esta función devuelve un valor de 0 si tiene éxito y -1 en caso de error.
     */

    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_chmod(const char *path, mode_t mode)
{
    int res;

    path = full(path);
    printf("[Fuse] chmod(%s)\n", path);
    res = chmod(path, mode);
    if (res == -1)
        return -errno;

    return 0;
}

static int fuse_chown(const char *path, uid_t uid, gid_t gid)
{
    int res;

    path = full(path);
    printf("[Fuse] lchown(%s)\n", path);
    res = lchown(path, uid, gid); /*hace falta decir que otorga permisos a un archivo?*/
    if (res == -1)
        return -errno;

    return 0;
}


static int fuse_open(const char *path, struct fuse_file_info *fi) {
    int res;

    path = full(path);
	printf("[Fuse] open(%s)\n", path);
    res = open(path, fi->flags);

    /* Este comando abre un archivo, puerto serie, o canalización de comandos y vuelve
       un identificador de canal que puede ser utilizado en futuras ejecuciones de comandos
       como leer, grabar, y se cierran. Si el primer carácter del nombre de archivo no es
       | a continuación, el comando abre un archivo: Nombre de archivo da el nombre del archivo que se
       abrir, y debe ajustarse a las convenciones descritas en el nombre del archivo
       entrada manual*/

    if (res == -1)
        return -errno;

    close(res);
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    int fd;
    int res;

    (void) fi;
    path = full(path);
	printf("[Fuse] read(%s)\n", path);
    fd = open(path, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    /* pread() lee hasta count bytes del descriptor de fichero fd a partir  de
       la posición offset (desde el principio del fichero) en el área temporal
       que empieza en buf.  La posición del puntero  de  L/E  del  fichero  no
       cambia. En caso de éxito se devuelve el número de bytes leídos
       */
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int fuse_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    int fd;
    int res;

    (void) fi;
    path = full(path);
    printf("[Fuse] write(%s)\n", path);
    fd = open(path, O_WRONLY);
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    /* pwrite()  escribe  hasta count bytes desde el buffer que empieza en buf
       al descriptor de fichero  fd  a  partir  de  la  posición  offset.   La
       posición del puntero del fichero no cambia.
       En caso de éxito se devuelve el número de bytes escritos.
       */
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int fuse_statfs(const char *path, struct statvfs *stbuf)
{
    int res;

    path = full(path);
    printf("[Fuse] statvfs(%s)\n", path);
    res = statvfs(path, stbuf);
    /*statfs devuelve información de un sistema de ficheros montado.  path el
       el camino de cualquier fichero en el sistema de ficheros montado.   buf
       es un puntero a una estructura statfs definida como sigue:

              struct statfs {
                 long    f_type;     tipo sistema ficheros (ver bajo)
                 long    f_bsize;    tamaño óptimo de bloque
                                     de transferencia
                 long    f_blocks;   total de bloques de datos en el sistema
                                        de ficheros
                 long    f_bfree;    bloques libres en el sf
                 long    f_bavail;    bloques libres disponibles para
                                        no-superusuarios
                 long    f_files;    total de nodos de ficheros en el sf
                 long    f_ffree;     nodos de ficheros libres en el sf
                 fsid_t  f_fsid;     id del sistema de ficheros
                 long    f_namelen;   longitud máxima de nombre de ficheros
                 long    f_spare[6];  de sobra, para más tarde
              };*/
    if (res == -1)
        return -errno;

    return 0;
}


#ifdef HAVE_SETXATTR
/* Esto es recontra opcional pero ya que estaba lo puse */
static int fuse_setxattr(const char *path, const char *name, const char *value,
                        size_t size, int flags) {
    int res;
    path = full(path);
    printf("[Fuse] setxattr(%s)\n", path);
    res = lsetxattr(path, name, value, size, flags);
    if (res == -1)
        return -errno;
    return 0;
}

static int fuse_getxattr(const char *path, const char *name, char *value,
                    size_t size) {
    int res;

    path = full(path);
	printf("[Fuse] getxattr(%s)\n", path);
    res = lgetxattr(path, name, value, size);
    if (res == -1)
        return -errno;
    return res;
}

static int fuse_listxattr(const char *path, char *list, size_t size) {
    int res;

    path = full(path);
    printf("[Fuse] listxattr(%s)\n", path);
    res = llistxattr(path, list, size);
    if (res == -1)
        return -errno;
    return res;
}

static int fuse_removexattr(const char *path, const char *name) {
    int res;

    path = full(path);
    printf("[Fuse] removexattr(%s)\n", path);
    res = lremovexattr(path, name);
    if (res == -1)
        return -errno;
    return 0;
}
#endif /* HAVE_SETXATTR */

static void *fuse_init(void)
{
  printf("[Fuse] init()\n");
  // truco para permitir el montaje como una superposición
  fchdir(save_dir);
  close(save_dir);
  return NULL;
}

static struct fuse_operations xmp_oper = {
    .init       = fuse_init,
    .getattr	= fuse_getattr,
    .access		= fuse_access,
    .readlink	= fuse_readlink,
    .readdir	= fuse_readdir,
    .mknod		= fuse_mknod,
    .mkdir		= fuse_mkdir,
    .unlink		= fuse_unlink,
    .rmdir		= fuse_rmdir,
    .rename		= fuse_rename,
    .link		= fuse_link,
    .chmod		= fuse_chmod,
    .chown		= fuse_chown,
    .open		= fuse_open,
    .read		= fuse_read,
    .write		= fuse_write,
    .statfs		= fuse_statfs,

	#ifdef HAVE_SETXATTR
		.setxattr	= fuse_setxattr,
		.getxattr	= fuse_getxattr,
		.listxattr	= fuse_listxattr,
		.removexattr= fuse_removexattr,
	#endif
};



int main(int argc, char *argv[])
{
int rc;

    umask(0);
    getcwd(initial_working_dir, PATHLEN_MAX);
    printf("[Fuse] cwd=%s\n", initial_working_dir);

    printf("[Fuse] main(%s, %s, %s, %s)\n", argv[0], argv[1], argv[2], argv[3]);
    strncpy(cached_mountpoint, argv[1], strlen(argv[1]));
    printf("[Fuse] mountpoint=%s\n", cached_mountpoint);

    save_dir = open(cached_mountpoint, O_RDONLY);
    rc = fuse_main(argc, argv, &xmp_oper, NULL);

    printf("[Fuse] umount(%s, %s, %s, %s)\n", argv[0], argv[1], argv[2], argv[3]);

    return rc;
}
