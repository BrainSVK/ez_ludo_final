#include "k_s_definitions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
//#include <bits/types/sig_atomic_t.h>

#define MAX_CLIENS 4
#define BUFFER_SZ 2048
#define NAME_LEN 32

volatile sig_atomic_t flag = 0;
int sock = 0;
char meno[NAME_LEN];

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int lenght) {
    for (int i = 0; i < lenght; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void catch_ctrl_c_and_exit() {
    flag = 1;
}

void recv_msg_handler() {
    char message[BUFFER_SZ] = {};
    int koniec = 0;
    while (!koniec) {
        int receive = recv(sock, message, BUFFER_SZ, 0);
        if (receive > 0) {
            printf("%s", message);
            str_trim_lf(message, strlen(message));
            int len = strlen(message);
            char* prikaz = &message[len-7];
            if (strcmp(prikaz, "odpojil") == 0) {
                printf("Jeden z uzivatelov sa odpojil Hra bohuzial konci!!\n");
                koniec = 1;
                flag =1;
            }
            //printf("%s",prikaz);
            str_overwrite_stdout();
        } else if (receive == 0) {
            koniec = 1;
        }
        bzero(message, BUFFER_SZ);
    }
}

void send_msg_handler() {
    char buffer[BUFFER_SZ]= {};
    char message[BUFFER_SZ + NAME_LEN + 2] = {};

    int koniec = 0;
    while (!koniec) {
        str_overwrite_stdout();
        fgets(buffer, BUFFER_SZ,stdin);
        str_trim_lf(buffer, BUFFER_SZ);
        if (strcmp(buffer, "exit") == 0) {
            flag = 1;
            koniec = 1;
        } else {
            sprintf(message, "%s: %s\n", meno, buffer);
            send(sock,message, strlen(message), 0);
        }
        bzero(buffer, BUFFER_SZ);
        bzero(message, BUFFER_SZ + NAME_LEN);
    }
    catch_ctrl_c_and_exit();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printError("Klienta je nutne spustit s nasledujucimi argumentmi: adresa port.");
    }
    
    //ziskanie adresy a portu servera <netdb.h>
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        printError("Server neexistuje.");
    }
    int port = atoi(argv[2]);
	if (port <= 0) {
		printError("Port musi byt cele cislo vacsie ako 0.");
	}
    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Zadajte herne meno: ");
    fgets(meno, NAME_LEN, stdin);

    str_trim_lf(meno, strlen(meno));

    if (strlen(meno) > NAME_LEN - 1 || strlen(meno) < 2) {
        printf("zadaj meno spravne");
        return EXIT_FAILURE;
    }

    //vytvorenie socketu <sys/socket.h>
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("Chyba - socket.");        
    }
    
    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - connect.");        
    } else {
        send(sock,meno,NAME_LEN, 0);
        printf("Spojenie so serverom bolo nadviazane.\n");

        pthread_t send_msg_thread;
        if (pthread_create(&send_msg_thread, NULL, (void*)send_msg_handler, NULL) != 0) {
            printf("ERROR: pthread\n");
            return EXIT_FAILURE;
        }

        pthread_t recv_msg_thread;
        if (pthread_create(&recv_msg_thread, NULL, (void*)recv_msg_handler, NULL) != 0) {
            printf("ERROR: pthread\n");
            return EXIT_FAILURE;
        }
        int koniec = 0;
        while (!koniec) {
            if (flag) {
                printf("Opustil si hru\n");
                koniec = 1;
            }
        }
    }

//    char buffer[BUFFER_LENGTH + 1];
//    buffer[BUFFER_LENGTH] = '\0';
//    int koniec = 0;
//    char menoHraca[BUFFER_LENGTH + 1];
//    strcpy(menoHraca,argv[3]);
//    write(sock, menoHraca, strlen(menoHraca) + 1);
//    while (!koniec) {
//        read(sock,buffer,BUFFER_LENGTH);
//        printf("%s je na rade:\n" , buffer);
//        if (strcmp(buffer, menoHraca) == 0) {
//            fgets(buffer, BUFFER_LENGTH, stdin);
//            //fflush(stdin);
//            char* pos = strchr(buffer, '\n');
//            if (pos != NULL) {
//                *pos = '\0';
//            }
//            //zapis dat do socketu <unistd.h>
//            write(sock,buffer,strlen(buffer)+1);
//            strcpy(buffer,"");
//        }
//        if (strcmp(buffer, endMsg) != 0) {
//            //citanie dat zo socketu <unistd.h>
//            read(sock, buffer, BUFFER_LENGTH);
//            printf("%s poslal nasledujuce data:\n", buffer);
//            read(sock, buffer, BUFFER_LENGTH);
//            printf("%s\n", buffer);
//            read(sock, buffer, BUFFER_LENGTH);
//            char* nieco = buffer;
//            printf("%s\n", buffer);
//            //strcpy(buffer,"");
//        }
//        else {
//            koniec = 1;
//        }
//    }
    //uzavretie socketu <unistd.h>
    close(sock);
    printf("Spojenie so serverom bolo ukoncene.\n");
    
    return (EXIT_SUCCESS);
}