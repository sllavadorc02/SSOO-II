#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/ipc.h>
#include <sys/wait.h>    
#include <sys/msg.h> 
#include <time.h>


#define NUM_HIJOS 10
pid_t pid_padre;
pid_t pid_hijos[NUM_HIJOS];
int buzon;

struct mensaje{
    long tipo;
    int dato;
};

int randomm(){
    return rand()%10;
}

// Manejador de SIGINT (Ctrl + C)
void manejadora_salida(int sig){
    if(getpid()==pid_padre){
        printf("\n Ctrl + C detectado. Eliminando semáforo y terminando procesos...\n");
    }
    
    for(int i=0; i<NUM_HIJOS; i++){
        if(pid_hijos[i]>0){
            kill(pid_hijos[i], SIGKILL);  // Mata a todos los procesos del grupo, incluyendo hijos
        }
    }

   
    if(getpid()==pid_padre){
        if(msgctl(buzon, IPC_RMID, NULL)==-1){
            printf("Error al eliminar el buzon");
            return;
        }
    
        exit(0);
    }
    
    //matar hijos primero y despues eliminar semaforo para que no se quede ninguno dentro del semáforo
    
}


int main(void){
    buzon=msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    int i;
    pid_padre=getpid();
    

    struct sigaction sa;
    sa.sa_handler = manejadora_salida;
    sigemptyset(&sa.sa_mask);   //se redirige la accion de sigint (sin bloquear ninguna señal)
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);


    if(buzon < 0){
        printf("error al crear un buzon");
        return -1;
    }


    pid_t pid;
    for(i=0; i<NUM_HIJOS; i++){
        pid=fork();
        if(pid<0){
            printf("Error al crear un proceso hijo");
            return -1;
        }else if(pid==0){   //es el hijo
            srand(getpid());

            struct mensaje men;
            men.tipo=11;
            men.dato=randomm();
            printf("%d\n", men.dato);

            if(msgsnd(buzon, &men, sizeof(men)-sizeof(long), 0) == -1){
                printf("Error al enviar mensaje");
                return -1;
            }
            
            exit(0); //los hijos envian y se mueren
        }else{
            pid_hijos[i]=pid;
        }
    }


    
  
    if(getpid()==pid_padre){
        struct mensaje rec;
        int suma=0;

        for(i=0; i<NUM_HIJOS; i++){
            if(msgrcv(buzon, &rec, sizeof(rec)-sizeof(long), 11, 0)==-1){
                printf("Error al recibir mensaje");
                return -1;
            }

            suma=suma+rec.dato;
        }

        printf("La suma de todos los numeros es: %d\n", suma);
      
    }



    for (i = 0; i < NUM_HIJOS; i++) {
        waitpid(pid_hijos[i], NULL, 0);  // espero a que terminen todos los hijos
    }

    
    if(msgctl(buzon, IPC_RMID, NULL)==-1){
        printf("Error al eliminar el buzon");
        return -1;
    }
    

    return 0;   
}

