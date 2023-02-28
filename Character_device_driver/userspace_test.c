#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    int buf_size=20;
    int status;
    char mybuffer_batt[buf_size];
    char mybuffer_tmp[buf_size];
    char mybuffer_light[buf_size];
    pid_t child_pid;
    child_pid = fork();
    if(child_pid<0) {
        printf("error creating child process, exiting :(");
        return 1;
    }
    if(child_pid == 0 ){
        printf("You are now child\n");
        int fd_b, fd_tmp, fd_l;
        fd_b = open("/dev/lunix0-batt",O_RDONLY);
        fd_tmp = open("/dev/lunix0-temp", O_RDONLY) ;
        fd_l = open("/dev/lunix0-light", O_RDONLY);
        read(fd_b, mybuffer_batt, buf_size);
        read(fd_tmp, mybuffer_tmp, buf_size);
        read(fd_l, mybuffer_light, buf_size);
        printf("Child: Battery level is: %s.\n", mybuffer_batt);
    	printf("Child: Temperature is %s degrees celcious.\n", mybuffer_tmp);
        printf("Child: Light is at level %s.\n", mybuffer_light);
        printf("Childing exiting\n");
        return 1;
    }



    printf("You are the father\n");
    int fd_b, fd_tmp, fd_l;
    fd_b = open("/dev/lunix0-batt",O_RDONLY);
    fd_tmp = open("/dev/lunix0-temp", O_RDONLY) ;
    fd_l = open("/dev/lunix0-light", O_RDONLY);
    read(fd_b, mybuffer_batt, buf_size);
    read(fd_tmp, mybuffer_tmp, buf_size);
    read(fd_l, mybuffer_light, buf_size);

    printf("Father: Battery level is: %s.\n", mybuffer_batt);
    printf("Father: Temperature is %s degrees celcious.\n", mybuffer_tmp);
    printf("Father: Light is at level %s.\n", mybuffer_light);
    printf("Father exiting\n");
    wait(&status);
    return 1;

}
