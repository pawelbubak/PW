//
// Created by pawelbubak on 13.01.18.
//

#ifndef CHAT_STRUKTURA_KOMUNIKACYJNA_H
#define CHAT_STRUKTURA_KOMUNIKACYJNA_H

struct msgbuf{
    long mType;
    int typ;
    int podtyp;
    char wiadomosc[128];
    char poleDanych[16];
    char poleDanych2[16];
};

#endif //CHAT_STRUKTURA_KOMUNIKACYJNA_H
