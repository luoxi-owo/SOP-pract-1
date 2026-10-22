/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion:                                    |
 +-----------------------------------------------------*/
#include "defines.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <signal.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{	
	int i,aux;
		for(i=0;i<nordenes ;i++){
			aux=fork();
			if(aux == -1){ /* comprobamos si no crea el hijo*/
				perror("fork");
				cerrar_fd();
				return ERROR;
			}
			if(aux==0){
				if (redirigir_entrada(i) == ERROR || redirigir_salida(i) == ERROR){
					exit(1);
				}
				cerrar_fd();
				args[i][nargs[i]] = NULL; /*Vaciamos la memoria por si no viene vacía*/
				execvp(ordenes[i],args[i]);
				fprintf(stderr,"Ha habido un error en la orden %d: %s\n",i,ordenes[i]); /* Si el exec falla lanza el print de error y hace exit para que el hijo no se quede vivo*/
				exit(1);
				}
			}
		
	
	cerrar_fd();
	if (!bgnd) while(wait(NULL)!=-1); /* Esperamos mientras no este en background*/
	
	
	return OK;
} // Fin de la funcion "ejecutar"
