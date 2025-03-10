#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include "filosofar.h"

void eliminar_sem(int id_sem){
    int err=semctl(id_sem, 0, IPC_RMID);    
    if(err==-1){
        printf("Error al eliminar semáforo...\n");
        return;
    }

}



void liberar_mem(int shm_id){
    int err=shmctl(shm_id, IPC_RMID, NULL);
    if(err==-1){
        printf("Error al liberar memoria compartida...\n");
        return;
    }
}

int main (int argc, char *argv[]){
    int err;
   //comprobar argumentos pasados al main
   //registrar señales a usar
   //obtener IPCs
   //FI_inicio con clave  41211294392005
   //crear procesos como se indique en el main
   //esperar muerte hijos sin cpu
   //eliminar IPC
   //llamar FI_fin


    
    //numero de filosofos, numero de vueltas por filosofo y lentitud de ejecucion>=0 (4)
    if(argc!=4){
        printf("Numero de parámetros incorrecto...\n");
        return -1;
    }

    int numFil=atoi(argv[1]);
    int numVuel=atoi(argv[2]);
    int lentitud=atoi(argv[3]);
    
    if(numFil<0 || numFil>MAXFILOSOFOS || numVuel<=0 || lentitud<0){
        printf("Parámetros incorrectos...\n");
        return -2;
    }



    struct DatosSimulaciOn ddssp;
    ddssp.maxFilOsofosEnPuente=10;
    ddssp.maxUnaDirecciOnPuente=0;
    ddssp.sitiosTemplo=10;
    ddssp.nTenedores=5;


    int numSemaforos=FI_getNSemAforos();
    int sem_inicio=semget(IPC_PRIVATE, numSemaforos, IPC_CREAT | 0600);
    if(sem_inicio<0){
        printf("Error al crear semaforos...\n");
        return -1;
    }


    int tamMemComp=FI_getTamaNoMemoriaCompartida();
    int shm_inicio=shmget(IPC_PRIVATE, tamMemComp, IPC_CREAT | 0600);
    if(shm_inicio<0){
        printf("Error al crear memoria compartida...\n");
        return -1;
    }


    //INICIO
    unsigned long long clave=41211294392005;
    err=FI_inicio(lentitud, clave, &ddssp, sem_inicio, shm_inicio, NULL);
    if(err ==-1){
        return -1;
    }




    //ELIMINO IPCs
    eliminar_sem(sem_inicio);
    liberar_mem(shm_inicio);



    //FIN
    err=FI_fin();
    if(err<0){
        printf("Error al hacer FI_fin...\n");
        return -1;
    }



   return 0;
}
