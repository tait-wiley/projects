#include <stdlib.h> //system()
#include <unistd.h> // fork() sleep()
#include <stdio.h> // popen()

int main(int argc, char **argv){
    FILE *fp1, *fp2, *fp3;
    // compiling microservers
    system("g++ UDPmicrotranslate.cpp -o translate");
    system("g++ UDPmicrocurrency.cpp -o currency");
    system("g++ UDPmicrovote.cpp -o vote");

    // running all the microservers
    if(!fork()){
        fp1 = popen("./translate", "r");
    }

    else if(!fork()){
        fp2 = popen("./currency", "r");
    }

    else if(!fork()){
        fp3 = popen("./vote", "r");
    }

    // run the servers for 10 minutes
    else{
        sleep(600);
        printf("closing\n");

        pclose(fp1);
        pclose(fp2);
        pclose(fp3);

    }

    return 0;



}
