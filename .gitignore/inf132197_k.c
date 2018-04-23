//
// Created by pawelbubak on 14.01.18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "inf132197_struktura_komunikacyjna.h"
#define GREEN 35
#define RED 31
#define BOLD 1

char login[16];

void zarzadzanie(int *wlaczony, int *zalogowany, int idKolejkiPriorytetowej, int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void znakZachety();
void znakZachetyZalogowanego();
void blad(char *tekst);
void informacje(char *tekst);
void nazwa();
void grupa(char *grupa);
void zarzadzanieLogowaniem(int *wlaczony, int *zalogowany, int *probyLogowania, int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void zalogujSie(int *zalogowany, int *probyLogowania, int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void wylogujSie(int *zalogowany, int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void wyswietlGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void wyswietlUzytkownikow(int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void wyswietlUzytkownikowGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void wyslijWiadomoscPrywatna(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int priorytet);
void wyslijWiadomoscPubliczna(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int priorytet);
void dolaczDoGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void opuscGrupe(int idKolejkiKomunikatow, int idKolejkiWiadomosci);
void odbierzWiadomosci(int idKolejkiPriorytetowej, int idKolejkiWiadomosci);

int main() {
    int zalogowany = 0;
    int idKolejkiWiadomosci = msgget(1240, IPC_CREAT | IPC_EXCL | 0644);
    if (idKolejkiWiadomosci == -1)
        idKolejkiWiadomosci = msgget(1240, IPC_CREAT | 0644);
    else{
        msgctl(idKolejkiWiadomosci,IPC_RMID,NULL);
        blad("Nie można nawiązać połączenia z serwerem!\n");
        exit(0);
    }
    int idKolejkiKomunikatow = msgget(5120, IPC_CREAT | IPC_EXCL | 0644);
    if (idKolejkiKomunikatow == -1)
        idKolejkiKomunikatow = msgget(5120, IPC_CREAT | 0644);
    else{
        msgctl(idKolejkiKomunikatow,IPC_RMID,NULL);
        blad("Nie można nawiązać połączenia z serwerem!\n");
        exit(0);
    }
    int idKolejkiPriorytetowej = msgget(2480, IPC_CREAT | IPC_EXCL | 0644);
    if (idKolejkiPriorytetowej == -1)
        idKolejkiPriorytetowej = msgget(2480, IPC_CREAT | 0644);
    else{
        msgctl(idKolejkiPriorytetowej,IPC_RMID,NULL);
        blad("Nie można nawiązać połączenia z serwerem!\n");
        exit(0);
    }

    int wlaczony = 1;
    int probyLogowania = 0;
    if (fork() == 0)
        execlp("clear", "clear", NULL);
    else {
        sleep(1);
        nazwa();
        informacje("Witaj na serwerze DarkPUT! \nAby się zalogować wywołaj komendę 'login' \nGdy czegoś nie wiesz sprawdź 'help'\n");
        while (wlaczony) {
            if (!zalogowany)
                zarzadzanieLogowaniem(&wlaczony, &zalogowany, &probyLogowania, idKolejkiKomunikatow,
                                      idKolejkiWiadomosci);
            else
                zarzadzanie(&wlaczony, &zalogowany, idKolejkiPriorytetowej, idKolejkiKomunikatow, idKolejkiWiadomosci);
        }
    }
}

void zarzadzanieLogowaniem(int *wlaczony, int *zalogowany, int *probyLogowania, int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    char wybor[16];
    znakZachety();
    scanf("%s", wybor);

    if (strcmp(wybor,"help") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        else {
            sleep(1);
            nazwa();
            informacje("Przydatne komendy serwera: 'help'\n");
            printf("    login - Zaloguj się \n\n    info - Wyświetla informacje o programie \n\n    exit - Zamknij program\n");
        }
    } else if (strcmp(wybor,"info") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        sleep(1);
        nazwa();
        informacje("Program DarkPUT Client 1.0.1\n");
        printf("Program ten ma za zadanie symulować chat - prowadzić \nwymianę informacji między użytkownikami podłączonymi do \njednego (tego samego) serwera.\n\n(s)Twórca: Paweł Bubak \nIdentyfikowany numerem: 132197\n");
    } else if (strcmp(wybor, "login") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        else {
            sleep(1);
            nazwa();
            if (msgget(1240, IPC_CREAT | IPC_EXCL | 0644) == -1) {
                zalogujSie(zalogowany, probyLogowania, idKolejkiKomunikatow, idKolejkiWiadomosci);
            } else{
                msgctl(idKolejkiWiadomosci,IPC_RMID,NULL);
                blad("Utracono połączenie z serwrem.\n");
                exit(0);
            }
        }
    } else if (strcmp(wybor, "exit") == 0){
        *wlaczony = 0;
    }
}

void zalogujSie(int *zalogowany, int *probyLogowania, int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    int pid, licznik = 0;
    char templogin[16];
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 1;
    msg.podtyp = 0;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    informacje("Login: ");
    scanf("%s",templogin);
    strcpy(msg.poleDanych,templogin);
    informacje("Hasło: ");
    scanf("%s",msg.poleDanych2);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    while (1) {
        informacje("Trwa łączenie z serwerem, proszę czekać\n");
        msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf) - sizeof(long), (long)getpid(), IPC_NOWAIT);
        if (msg.typ == 5){
            if (msg.podtyp == 0){
                if (fork() == 0)
                    execlp("clear","clear",NULL);
                else{
                    *zalogowany = 1;
                    sleep(1);
                    strcpy(login,templogin);
                    nazwa();
                    informacje("Witaj! Właśnie zostałeś zalogowany i możesz w pełni korzystać z funkcjonalności serwera.\n");
                    break;
                }
            } else if (msg.podtyp == -1){
                *probyLogowania = *probyLogowania + 1;
                if (*probyLogowania == 3){
                    *probyLogowania = 0;
                    msg.mType = 1l;
                    msg.typ = 5;
                    msg.podtyp = 0;
                    strcpy(msg.wiadomosc, pidc);
                    strcpy(msg.poleDanych, templogin);
                    strcpy(msg.poleDanych2, "3");
                    msgsnd(idKolejkiWiadomosci,&msg, sizeof(struct msgbuf)- sizeof(long),0);
                    msgrcv(idKolejkiKomunikatow,&msg, sizeof(struct msgbuf) -sizeof(long),(long)pid,0);
                    blad(msg.wiadomosc);
                }
                else
                    blad(msg.wiadomosc);
                break;
            }
        }
        sleep(4);
        licznik++;
        if (licznik == 3) {
            informacje("Logowanie...\n");
            break;
        }
    }
}

void zarzadzanie(int *wlaczony, int *zalogowany, int idKolejkiPriorytetowej, int idKolejkiKomunikatow, int idKolejkiWiadomosci){
    char wybor[16];
    znakZachetyZalogowanego();
    scanf("%s", wybor);

    if (msgget(1240, IPC_CREAT | IPC_EXCL | 0644) == -1) {
        if (strcmp(wybor,"help") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                informacje("Przydatne komendy serwera: 'help'\n");
                printf("    user - Wyświetla listę użytkowników na serwerze\n    group - Wyświetl listę dostępnych grup na serwerze\n    igroup 'nazwa' - Wyświetla nazwy użytkowników zapisanych do wskazanej grupy\n\n    agroup 'nazwa' - Dołączenie do grupy\n    rgroup 'nazwa' - Opuszczenie grupy\n\n    ppw - Priorytetowa wiadomość prywatna\n    pw - Wiadomość prywatna\n    pgw - Wiadomość grupowa priorytetowa\n    gw - Wiadomość grupowa\n\n    rcv - Odbiór wiadomości\n\n    info - Wyświetla informacje o programie\n\n    logout - Wyloguj się\n    exit - Zamknij program\n");
            }
        } else if (strcmp(wybor,"info") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            sleep(1);
            nazwa();
            informacje("Program DarkPUT Client 1.0.1\n");
            printf("Program ten ma za zadanie symulować chat - prowadzić \nwymianę informacji między użytkownikami podłączonymi do \njednego (tego samego) serwera.\n\n(s)Twórca: Paweł Bubak \nIdentyfikowany numerem: 132197\n");
        } else if (strcmp(wybor,"logout") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                wylogujSie(zalogowany, idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "group") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyswietlGrupy(idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "user") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyswietlUzytkownikow(idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "igroup") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyswietlUzytkownikowGrupy(idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "ppw") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyslijWiadomoscPrywatna(idKolejkiKomunikatow, idKolejkiWiadomosci, 0);
            }
        } else if (strcmp(wybor, "pw") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyslijWiadomoscPrywatna(idKolejkiKomunikatow, idKolejkiWiadomosci, 1);
            }
        } else if (strcmp(wybor, "pgw") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyslijWiadomoscPubliczna(idKolejkiKomunikatow, idKolejkiWiadomosci, 0);
            }
        } else if (strcmp(wybor, "gw") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                wyslijWiadomoscPubliczna(idKolejkiKomunikatow, idKolejkiWiadomosci, 1);
            }
        } else if (strcmp(wybor, "agroup") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                dolaczDoGrupy(idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "rgroup") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                opuscGrupe(idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "rcv") == 0) {
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                nazwa();
                odbierzWiadomosci(idKolejkiPriorytetowej, idKolejkiWiadomosci);
            }
        } else if (strcmp(wybor, "exit") == 0){
            if (fork() == 0)
                execlp("clear","clear",NULL);
            else {
                sleep(1);
                *wlaczony = 0;
                wylogujSie(zalogowany, idKolejkiKomunikatow, idKolejkiWiadomosci);
            }
        }
    } else {
        msgctl(idKolejkiWiadomosci, IPC_RMID, NULL);
        blad("Utracono połączenie z serwrem.\n");
        exit(0);
    }
}

