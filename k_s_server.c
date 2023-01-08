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
//#include <signal.h>

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
    int vyhral;
} client_t;

client_t  *clients[MAX_CLEINTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void queue_remove(int _uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == _uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_massage(char *s, int _uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid != _uid) {
                if (write(clients[i]->clientSock, s, strlen(s)) < 0) {
                    printf("ERROR: write sa pokazil\n");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_massage_toMe(char *s, int _uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLEINTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == _uid) {
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
                int len = strlen(buffer);
                char* prikaz = &buffer[len-6];
                if ((strcmp(prikaz, "vykres") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                    vykresli(prikaz);
                    send_massage_toMe(prikaz,client->uid);
                }
                if (buffer[pomlcka] == '-') {
                    if (client->uid == getPoradie()) {
                        printf("je na rade: %s\n",client->meno);
                        len = strlen(buffer);
                        prikaz = &buffer[len-6];

                        if ((strcmp(prikaz, "-dhodk") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (hod == 0) {
                                hod = hodKockou();
                                sprintf(buffer,"%s: padlo cislo: %d\n",client->meno, hod);
                                printf("%s: padlo cislo: %d\n",client->meno, hod);
                                send_massage_toAll(buffer);
                                if (hod == 6) {
                                    sest++;
                                }
                                if (masKymPohnut(client->uid) == 0 && hod % 6 != 0) {
                                    printf("%s: nema sa skym pohnut\n", client->meno);
                                    char *error = "Nemas sa skym pohnut padlo\n";
                                    send_massage_toMe(error, client->uid);

                                    if (getPoradie() == 3 && clients[0]->vyhral == 0) {
                                        nastavPoradie(0);
                                    } else if (getPoradie() == 3 && clients[1]->vyhral == 0) {
                                        nastavPoradie(1);
                                    } else if (getPoradie() == 3 && clients[2]->vyhral == 0) {
                                        nastavPoradie(2);
                                    } else if (getPoradie() == 3 && clients[getPoradie()]->vyhral == 0) {
                                        nastavPoradie(getPoradie());
                                    } else {
                                        if (getPoradie() == 3) {
                                            printf("Hraci ukoncili hru!!!\n");
                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                            send_massage_toAll(buffer);
                                            nastavPoradie(-1);
                                        } else {
                                            if (getPoradie() == 0 && clients[getPoradie() +1]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 1);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 2]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 2);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 3]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 3);
                                            } else if (getPoradie() == 0 && clients[getPoradie()]->vyhral == 0) {
                                                nastavPoradie(getPoradie());
                                            } else {
                                                if (getPoradie() == 0) {
                                                    printf("Hraci ukoncili hru!!!\n");
                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                    send_massage_toAll(buffer);
                                                    nastavPoradie(-1);
                                                } else {
                                                    if (getPoradie() == 1 && clients[getPoradie() + 1]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 1);
                                                    } else if (getPoradie() == 1 && clients[getPoradie() + 2]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 2);
                                                    } else if (getPoradie() == 1 && clients[0]->vyhral == 0) {
                                                        nastavPoradie(0);
                                                    } else if (getPoradie() == 1 && clients[getPoradie()]->vyhral == 0) {
                                                        nastavPoradie(getPoradie());
                                                    } else {
                                                        if (getPoradie() == 1) {
                                                            printf("Hraci ukoncili hru!!!\n");
                                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                            send_massage_toAll(buffer);
                                                            nastavPoradie(-1);
                                                        } else {
                                                            if (getPoradie() == 2 && clients[getPoradie() + 1]->vyhral == 0) {
                                                                nastavPoradie(getPoradie() + 1);
                                                            } else if (getPoradie() == 2 && clients[0]->vyhral == 0) {
                                                                nastavPoradie(0);
                                                            } else if (getPoradie() == 2 && clients[1]->vyhral == 0) {
                                                                nastavPoradie(1);
                                                            } else if (getPoradie() == 2 && clients[getPoradie()]->vyhral == 0) {
                                                                nastavPoradie(getPoradie());
                                                            } else {
                                                                if (getPoradie() == 2) {
                                                                    printf("Hraci ukoncili hru!!!\n");
                                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                                    send_massage_toAll(buffer);
                                                                    nastavPoradie(-1);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (getPoradie() != -1) {
                                        sprintf(buffer, "na rade je: %s\n", clients[getPoradie()]->meno);
                                        printf("na rade je: %s\n", clients[getPoradie()]->meno);
                                        send_massage_toAll(buffer);
                                    }
                                    hod = 0;
                                    sest = 0;
                                }
                            } else if (hod % 6 == 0) {
                                hod += hodKockou();
                                if (hod % 6 == 0) {
                                    sest++;
                                }
                                sprintf(buffer,"%s: hadzal si znovu a mas: %d z toho %d sestky\n",client->meno, hod,sest);
                                printf("%s: padlo cislo: %d\n",client->meno, hod);
                                send_massage_toAll(buffer);
                            } else {
                                printf("%s: uz si si hodil\n", client->meno);
                                char* error = "Uz si hodzal!!!\n";
                                send_massage_toMe(error,client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dnast") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (sest > 0) {
                                int skontroluj = nastavPanacika(client->uid);
                                if (skontroluj == 1 && sest > 0) {
                                    vykresli(prikaz);
                                    printf("**%s", prikaz);
                                    send_massage_toAll(prikaz);

                                    sprintf(buffer, "Hrac: %s si postavl noveho panacika\n",client->meno);
                                    printf("Hrac: %s si postavl noveho panacika\n", client->meno);
                                    send_massage_toAll(buffer);
                                    sest = sest - 1;
                                    printf("%s: spotreboval si 6 mas ich %d", client->meno, sest);
                                    hod -= 6;
                                    printf("Hrac: %s zostalo %d na pohnutie\n", client->meno, hod);
                                    sprintf(buffer, "Hrac: %s zostava na pohnutie %d\n", client->meno,hod);
                                    send_massage_toMe(buffer, client->uid);
                                } else {
                                    printf("Hrac: %s si nemohol postavit noveho panacika\n",client->meno);
                                    sprintf(buffer,"Nemozes si postavit noveho panacika\n");
                                    send_massage_toMe(buffer,client->uid);
                                }
                            } else {
                                printf("Hrac: %s si nemohol postavit noveho panacika\n",client->meno);
                                sprintf(buffer,"Nemozes si postavit noveho panacika\n");
                                send_massage_toMe(buffer,client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dpos1") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (hod > 0) {
                                int skontroluj = skusPosunutPanacika(client->uid, 0, hod);
                                if (skontroluj == 1 && hod > 0) {
                                    vykresli(prikaz);
                                    printf("**%s", prikaz);
                                    send_massage_toAll(prikaz);

                                    sprintf(buffer, "Hrac: %s sa posunul panacikom cislo 1\n",client->meno);
                                    printf("Hrac: %s sa posunul panacikom cislo 1\n", client->meno);
                                    send_massage_toAll(buffer);
                                    hod = 0;
                                    sest = 0;
                                    if (skontrolujCiVyhral(client->uid) == 1) {
                                        printf("Hrac: %s vyhral\n", client->meno);
                                        sprintf(buffer, "Hrac: %s vyhral\n", client->meno);
                                        send_massage_toAll(buffer);
                                        clients[getPoradie()]->vyhral = 1;
                                        client->vyhral = 1;
                                    }
                                    if (getPoradie() == 3 && clients[0]->vyhral == 0) {
                                        nastavPoradie(0);
                                    } else if (getPoradie() == 3 && clients[1]->vyhral == 0) {
                                        nastavPoradie(1);
                                    } else if (getPoradie() == 3 && clients[2]->vyhral == 0) {
                                        nastavPoradie(2);
                                    } else if (getPoradie() == 3 && clients[getPoradie()]->vyhral == 0) {
                                        nastavPoradie(getPoradie());
                                    } else {
                                        if (getPoradie() == 3) {
                                            printf("Hraci ukoncili hru!!!\n");
                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                            send_massage_toAll(buffer);
                                            nastavPoradie(-1);
                                        } else {
                                            if (getPoradie() == 0 && clients[getPoradie() +1]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 1);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 2]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 2);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 3]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 3);
                                            } else if (getPoradie() == 0 && clients[getPoradie()]->vyhral == 0) {
                                                nastavPoradie(getPoradie());
                                            } else {
                                                if (getPoradie() == 0) {
                                                    printf("Hraci ukoncili hru!!!\n");
                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                    send_massage_toAll(buffer);
                                                    nastavPoradie(-1);
                                                } else {
                                                    if (getPoradie() == 1 && clients[getPoradie() + 1]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 1);
                                                    } else if (getPoradie() == 1 && clients[getPoradie() + 2]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 2);
                                                    } else if (getPoradie() == 1 && clients[0]->vyhral == 0) {
                                                        nastavPoradie(0);
                                                    } else if (getPoradie() == 1 && clients[getPoradie()]->vyhral == 0) {
                                                        nastavPoradie(getPoradie());
                                                    } else {
                                                        if (getPoradie() == 1) {
                                                            printf("Hraci ukoncili hru!!!\n");
                                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                            send_massage_toAll(buffer);
                                                            nastavPoradie(-1);
                                                        } else {
                                                            if (getPoradie() == 2 && clients[getPoradie() + 1]->vyhral == 0) {
                                                                nastavPoradie(getPoradie() + 1);
                                                            } else if (getPoradie() == 2 && clients[0]->vyhral == 0) {
                                                                nastavPoradie(0);
                                                            } else if (getPoradie() == 2 && clients[1]->vyhral == 0) {
                                                                nastavPoradie(1);
                                                            } else if (getPoradie() == 2 && clients[getPoradie()]->vyhral == 0) {
                                                                nastavPoradie(getPoradie());
                                                            } else {
                                                                if (getPoradie() == 2) {
                                                                    printf("Hraci ukoncili hru!!!\n");
                                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                                    send_massage_toAll(buffer);
                                                                    nastavPoradie(-1);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (getPoradie() != -1) {
                                        sprintf(buffer, "na rade je: %s\n", clients[getPoradie()]->meno);
                                        printf("na rade je: %s\n", clients[getPoradie()]->meno);
                                        send_massage_toAll(buffer);
                                    }
                                } else {
                                    printf("Hrac: %s sa nemohol posunut panacikom cislo 1\n", client->meno);
                                    sprintf(buffer, "Nemozes sa posunut panacikom cislo 1\n");
                                    send_massage_toMe(buffer, client->uid);
                                }
                            } else {
                                printf("Hrac: %s sa nemohol posunut panacikom cislo 1\n",client->meno);
                                sprintf(buffer,"Nemozes sa posunut panacikom cislo 1\n");
                                send_massage_toMe(buffer,client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dpos2") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (hod > 0) {
                                int skontroluj = skusPosunutPanacika(client->uid,1,hod);
                                if (skontroluj == 1 && hod > 0) {
                                    vykresli(prikaz);
                                    printf("**%s", prikaz);
                                    send_massage_toAll(prikaz);

                                    sprintf(buffer, "Hrac: %s sa posunul panacikom cislo 2\n", client->meno);
                                    printf("Hrac: %s sa posunul panacikom cislo 2\n", client->meno);
                                    send_massage_toAll(buffer);
                                    hod = 0;
                                    sest = 0;
                                    if (skontrolujCiVyhral(client->uid) == 1) {
                                        printf("Hrac: %s vyhral\n", client->meno);
                                        sprintf(buffer, "Hrac: %s vyhral\n", client->meno);
                                        send_massage_toAll(buffer);
                                        clients[getPoradie()]->vyhral = 1;
                                        client->vyhral = 1;
                                    }
                                    if (getPoradie() == 3 && clients[0]->vyhral == 0) {
                                        nastavPoradie(0);
                                    } else if (getPoradie() == 3 && clients[1]->vyhral == 0) {
                                        nastavPoradie(1);
                                    } else if (getPoradie() == 3 && clients[2]->vyhral == 0) {
                                        nastavPoradie(2);
                                    } else if (getPoradie() == 3 && clients[getPoradie()]->vyhral == 0) {
                                        nastavPoradie(getPoradie());
                                    } else {
                                        if (getPoradie() == 3) {
                                            printf("Hraci ukoncili hru!!!\n");
                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                            send_massage_toAll(buffer);
                                            nastavPoradie(-1);
                                        } else {
                                            if (getPoradie() == 0 && clients[getPoradie() +1]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 1);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 2]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 2);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 3]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 3);
                                            } else if (getPoradie() == 0 && clients[getPoradie()]->vyhral == 0) {
                                                nastavPoradie(getPoradie());
                                            } else {
                                                if (getPoradie() == 0) {
                                                    printf("Hraci ukoncili hru!!!\n");
                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                    send_massage_toAll(buffer);
                                                    nastavPoradie(-1);
                                                } else {
                                                    if (getPoradie() == 1 && clients[getPoradie() + 1]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 1);
                                                    } else if (getPoradie() == 1 && clients[getPoradie() + 2]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 2);
                                                    } else if (getPoradie() == 1 && clients[0]->vyhral == 0) {
                                                        nastavPoradie(0);
                                                    } else if (getPoradie() == 1 && clients[getPoradie()]->vyhral == 0) {
                                                        nastavPoradie(getPoradie());
                                                    } else {
                                                        if (getPoradie() == 1) {
                                                            printf("Hraci ukoncili hru!!!\n");
                                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                            send_massage_toAll(buffer);
                                                            nastavPoradie(-1);
                                                        } else {
                                                            if (getPoradie() == 2 && clients[getPoradie() + 1]->vyhral == 0) {
                                                                nastavPoradie(getPoradie() + 1);
                                                            } else if (getPoradie() == 2 && clients[0]->vyhral == 0) {
                                                                nastavPoradie(0);
                                                            } else if (getPoradie() == 2 && clients[1]->vyhral == 0) {
                                                                nastavPoradie(1);
                                                            } else if (getPoradie() == 2 && clients[getPoradie()]->vyhral == 0) {
                                                                nastavPoradie(getPoradie());
                                                            } else {
                                                                if (getPoradie() == 2) {
                                                                    printf("Hraci ukoncili hru!!!\n");
                                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                                    send_massage_toAll(buffer);
                                                                    nastavPoradie(-1);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (getPoradie() != -1) {
                                        sprintf(buffer, "na rade je: %s\n", clients[getPoradie()]->meno);
                                        printf("na rade je: %s\n", clients[getPoradie()]->meno);
                                        send_massage_toAll(buffer);
                                    }
                                } else {
                                    printf("Hrac: %s sa nemohol posunut panacikom cislo 2\n", client->meno);
                                    sprintf(buffer, "Nemozes sa posunut panacikom cislo 2\n");
                                    send_massage_toMe(buffer, client->uid);
                                }
                            } else {
                                printf("Hrac: %s sa nemohol posunut panacikom cislo 2\n",client->meno);
                                sprintf(buffer,"Nemozes sa posunut panacikom cislo 2\n");
                                send_massage_toMe(buffer,client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dpos3") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            if (hod > 0) {
                                int skontroluj = skusPosunutPanacika(client->uid,2,hod);
                                if (skontroluj == 1 && hod > 0) {
                                    vykresli(prikaz);
                                    printf("**%s", prikaz);
                                    send_massage_toAll(prikaz);

                                    sprintf(buffer, "Hrac: %s sa posunul panacikom cislo 2\n", client->meno);
                                    printf("Hrac: %s sa posunul panacikom cislo 2\n", client->meno);
                                    send_massage_toAll(buffer);
                                    hod = 0;
                                    sest = 0;
                                    if (skontrolujCiVyhral(client->uid) == 1) {
                                        printf("Hrac: %s vyhral\n", client->meno);
                                        sprintf(buffer, "Hrac: %s vyhral\n", client->meno);
                                        send_massage_toAll(buffer);
                                        clients[getPoradie()]->vyhral = 1;
                                        client->vyhral = 1;
                                    }
                                    if (getPoradie() == 3 && clients[0]->vyhral == 0) {
                                        nastavPoradie(0);
                                    } else if (getPoradie() == 3 && clients[1]->vyhral == 0) {
                                        nastavPoradie(1);
                                    } else if (getPoradie() == 3 && clients[2]->vyhral == 0) {
                                        nastavPoradie(2);
                                    } else if (getPoradie() == 3 && clients[getPoradie()]->vyhral == 0) {
                                        nastavPoradie(getPoradie());
                                    } else {
                                        if (getPoradie() == 3) {
                                            printf("Hraci ukoncili hru!!!\n");
                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                            send_massage_toAll(buffer);
                                            nastavPoradie(-1);
                                        } else {
                                            if (getPoradie() == 0 && clients[getPoradie() +1]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 1);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 2]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 2);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 3]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 3);
                                            } else if (getPoradie() == 0 && clients[getPoradie()]->vyhral == 0) {
                                                nastavPoradie(getPoradie());
                                            } else {
                                                if (getPoradie() == 0) {
                                                    printf("Hraci ukoncili hru!!!\n");
                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                    send_massage_toAll(buffer);
                                                    nastavPoradie(-1);
                                                } else {
                                                    if (getPoradie() == 1 && clients[getPoradie() + 1]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 1);
                                                    } else if (getPoradie() == 1 && clients[getPoradie() + 2]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 2);
                                                    } else if (getPoradie() == 1 && clients[0]->vyhral == 0) {
                                                        nastavPoradie(0);
                                                    } else if (getPoradie() == 1 && clients[getPoradie()]->vyhral == 0) {
                                                        nastavPoradie(getPoradie());
                                                    } else {
                                                        if (getPoradie() == 1) {
                                                            printf("Hraci ukoncili hru!!!\n");
                                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                            send_massage_toAll(buffer);
                                                            nastavPoradie(-1);
                                                        } else {
                                                            if (getPoradie() == 2 && clients[getPoradie() + 1]->vyhral == 0) {
                                                                nastavPoradie(getPoradie() + 1);
                                                            } else if (getPoradie() == 2 && clients[0]->vyhral == 0) {
                                                                nastavPoradie(0);
                                                            } else if (getPoradie() == 2 && clients[1]->vyhral == 0) {
                                                                nastavPoradie(1);
                                                            } else if (getPoradie() == 2 && clients[getPoradie()]->vyhral == 0) {
                                                                nastavPoradie(getPoradie());
                                                            } else {
                                                                if (getPoradie() == 2) {
                                                                    printf("Hraci ukoncili hru!!!\n");
                                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                                    send_massage_toAll(buffer);
                                                                    nastavPoradie(-1);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (getPoradie() != -1) {
                                        sprintf(buffer, "na rade je: %s\n", clients[getPoradie()]->meno);
                                        printf("na rade je: %s\n", clients[getPoradie()]->meno);
                                        send_massage_toAll(buffer);
                                    }
                                } else {
                                    printf("Hrac: %s sa nemohol posunut panacikom cislo 3\n", client->meno);
                                    sprintf(buffer, "Nemozes sa posunut panacikom cislo 3\n");
                                    send_massage_toMe(buffer, client->uid);
                                }
                            } else {
                                printf("Hrac: %s sa nemohol posunut panacikom cislo 3\n",client->meno);
                                sprintf(buffer,"Nemozes sa posunut panacikom cislo 3\n");
                                send_massage_toMe(buffer,client->uid);
                            }
                        }

                        if ((strcmp(prikaz, "-dpos4") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            if (hod > 0) {
                                int skontroluj = skusPosunutPanacika(client->uid, 3, hod);
                                if (skontroluj == 1) {
                                    vykresli(prikaz);
                                    printf("**%s", prikaz);
                                    send_massage_toAll(prikaz);

                                    sprintf(buffer, "Hrac: %s sa posunul panacikom cislo 4\n", client->meno);
                                    printf("Hrac: %s sa posunul panacikom cislo 4\n", client->meno);
                                    send_massage_toAll(buffer);
                                    hod = 0;
                                    sest = 0;
                                    if (skontrolujCiVyhral(client->uid) == 1) {
                                        printf("Hrac: %s vyhral\n", client->meno);
                                        sprintf(buffer, "Hrac: %s vyhral\n", client->meno);
                                        send_massage_toAll(buffer);
                                        clients[getPoradie()]->vyhral = 1;
                                        client->vyhral = 1;
                                    }
                                    if (getPoradie() == 3 && clients[0]->vyhral == 0) {
                                        nastavPoradie(0);
                                    } else if (getPoradie() == 3 && clients[1]->vyhral == 0) {
                                        nastavPoradie(1);
                                    } else if (getPoradie() == 3 && clients[2]->vyhral == 0) {
                                        nastavPoradie(2);
                                    } else if (getPoradie() == 3 && clients[getPoradie()]->vyhral == 0) {
                                        nastavPoradie(getPoradie());
                                    } else {
                                        if (getPoradie() == 3) {
                                            printf("Hraci ukoncili hru!!!\n");
                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                            send_massage_toAll(buffer);
                                            nastavPoradie(-1);
                                        } else {
                                            if (getPoradie() == 0 && clients[getPoradie() +1]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 1);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 2]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 2);
                                            } else if (getPoradie() == 0 && clients[getPoradie() + 3]->vyhral == 0) {
                                                nastavPoradie(getPoradie() + 3);
                                            } else if (getPoradie() == 0 && clients[getPoradie()]->vyhral == 0) {
                                                nastavPoradie(getPoradie());
                                            } else {
                                                if (getPoradie() == 0) {
                                                    printf("Hraci ukoncili hru!!!\n");
                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                    send_massage_toAll(buffer);
                                                    nastavPoradie(-1);
                                                } else {
                                                    if (getPoradie() == 1 && clients[getPoradie() + 1]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 1);
                                                    } else if (getPoradie() == 1 && clients[getPoradie() + 2]->vyhral == 0) {
                                                        nastavPoradie(getPoradie() + 2);
                                                    } else if (getPoradie() == 1 && clients[0]->vyhral == 0) {
                                                        nastavPoradie(0);
                                                    } else if (getPoradie() == 1 && clients[getPoradie()]->vyhral == 0) {
                                                        nastavPoradie(getPoradie());
                                                    } else {
                                                        if (getPoradie() == 1) {
                                                            printf("Hraci ukoncili hru!!!\n");
                                                            sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                            send_massage_toAll(buffer);
                                                            nastavPoradie(-1);
                                                        } else {
                                                            if (getPoradie() == 2 && clients[getPoradie() + 1]->vyhral == 0) {
                                                                nastavPoradie(getPoradie() + 1);
                                                            } else if (getPoradie() == 2 && clients[0]->vyhral == 0) {
                                                                nastavPoradie(0);
                                                            } else if (getPoradie() == 2 && clients[1]->vyhral == 0) {
                                                                nastavPoradie(1);
                                                            } else if (getPoradie() == 2 && clients[getPoradie()]->vyhral == 0) {
                                                                nastavPoradie(getPoradie());
                                                            } else {
                                                                if (getPoradie() == 2) {
                                                                    printf("Hraci ukoncili hru!!!\n");
                                                                    sprintf(buffer, "Vsetci ste dostali svojich do domceku gratulujem!!! pomocou prikazu exit ukoncite program\n");
                                                                    send_massage_toAll(buffer);
                                                                    nastavPoradie(-1);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (getPoradie() != -1) {
                                        sprintf(buffer, "na rade je: %s\n", clients[getPoradie()]->meno);
                                        printf("na rade je: %s\n", clients[getPoradie()]->meno);
                                        send_massage_toAll(buffer);
                                    }
                                } else {
                                    printf("Hrac: %s sa nemohol posunut panacikom cislo 4\n", client->meno);
                                    sprintf(buffer, "Nemozes sa posunut panacikom cislo 4\n");
                                    send_massage_toMe(buffer, client->uid);
                                }
                            } else {
                                printf("Hrac: %s sa nemohol posunut panacikom cislo 4\n", client->meno);
                                sprintf(buffer, "Nemozes sa posunut panacikom cislo 4\n");
                                send_massage_toMe(buffer, client->uid);
                            }
                        }
                        //testovacie prikazy
                        if ((strcmp(prikaz, "-dhod6") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 6;
                            sest += 1;
                        }

                        if ((strcmp(prikaz, "-dhod5") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 5;
                        }

                        if ((strcmp(prikaz, "-dhod4") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 4;
                        }

                        if ((strcmp(prikaz, "-dhod3") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 3;
                        }

                        if ((strcmp(prikaz, "-dhod2") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 2;
                        }

                        if ((strcmp(prikaz, "-dhod1") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 1;
                        }

                        if ((strcmp(prikaz, "-dvyhr") == 0) && jeTuEsteNiekto == MAX_CLEINTS ) {
                            hod += 40;
                        }

                        if ((strcmp(prikaz, "-dvykr") == 0) && jeTuEsteNiekto == MAX_CLEINTS) {
                            vykresli(prikaz);
                            printf("**%s",prikaz);
                            send_massage_toAll(prikaz);
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
        client->vyhral = 0;

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
    nastavPoradie(0);
    sprintf(message,"Na rade je :%s\n", clients[getPoradie()]->meno);
    send_massage_toAll(message);
    if (clientSocket < 0) {
        printError("Chyba - accept.");
    }
    while (jeTuEsteNiekto > 0) {

    }
    printf("Vsetci sa odpojili hra konci!!!\n");
    return (EXIT_SUCCESS);

}
