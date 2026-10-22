/*-----------------------------------------------------+
 |     E J E C U C I O N . C                           |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion: creacion de procesos y gestion de  |
 |                  ordenes en segundo plano (&)       |
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
 
 
/*-----------------------------------------------------+
 |  TABLA DE TRABAJOS EN BACKGROUND                     |
 +-----------------------------------------------------*/
#define MAXTRABAJOS 20
 
typedef struct {
	int   usado;                 /* 1 si la posicion esta ocupada              */
	pid_t pids[PIPELINE];        /* pids de los procesos del pipeline (0=ya recogido) */
	int   vivos;                 /* procesos que aun no se han recogido        */
	pid_t ultimo;                /* pid de la ultima orden del pipeline        */
	int   estado;                /* estado de terminacion de la ultima orden   */
	int   terminado;             /* 1 cuando se han recogido todos sus procesos*/
	char  linea[MAXLINE+1];      /* linea de ordenes, para el mensaje final    */
} TRABAJO;
 
static TRABAJO trabajos[MAXTRABAJOS];
static int manejador_instalado = 0;
 
 
/* Manejador de SIGCHLD: recoge (waitpid) los procesos de background que han  */
/* terminado, para que no queden zombies. Solo espera por pids de la tabla,   */
/* asi nunca le "roba" un hijo al waitpid de las ordenes en foreground.       */
/* No imprime nada (printf no es seguro dentro de un manejador): solo anota.  */
static void manejador_sigchld(int sig)
{
	int t, k, st;
	int errno_guardado = errno;   /* waitpid puede cambiar errno */
 
	for (t = 0; t < MAXTRABAJOS; t++) {
		if (!trabajos[t].usado || trabajos[t].terminado) continue;
		for (k = 0; k < PIPELINE; k++) {
			if (trabajos[t].pids[k] <= 0) continue;
			if (waitpid(trabajos[t].pids[k], &st, WNOHANG) == trabajos[t].pids[k]) {
				if (trabajos[t].pids[k] == trabajos[t].ultimo) trabajos[t].estado = st;
				trabajos[t].pids[k] = 0;
				trabajos[t].vivos--;
				if (trabajos[t].vivos == 0) trabajos[t].terminado = 1;
			}
		}
	}
	errno = errno_guardado;
}
 
 
static void instalar_manejador(void)
{
	struct sigaction sa;
 
	sa.sa_handler = manejador_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;     /* getchar/waitpid se reanudan solos si llega la senyal */
	if (sigaction(SIGCHLD, &sa, NULL) == -1) perror("sigaction");
	manejador_instalado = 1;
}
 
 
/* Concatena src al final de dst sin pasarse del tamanyo del buffer */
static void anyadir(char *dst, const char *src)
{
	strncat(dst, src, MAXLINE - strlen(dst));
}
 
 
/* Reconstruye la linea de ordenes: cmd args [< f] | cmd args [> f | >> f] &  */
static void construir_linea(char *dst, int nordenes, int *nargs, char ***args)
{
	int i, j;
 
	dst[0] = '\0';
	for (i = 0; i < nordenes; i++) {
		if (i > 0) anyadir(dst, " | ");
		for (j = 0; j < nargs[i]; j++) {
			if (j > 0) anyadir(dst, " ");
			anyadir(dst, args[i][j]);
		}
		if (i == 0 && fich_entrada()[0] != '\0') {
			anyadir(dst, " < ");
			anyadir(dst, fich_entrada());
		}
	}
	if (fich_salida()[0] != '\0') {
		anyadir(dst, es_append() ? " >> " : " > ");
		anyadir(dst, fich_salida());
	}
	anyadir(dst, " &");
}
 
 
int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{
	int i, k, aux, t = -1;
	pid_t pids[PIPELINE];
	sigset_t mascara, anterior;
 
	if (!manejador_instalado) instalar_manejador();
 
	/* Bloqueamos SIGCHLD mientras creamos los hijos y los apuntamos en la tabla. */
	/* Si un hijo muy rapido terminara antes de estar apuntado, el manejador no   */
	/* lo encontraria y se quedaria zombie. La senyal queda pendiente y llega al  */
	/* desbloquear.                                                               */
	sigemptyset(&mascara);
	sigaddset(&mascara, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mascara, &anterior);
 
	if (bgnd) {
		for (t = 0; t < MAXTRABAJOS && trabajos[t].usado; t++);
		if (t == MAXTRABAJOS) {
			fprintf(stderr, "Demasiadas ordenes en background\n");
			cerrar_fd();
			sigprocmask(SIG_SETMASK, &anterior, NULL);
			return ERROR;
		}
	}
 
	for(i=0;i<nordenes ;i++){
		aux=fork();
		if(aux == -1){ /* comprobamos si no crea el hijo*/
			perror("fork");
			cerrar_fd();
			for (k = 0; k < i; k++) waitpid(pids[k], NULL, 0); /* recoger los ya creados */
			sigprocmask(SIG_SETMASK, &anterior, NULL);
			return ERROR;
		}
		if(aux==0){
			sigprocmask(SIG_SETMASK, &anterior, NULL); /* el hijo no hereda SIGCHLD bloqueada */
			if (redirigir_entrada(i) == ERROR || redirigir_salida(i) == ERROR){
				_exit(1);
			}
			cerrar_fd();
			args[i][nargs[i]] = NULL;   /* execvp necesita el vector terminado en NULL */
			execvp(ordenes[i],args[i]);
			fprintf(stderr,"Ha habido un error en la orden %d: %s\n",i,ordenes[i]); /* Si el exec falla lanza el print de error y hace exit para que el hijo no se quede vivo*/
			_exit(1);
		}
		pids[i] = aux;
	}
 
	cerrar_fd();
 
	if (bgnd) {
		/* Apuntamos el trabajo en la tabla y avisamos como bash: [n] pid */
		for (k = 0; k < PIPELINE; k++) trabajos[t].pids[k] = (k < nordenes) ? pids[k] : 0;
		trabajos[t].vivos     = nordenes;
		trabajos[t].ultimo    = pids[nordenes-1];
		trabajos[t].estado    = 0;
		trabajos[t].terminado = 0;
		construir_linea(trabajos[t].linea, nordenes, nargs, args);
		trabajos[t].usado     = 1;
		printf("[%d] %d\n", t+1, (int)pids[nordenes-1]);
		sigprocmask(SIG_SETMASK, &anterior, NULL);
	} else {
		sigprocmask(SIG_SETMASK, &anterior, NULL);
		/* Esperamos SOLO a los hijos de esta orden. Con wait(NULL) tambien se    */
		/* esperaria a los de background y el shell se quedaria bloqueado.         */
		for (k = 0; k < nordenes; k++)
			while (waitpid(pids[k], NULL, 0) == -1 && errno == EINTR);
	}
 
	return OK;
} // Fin de la funcion "ejecutar"
 
 
void informar_bg(void)
{
	int t, st;
	char texto[32];
	sigset_t mascara, anterior;
 
	/* Bloqueamos SIGCHLD para que el manejador no modifique la tabla mientras la leemos */
	sigemptyset(&mascara);
	sigaddset(&mascara, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mascara, &anterior);
 
	for (t = 0; t < MAXTRABAJOS; t++) {
		if (!trabajos[t].usado || !trabajos[t].terminado) continue;
		st = trabajos[t].estado;
		if (WIFEXITED(st) && WEXITSTATUS(st) == 0)  sprintf(texto, "Done");
		else if (WIFEXITED(st))                     sprintf(texto, "Exit %d", WEXITSTATUS(st));
		else if (WIFSIGNALED(st))                   sprintf(texto, "Killed");
		else                                        sprintf(texto, "Terminado");
		printf("[%d]  %-22s %s\n", t+1, texto, trabajos[t].linea);
		trabajos[t].usado = 0;   /* liberamos la posicion */
	}
	fflush(stdout);
 
	sigprocmask(SIG_SETMASK, &anterior, NULL);
} // Fin de la funcion "informar_bg"
