#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/shm.h>

int numeroRandom(){
    return rand() % 50;
}

void signall(int id_sem){
    struct sembuf m;
    m.sem_num=0;
    m.sem_op=1;
    m.sem_flg=0;
    int err1=semop(id_sem, &m, 1);
    if(err1==-1){
        printf("Error al hacer signal...\n");
        return;
    }
}

void waitt(int id_sem){
    struct sembuf m;
    m.sem_num=0;
    m.sem_op=-1;
    m.sem_flg=0;
    int err1=semop(id_sem, &m, 1);
    if(err1==-1){
        printf("Error al hacer wait...\n");
        return;
    }
}

void eliminar_sem(int id_sem){
    int err=semctl(id_sem, 0, IPC_RMID);    
    if(err==-1){
        printf("Error al eliminar semáforo...\n");
        return;
    }


}


int main (void){
    pid_t pidPadre=getpid();
    int i;

    //Semaforos:

    int id_sem=semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);    //solicita semáforo
    if(id_sem==-1){
        printf("Error al solicitar semáforos...\n");
        return -1;
    }

    int err=semctl(id_sem, 0, SETVAL, 0);   //iniciar semáforo a 0
    if(err==-1){
        printf("Error al establecer limite en semaforo 0...\n");
        eliminar_sem(id_sem);
        return -1;
    }

    //memoria compartida:

    int shm_id=shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0600);    //solicita memoria
    if(shm_id==-1){
        printf("Error al solicitar memoria compartida...\n");
        eliminar_sem(id_sem);
        return -1;
    }

    int *memoria=(int *)shmat(shm_id, 0, 0);    //puntero a zona compartida
    if(memoria==(int *)-1){
        printf("Error al crear memoria compartida...\n");
        eliminar_sem(id_sem);
        shmctl(shm_id, IPC_RMID, NULL); //libera memoria compartida
        return -1;
    }

    
    pid_t pidHijo;
    pid_t pid=fork();

    if(pid==-1){ 
        printf("Error al crear el hijo...\n");
        return -1;
    }else if(pid==0){       //hijo
        pidHijo=getpid();
        srand(getpid());
        int num1=numeroRandom();
        int num2=numeroRandom();
        int *memoria_h=(int *)shmat(shm_id, 0, 0);  //apunta con puntero a misma zona memoria
        if(memoria_h==(void *)-1){
            printf("Error al asignar memoria compartida al hijo...\n");
            return -1;
        }

        printf("Soy el hijo (%d) y voy a enviar %d y %d \n", getpid(), num1, num2);
        memoria_h[0]=num1;  //guarda en memoria los valores
        memoria_h[1]=num2;  //trata la memoria como vector, cada celda un trozo
        
        signall(id_sem);    //permite al padre leer

        shmdt(memoria_h);   //libera puntero a memoria compartida
        return 0;



    }else{      //padre
        waitt(id_sem);  //espera que el hijo escriba los valores
        printf("Soy el padre (%d) y leo %d y %d \n", getpid(), memoria[0], memoria[1]);
    }




    //esperar hijo
    waitpid(pidHijo, NULL, 0);

    //liberar memoria compartida
    shmdt(memoria); //libera puntero a memoria
    shmctl(shm_id, IPC_RMID, NULL); //libera memoria compartida

    //elimina semaforo
    eliminar_sem(id_sem);
    return 0;
}