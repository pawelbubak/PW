//
// Created by pawelbubak on 13.01.18.
//

#ifndef CHAT_UZYTKOWNIK_H
#define CHAT_UZYTKOWNIK_H
struct Uzytkownik{
    char login[16];
    char haslo[16];
    int zalogowany;
    int pid;
    int zablokowany;
};
#endif //CHAT_UZYTKOWNIK_H
