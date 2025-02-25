#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#define NUM_HIJOS 5

int sem_id;  // Semáforo global
pid_t pids_hijos[NUM_HIJOS];
pid_t pid_padre;

// Manejador de SIGINT (Ctrl + C)
void manejadora_salida(int sig){
    if(getpid()==pid_padre){
        printf("\n Ctrl + C detectado. Eliminando semáforo y terminando procesos...\n");
    }
    
    for(int i=0; i<NUM_HIJOS; i++){
        if(pids_hijos[i]>0){
            kill(pids_hijos[i], SIGKILL);  // Mata a todos los procesos del grupo, incluyendo hijos
        }
    }

   
    if(getpid()==pid_padre){
        int err3=semctl(sem_id, 0, IPC_RMID);  // Eliminar semáforo
        if(err3<0){
            printf("Error al eliminar el semaforo!!");
            exit(0);
        }
        exit(0);
    }
    
    //matar hijos primero y despues eliminar semaforo para que no se quede ninguno dentro del semáforo
    
}

void wait_sem(int sem_id){
    struct sembuf op;
    op.sem_num=0;
    op.sem_op=-1;
    op.sem_flg=0;
    semop(sem_id, &op, 1);
}

void signal_sem(int sem_id){
    struct sembuf op;
    op.sem_num=0;
    op.sem_op=5; //son 5 procesos
    op.sem_flg=0;
    semop(sem_id, &op, 1);
}



int main (void){
    int i;
    pid_t pid;
    pid_padre=getpid();
    sem_id=semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);    //creo el semáforo

    if(sem_id<0){   //compruebo si se creó bien el semáforo
        printf("Error al crear el semaforo...");
        return -1;
    }


    // Capturar señal SIGINT (Ctrl + C)
    struct sigaction sa;
    sa.sa_handler = manejadora_salida;
    sigemptyset(&sa.sa_mask);   //se redirige la accion de sigint (sin bloquear ninguna señal)
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
   

    int err=semctl(sem_id, 0, SETVAL, 0); //pone la zona critica a 0
    if(err<0){
        printf("Error al poner la zona crítica a cero");
        return -1;
    }

    for(i=0; i<NUM_HIJOS; i++){
        pid=fork();
        if(pid<0){
            printf("Error!!");
            return -1;
        }else if(pid==0){   //es el hijo
            pids_hijos[i]=getpid();
            break;
        }
    }



    if(getpid()==pid_padre){
        printf("Espero 7 segundos...\n");
        sleep(7);
        signal_sem(sem_id); //abro el semáforo (solo una vez pq incrementa en 5)

        for (i = 0; i < NUM_HIJOS; i++) {
            waitpid(pids_hijos[i], NULL, 0);  // espero a que terminen todos los hijos
        }

        int err2=semctl(sem_id, 0, IPC_RMID); //elimino el semáforo
        if(err2<0){
            printf("Error al eliminar el semáforo");
            return 0;
        }


    }else{
        wait_sem(sem_id);   //decrementa en 1 porque entran los 5
        printf("Hijo: %d\n", getpid());
        signal_sem(sem_id); 
        return 0;
    }


    return 0;
}
