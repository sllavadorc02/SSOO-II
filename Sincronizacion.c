#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>




void signall(int id_sem, int num){
    struct sembuf m;
    m.sem_num=num;
    m.sem_op=1;
    m.sem_flg=0;
    int err1=semop(id_sem, &m, 1);
    if(err1==-1){
        printf("Error al hacer signal...\n");
        return;
    }
}

void waitt(int id_sem, int num){
    struct sembuf m;
    m.sem_num=num;
    m.sem_op=-1;
    m.sem_flg=0;
    int err1=semop(id_sem, &m, 1);
    if(err1==-1){
        printf("Error al hacer wait...\n");
        return;
    }
}


pid_t pidHijos[2];

int main (void){
    int id_sem=semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    pid_t pidPadre=getpid();
    int i;
    if(id_sem==-1){
        printf("Error al solicitar semáforos...\n");
        return -1;
    }

    int err=semctl(id_sem, 0, SETVAL, 0);   //semáforo del 0 a 0
    if(err==-1){
        printf("Error al establecer limite en semaforo 0...\n");
        return -1;
    }

    err=semctl(id_sem, 1, SETVAL, 1);   //semáforo del 1 a 1
    if(err==-1){
        printf("Error al establecer limite en semaforo 1...\n");
        return -1;
    }


    for(i=0; i<2; i++){
        pid_t pid=fork();
        if(pid<0){
            printf("Error al crear hijos...\n");
            return -1;
        }else if(pid==0){   //hijo
            pidHijos[i]=getpid();
            break;

        }
    }



    if(getpid()==pidHijos[0]){   //imprime 0
        for(i=0; i<=100; i++){
            waitt(id_sem, 0);
            printf("0");
            fflush(stdout);
            signall(id_sem, 1);
        }

        return 0;

    }else if(getpid()==pidHijos[1]){    //imprime 1
        for(i=0; i<=100; i++){
            waitt(id_sem, 1);
            printf("1");
            fflush(stdout);
            signall(id_sem, 0);

        }
        return 0;
    }




    //esperar a los hijos;
    for(i=0; i<2; i++){
        waitpid(pidHijos[i], NULL, 0);
    }

    err=semctl(id_sem, 0, IPC_RMID);    //elimina semaforo
    if(err==-1){
        printf("Error al eliminar semáforo...\n");
        return -1;
    }

    return 0;
}