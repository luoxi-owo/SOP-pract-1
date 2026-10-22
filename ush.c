/*-----------------------------------------------------+
 |     U S H. C                                        
 +-----------------------------------------------------+
 |     Versión :                                       |                      
 |     Autor :                                         |
 |     Asignatura :  SOP-GIIROB                        |                               
 |     Descripción :                                   |
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "profe.h"

#include <stdio.h>


//
// Declaraciones de funciones locales
//
void visualizar( void );
int leerLinea( char *linea, int maxLinea );

//
// Prog. ppal.
// 
int main(int argc, char * argv[]){
    
  char line[255];
  int res;
  char **m_ordenes;
  char ***m_argumentos;
  int *m_num_arg;
  int m_n;

  while(1)
  {
    
    do
    {
        res=leerLinea(line,MAXLINE);    
        if (res==-2) {
      		fprintf(stdout,"logout\n");
		      exit(0);
        }
	
    	  if (res==-1){
		      fprintf(stdout,"linea muy larga\n");
	      }
     }while(res<=0);

     if (analizar(line)==OK){
       m_n=num_ordenes();
	     m_num_arg=num_argumentos();
	     m_ordenes=get_ordenes();
	     m_argumentos=get_argumentos();
	     if(m_n>0)
	     {
          if (pipeline(m_n,fich_entrada(),fich_salida(),es_append(),es_background())==OK)
                    ejecutar(m_n,m_num_arg,m_ordenes,m_argumentos,es_background());
        }

       visualizar();
     }
 }    

  return 0;
}





/****************************************************************/
/*                       leerLinea                             
  --------------------------------------------------------------
                                                               
   DESCRIPCIÓN:                                                 
   Obtiene la línea de órdenes para el mShell.    
   Util para depuracion.                                        
                                                                
   ENTRADA:                                                 
    linea - puntero a un vector de carácteres donde se almancenan los caracteres 
   leídos del teclado
    tamanyLinea - tamaño máximo del vector anterior

   SALIDA:
    -- linea - si termina bien, contiene como último carácter el retorno de carro.
    -- leerLinea -  Entero que representa el motivo de finalización de la función:
     > 0 - terminación correcta, número de caracteres leídos, incluído '\n'
     -1 - termina por haber alcanzado el número máximo de caracteres que se 
    espera leer de teclado, sin encontrar '\n'.
     -2 - termina por haber leído fin de fichero (EOF).
*/
/****************************************************************/
//char * getline(void)
int leerLinea( char *linea, int maxLinea )
{	
	//inicio contador de bucle
	int i=0;

	int c=0;
	char cwd[1024];
	
	char* a=getcwd(cwd,sizeof(cwd));


	printf("\033[1;35m%s\033[0m:\033[1;34m%s\033[0m$ ", PROMPT, a); 

	do{
	//Conseguir caracter, guardarlo en la linea.
		if(i>=maxLinea){
			while (c != '\n'&& c != EOF) c=getchar(); /*vacia el resto de la linea*/
			return -1;
		}

		c=getchar();
		linea[i]=c;
		if(c==EOF){
			return -2;	
		}
		i++;
		
	}while(c!='\n');
	return i;
	 
}




/****************************************************************/
/*                       visualizar                             */
/*--------------------------------------------------------------*/
/*                                                              */
/* DESCRIPCIÓN:                                                 */
/* Visualiza los distintos argumentos de la orden analizada.    */
/* Util para depuracion.                                        */
/*                                                              */
/* ENTRADA: void                                                */
/*                                                              */
/* SALIDA: void                                                 */
/*                                                              */
/****************************************************************/
void visualizar( void )
{  	
	int n=num_ordenes();
	printf("Numero de ordenes: %d \n",n);

	char **ordenes=get_ordenes();
	int i=0;
	int j=0;

	int *na=num_argumentos();
	
	char ***g_arg=get_argumentos();
	
	char *f_s=fich_salida();
	char *f_e=fich_entrada();
	
    printf("\n");
    printf("   /\\_/\\     +-----------------------------------+\n");
    printf("  ( o.o )    |         VISUALIZAR ORDEN          |\n");
    printf("   > ^ <     +-----------------------------------+\n");

	
	while(i<n){
		printf("\033[1m-->Orden %d: %s\033[0m\n", i+1, ordenes[i]);	
		
		printf("-->Numero de argumentos: %d \n",na[i]);

		j=0; /*inicializamos la variable para el bucle interior*/
				
		while(j<na[i]){
			printf("-->Los argumentos son: %s \n",g_arg[i][j]);
			j++;
		
		}	
		i++;
		

		
	}

	if(f_e[0]=='\0'){
		printf("-->No hay fichero de entrada\n");
	}else{
		printf("-->Fichero entrada: %s \n",f_e);
	}

	if(f_s[0]!='\0'){
		printf("-->Fichero salida: %s \n",f_s);
		int mod=es_append();
		if(mod==1){
			printf("-->Modo append \n");
		}
		else if(mod==0){
			printf("-->Modo Trunc \n");
		}else{
		printf("-->Modo desconocido \n");
		}
	}else{
		printf("-->No hay redireccion de salida \n");
	}

	if (es_background()==1){
		printf("-->Esta siendo ejecutado en background \n");
	}else{
		printf("-->Esta siendo ejecutado en foreground \n");
	}

} // Fin de "visualizar"



