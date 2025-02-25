#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 	//función pipe

int main(void){
    int p[2];
    int s=11, h;
    int err=pipe(p);
    if(err==-1){
        printf("error al crear la tubería");
        return -1;
    }

    pid_t hijo= fork();

    if(hijo==0){	//hijo lee
        close(p[1]);	//cerrar escritura
        read(p[0], &h, sizeof(int));
        printf("El valor recibido es %d", h);

        close(p[0]);

    }else if(hijo==-1){
        printf("error al hacer fork");
        return -1;

    }else{
        close(p[0]);	//cerrar lectura
        sleep(7);
        write(p[1], &s, sizeof(int));
        close(p[1]);

    }

    return 0;
}
