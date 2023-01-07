#include "k_s_definitions.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "clovece.h"
#include <signal.h>

#define BUFFER_SZ 2048
#define NAME_LEN  32
#define MAX_CLEINTS 4

static _Atomic unsigned int cli_count = 0;
static int uid = 0;
int jeTuEsteNiekto = 0;
int serverSocket;
int hod = 0;
int sest = 0;

typedef struct {
    struct sockaddr_in address;
    int clientSock;
    int uid;
    char meno[NAME_LEN];
} client_t;

client_t  *clients[MAX_CLEINTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void queue_add(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_massage(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                if (write(clients[i]->clientSock, s, strlen(s)) < 0) {
                    printf("ERROR: write sa pokazil\n");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_massage_toMe(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                if (write(clients[i]->clientSock, s, strlen(s)) < 0) {
                    printf("ERROR: write sa pokazil\n");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_massage_toAll(char *s) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (write(clients[i]->clientSock, s, strlen(s)) < 0) {
                printf("ERROR: write sa pokazil\n");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

char* spracujData(char *data) {
    char *akt = data;
    while (*akt != '\0') {
        if (islower(*akt)) {
            *akt = toupper(*akt);
        }
        else if (isupper(*akt)) {
            *akt = tolower(*akt);
        }
		akt++;		
    }
    return data;
}

void *handle_client(void *arg) {
    jeTuEsteNiekto++;
    char buffer[BUFFER_SZ];
    char meno[NAME_LEN];
    int leave_flag = 0;
    cli_count++;
    client_t  *client = (client_t*)arg;

    if (recv(client->clientSock, meno, NAME_LEN, 0) <= 0 || strlen(meno) >= NAME_LEN - 1) {
        printf("Enter meno spravne\n");
        leave_flag = 1;
    } else {
        strcpy(client->meno,meno);
        sprintf(buffer,"hrac: %s sa pripojil\n", client->meno);
        printf("%s", buffer);
        send_massage(buffer, client->uid);
    }
    bzero(buffer , BUFFER_SZ);
    int koniec = 0;
    while (!koniec) {

        int receive = recv(client->clientSock, buffer, BUFFER_SZ, 0);

        if (receive > 0) {
            if (strlen(buffer) > 0) {
                send_massage(buffer, client->uid);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s\n", buffer);
                int lenght = strlen(buffer);
                int pomlcka = lenght - 6;
                if (buffer[pomlcka] == '-') {
                    if (client->uid == getPoradie()) {
                        printf("je na rade: %s\n",client->meno);
                        int len = strlen(buffer);
                        char* prikaz = &buffer[len-6];

                        if ((strcmp(prikaz, "-dhodk") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (hod == 0) {
                                hod = hodKockou();
                            } else if (hod % 6 == 0) {
                                hod += hodKockou();
                                sest++;
                            } else {
                                printf("%s: uz si si hodil\n", client->meno);
                                char *error = "";
                                sprintf(error, "Uz si si hodil a padlo ti:%d s %d sestkami\n", hod, sest);
                                send_massage_toMe(error, client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dnast") == 0) && jeTuEsteNiekto == MAX_CLEINTS && sest > 0) {
                            int skontroluj = nastavPanacika(client->uid);
                            if (skontroluj == 1) {
                                sprintf(buffer,"Hrac: %s si postavl noveho panacika\n",clients[getPoradie()]->meno);
                                printf("Hrac: %s si postavl noveho panacika\n",clients[getPoradie()]->meno);
                                send_massage_toAll(buffer);
                                sest--;
                                hod -= 6;
                            } else {
                                printf("Hrac: %s si nemohol postavit noveho panacika\n",clients[getPoradie()]->meno);
                                sprintf(buffer,"Nemozes si postavit noveho panacika\n");
                                send_massage_toMe(buffer,client->uid);
                            }
                            if (hod == 0) {
                                if (getPoradie() == 3) {
                                    nastavPoradie(0);
                                } else {
                                    nastavPoradie(getPoradie()+1);
                                }
                            }
                        }

                        if ((strcmp(prikaz, "-dposn") == 0) && jeTuEsteNiekto == MAX_CLEINTS && hod > 0) {

                            hod = 0;
                        }

                        if ((strcmp(prikaz, "-dvykr") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            vykresli(prikaz);
                            printf("**%s",prikaz);
                            send_massage_toAll(prikaz);
                            if (getPoradie() == 3) {
                                nastavPoradie(0);
                            } else {
                                nastavPoradie(getPoradie()+1);
                            }
                            sprintf(buffer,"na rade je: %s\n",clients[getPoradie()]->meno);
                            printf("na rade je: %s\n",clients[getPoradie()]->meno);
                            send_massage_toAll(buffer);
                        }
                        if ((strcmp(prikaz, "-duidh") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            printf("%d\n",getIdHraca(client->uid));
                        }
                        if ((strcmp(prikaz, "-dhrac") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            vypisStavHraca(client->uid,prikaz);
                            send_massage_toMe(prikaz,client->uid);
                        }

                    } else {
                        printf("nie je na rade: %s\n",client->meno);
                        char* error = "nie si na rade\n";
                        send_massage_toMe(error,client->uid);
                        //send_massage_toAll(error);
                    }
                }
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            sprintf(buffer, "%s sa odpojil\n", client->meno);
            printf("%s", buffer);
            send_massage(buffer, client->uid);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        if (leave_flag) {
            koniec = 1;
        }
        bzero(buffer , BUFFER_SZ);
    }
    bzero(buffer , BUFFER_SZ);
    close(client->clientSock);
    queue_remove(client->uid);
    queue_remove_hrac(client->uid);
    free(client);
    cli_count--;
    pthread_detach(pthread_self());
    jeTuEsteNiekto--;
    return NULL;
}

int main(int argc, char** argv) {
    srand(time(0));
    if (argc < 2) {
        printError("Sever je nutne spustit s nasledujucimi argumentmi: port.");
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }
    pthread_t tid;
    //vytvorenie TCP socketu <sys/socket.h>
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Chyba - socket.");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu

    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - bind.");
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    listen(serverSocket, 10);
    printf("Server bol spusteny.\n");

    //server caka na pripojenie klienta <sys/socket.h>
    int koniec = 0;
    nastavNaX();
    int clientSocket = 0;
    int pocetKlientov = 1;
    char message[BUFFER_SZ] = {};
    while (pocetKlientov  <= MAX_CLEINTS ) {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);

        client_t *client = (client_t *) malloc(sizeof(client_t));
        client->address = clientAddress;
        client->clientSock = clientSocket;
        client->uid = uid++;

        queue_add(client);
        pthread_create(&tid, NULL, &handle_client, (void *)client);
        sprintf(message, "Pocet pripojených: %d zostava: %d\n", pocetKlientov, MAX_CLEINTS-pocetKlientov);
        printf("Pocet pripojených: %d zostava: %d \n", pocetKlientov,MAX_CLEINTS-pocetKlientov);
        send_massage_toAll(message);
        pocetKlientov++;
        sleep(1);
    }
    printf("maximalny pocet klientov sa pripojil\n");
    close(serverSocket);
    vyhresliHraciuPlochu();
    vykresliDomPlochu();
    vykresliZacDomPlochu();
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        pridajHraca(clients[i]->uid,clients[i]->meno);
    }
    printf("Hráči boli pridaný\n");
    if (clientSocket < 0) {
        printError("Chyba - accept.");
    }
    while (jeTuEsteNiekto > 0) {

    }
    printf("Vsetci sa odpojili hra konci!!!\n");
    return (EXIT_SUCCESS);

}
//    printf("Klient sa pripojil na server.\n");
//    char buffer[BUFFER_LENGTH + 1];
//    buffer[BUFFER_LENGTH] = '\0';
//    int koniec = 0;
//    read(clientSocket[0], buffer, BUFFER_LENGTH);
//    char menoHraca[BUFFER_LENGTH + 1];
//    strcpy(menoHraca,buffer);
//    read(clientSocket[1], buffer, BUFFER_LENGTH);
//    char menoHraca1[BUFFER_LENGTH + 1];
//    strcpy(menoHraca1,buffer);
//    char * menoHracov[2];
//    int i = 0;
//    menoHracov[0] = menoHraca;
//    menoHracov[1] = menoHraca1;
//    while (!koniec) {
//        stdin->_lock;
//        //citanie dat zo socketu <unistd.h>
//        printf("na rade je %s\n", menoHracov[i]);
//        write(clientSocket[0], menoHracov[i], BUFFER_LENGTH);
//        write(clientSocket[1], menoHracov[i], BUFFER_LENGTH);
//        read(clientSocket[i], buffer, BUFFER_LENGTH);
//        if (strcmp(buffer, endMsg) != 0) {
//            printf("%s poslal nasledujuce data:\n%s\n", menoHracov[i],buffer);
//            //spracujData(buffer);
//			//zapis dat do socketu <unistd.h>
//            for (int j = 0; j < 2; ++j) {
//                write(clientSocket[j], menoHracov[i], BUFFER_LENGTH);
//                //write(clientSocket[j], buffer, strlen(menoHracov[i]) + 1);
//            }
//
//            for (int j = 0; j < 2; ++j) {
//                //write(clientSocket[j], menoHracov[i], strlen(buffer) + 1);
//                write(clientSocket[j], buffer, BUFFER_LENGTH);
//            }
//
//            for (int j = 0; j < 2; ++j) {
//                //write(clientSocket[j], menoHracov[i], strlen(buffer) + 1);
//                write(clientSocket[j], vykresli(),  BUFFER_LENGTH);
//            }
//
//        }
//        else {
//            koniec = 1;
//        }
//
//        if (i == 1) {
//            i = 0;
//        } else {
//            i = 1;
//        }
//    }
//    printf("Klient ukoncil komunikaciu.\n");
//
//    //uzavretie pasivneho socketu <unistd.h>
//    close(serverSocket);
//    for (int i = 0; i < 2; ++i) {
//        if (clientSocket[i] < 0) {
//            printError("Chyba - accept.");
//        }
//        //uzavretie socketu klienta <unistd.h>
//        close(clientSocket[i]);
//    }
//    return (EXIT_SUCCESS);
//}