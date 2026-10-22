/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Version    :                        
 |     Autor :   
 |     Asignatura :  SOP-GIIROB                                                       
 |     Descripcion: 
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

REDIRECCION_ORDENES red_ordenes;


void redireccion_ini(void)
{
	int i=0;
	while(i<PIPELINE-1){
		red_ordenes[i].entrada=0;
		red_ordenes[i].salida=1;
		i++;
	}
}//Inicializar los valores de la estructura cmdfd



int pipeline(int nordenes, char * infile, char * outfile, int append, int bgnd)
{
	int fd[2];
	int fd1;
	int fd2;
	
	redireccion_ini();
	
	int i=0;
	if (nordenes > PIPELINE-1) {                 
		fprintf(stderr, "Demasiadas ordenes (maximo %d)\n", PIPELINE-1);
		return ERROR;
	}
	if(infile[0]!='\0'){
		fd1 = open(infile,O_RDONLY);

		if(fd1==-1){
			perror(infile);
			return ERROR;
		}
		red_ordenes[0].entrada = fd1;
	}

	if(outfile[0]!='\0'){
		if (append){
			fd2 = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
		} else{
			fd2 = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}

		if (fd2 == -1){
			perror (outfile);
			cerrar_fd();
			return ERROR;
		}
		red_ordenes[nordenes-1].salida = fd2;
	}

	while(i<nordenes-1){
		if(pipe(fd)==-1){
			perror (pipe);
			cerrar_fd();
			return ERROR;
		}
		red_ordenes[i].salida = fd[1];
		red_ordenes[i+1].entrada = fd[0];
		i++;
	}

	if (bgnd && infile[0] == '\0'){
		red_ordenes[0].entrada = open("/dev/null", O_RDONLY);

		if (red_ordenes[0].entrada  == -1){
			perror(("/dev/null"));
			cerrar_fd();
			return ERROR;
		}
	}

	return OK;
		


} // Fin de la función "pipeline"



int redirigir_entrada(int i)
{
	
	if(red_ordenes[i].entrada!= STDIN_FILENO){
		int e=dup2(red_ordenes[i].entrada,STDIN_FILENO);
		if(e==-1){
			perror(dup2);
			return ERROR;
		}
	}
	return OK;
} // Fin de la función "redirigir_entrada"



int redirigir_salida(int i)
{

	if(red_ordenes[i].salida!=STDIN_FILENO){
		int e=dup2(red_ordenes[i].salida,STDOUT_FILENO);
		if(e==-1){
			perror(dup2);
			return ERROR;
		}
	}
	return OK;
} // Fin de la función "redirigir_salida"


int cerrar_fd()
{
	int i=0;
	while(i < PIPELINE -1 ){
		if (red_ordenes[i].entrada >2) close(red_ordenes[i].entrada); // comprobamos que estamos en la posición correcta para no eliminar una posicion erronea
		if (red_ordenes[i].salida >2) close(red_ordenes[i].salida);
		i++;
	}

	return OK;
} // Fin de la función "cerrar_fd"