void odbierzWiadomosci(int idKolejkiPriorytetowej, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    while (msgrcv(idKolejkiPriorytetowej,&msg, sizeof(struct msgbuf)- sizeof(long),(long)getpid(),IPC_NOWAIT) != -1){
        blad("[P]");
        if (msg.typ == 4) {
            grupa(msg.poleDanych2);
        }
        informacje(msg.poleDanych);
        printf(": %s\n",msg.wiadomosc);
    }
    while (msgrcv(idKolejkiWiadomosci,&msg, sizeof(struct msgbuf)- sizeof(long),(long)getpid(),IPC_NOWAIT) != -1){
        if (msg.typ == 4) {
            grupa(msg.poleDanych2);
        }
        informacje(msg.poleDanych);
        printf(": %s\n",msg.wiadomosc);
    }
}

void opuscGrupe(int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych2);
    informacje("Prośba o opuszczenie grupy ");
    informacje(msg.poleDanych2);
    informacje("\n");
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 2;
    msg.podtyp = 1;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), (long)getpid(), 0);
    if (!msg.podtyp)
        informacje(msg.wiadomosc);
    else
        blad(msg.wiadomosc);
}

void dolaczDoGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych2);
    informacje("Prośba o dołączenie do grupy ");
    informacje(msg.poleDanych2);
    informacje("\n");
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 2;
    msg.podtyp = 0;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), (long)pid, 0);
    if (!msg.podtyp)
        informacje(msg.wiadomosc);
    else
        blad(msg.wiadomosc);
}

