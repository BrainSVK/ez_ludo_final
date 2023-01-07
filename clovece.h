//
// Created by erikv on 7. 1. 2023.
//

#ifndef EZ_LUDO_CLOVECE_H
#define EZ_LUDO_CLOVECE_H

void pridajHraca(int _uid,char* _meno);
void vyhresliHraciuPlochu();
void vykresliZacDomPlochu();
void vykresliDomPlochu();
int hodKockou();
void vykresli(char* vykresli_);
int getIdHraca(int index);
void queue_remove_hrac(int uid);
void vypisStavHraca(int index,char* vykresli_);
int nastavPanacika(int index);

#endif //EZ_LUDO_CLOVECE_H
