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
	if(bgnd==0){
		int i,aux;
		for(i=0;i<nordenes ;i++){
			aux=fork();
			if(aux==0){
				redirigir_entrada(i);
				redirigir_salida(i);
				cerrar_fd();
				int e=execvp(ordenes[i],args[i]);
				if(e==-1){
					printf("Ha habido un error en la orden %d: %s",i,ordenes[i]);
				}
			}
		}
	
	cerrar_fd();
	while(wait(NULL)!=-1);
	
	}
	return OK;
} // Fin de la funcion "ejecutar"