void wyslijWiadomoscPubliczna(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int priorytet) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych2);
    if (!priorytet) {
        informacje("Proszę wprowadzić treść wiadomości priorytetowej do grupy: ");
    }
    else{
        informacje("Proszę wprowadzić treść wiadomości do grupy: ");
    }
    informacje(msg.poleDanych2);
    informacje("\n");
    int pid = getpid();
    char *line;
    size_t lineSize = 128;
    line = (char *)malloc(lineSize * sizeof(char));
    msg.mType = 1;
    msg.typ = 4;
    msg.podtyp = priorytet;
    sleep(1);
    znakZachety();
    getchar();
    //fgets(msg.wiadomosc,128,stdin);
    getline(&line,&lineSize,stdin);
    strcpy(msg.poleDanych,login);
    strcpy(msg.wiadomosc,line);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), (long)pid, 0);
    if (msg.podtyp == 0) {
        informacje(msg.wiadomosc);
    } else
        blad(msg.wiadomosc);
    free(line);
}

void wyslijWiadomoscPrywatna(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int priorytet) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych2);
    if (!priorytet) {
        informacje("Proszę wprowadzić treść wiadomości priorytetowej do ");
    }
    else{
        informacje("Proszę wprowadzić treść wiadomości do ");
    }
    informacje(msg.poleDanych2);
    informacje("\n");
    int pid = getpid();
    char *line;
    size_t lineSize = 128;
    line = (char *)malloc(lineSize * sizeof(char));
    msg.mType = 1;
    msg.typ = 3;
    msg.podtyp = priorytet;
    sleep(1);
    znakZachety();
    getchar();
    //fgets(msg.wiadomosc,128,stdin);
    getline(&line,&lineSize,stdin);
    strcpy(msg.poleDanych,login);
    strcpy(msg.wiadomosc,line);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), (long)pid, 0);
    if (msg.podtyp == 0) {
        informacje(msg.wiadomosc);
    } else
        blad(msg.wiadomosc);
    free(line);
}

void wyswietlUzytkownikowGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych2);
    informacje("Lista użytkowników przynależących do grupy ");
    informacje(msg.poleDanych2);
    informacje("\n");
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 0;
    msg.podtyp = 1;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    while (1) {
        msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), pid, 0);
        printf("    %s\n",msg.wiadomosc);
        if (strcmp(msg.poleDanych2,"koniec") == 0)
            break;
    }
}

void wyswietlUzytkownikow(int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    informacje("Lista aktywnych użytkowników na serwerze\n");
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 0;
    msg.podtyp = 0;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    while (1) {
        msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), pid, 0);
        printf("    %s\n",msg.wiadomosc);
        if (strcmp(msg.poleDanych2,"koniec") == 0)
            break;
    }
}

void wyswietlGrupy(int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    informacje("Lista dostępnych grup na serwerze\n");
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 0;
    msg.podtyp = 2;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    while (1) {
        msgrcv(idKolejkiKomunikatow, &msg, sizeof(struct msgbuf)- sizeof(long), pid, 0);
        printf("    %s\n",msg.wiadomosc);
        if (strcmp(msg.poleDanych2,"koniec") == 0)
            break;
    }
}

void wylogujSie(int *zalogowany, int idKolejkiKomunikatow, int idKolejkiWiadomosci) {
    struct msgbuf msg;
    int pid;
    char pidc[16];
    pid = getpid();
    msg.mType = 1;
    msg.typ = 1;
    msg.podtyp = 1;
    sprintf(pidc,"%d",pid);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,login);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
    msgrcv(idKolejkiKomunikatow,&msg, sizeof(struct msgbuf) -sizeof(long),(long)pid,0);
    informacje(msg.wiadomosc);
    *zalogowany = 0;
    sleep(2);
}

void znakZachety(){
    printf("%c[%dm", 0x1B, GREEN);
    printf("%c[%dm$ ", 0x1B, BOLD);
    printf("%c[%dm",0x1B,0);
}

void znakZachetyZalogowanego(){
    printf("%c[%dm", 0x1B, GREEN);
    printf("%c[%dm", 0x1B, BOLD);
    printf("%s$ ", login);
    printf("%c[%dm",0x1B,0);
}

void blad(char *tekst){
    printf("%c[%dm", 0x1B, RED);
    printf("%c[%dm", 0x1B, BOLD);
    printf("%s", tekst);
    printf("%c[%dm",0x1B,0);
}

void nazwa(){
    printf("%c[%dm", 0x1B, 36);
    printf("%c[%dm", 0x1B, BOLD);
    printf("%s", "Serwer DarkPUT\n");
    printf("%c[%dm",0x1B,0);
}


void informacje(char *tekst){
    printf("%c[%dm", 0x1B, BOLD);
    printf("%s", tekst);
    printf("%c[%dm",0x1B,0);
}

void grupa(char *grupa){
    printf("%c[%dm", 0x1B, 32);
    printf("%c[%dm", 0x1B, BOLD);
    printf("[%s]", grupa);
    printf("%c[%dm",0x1B,0);
}