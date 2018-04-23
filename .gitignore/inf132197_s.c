#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/shm.h>
#include <signal.h>
#include "inf132197_struktura_komunikacyjna.h"
#include "inf132197_uzytkownik.h"
#include "inf132197_grupa.h"
#define GREEN 35
#define RED 31
#define BOLD 1
#define USERS_FILE "users.txt"
#define GROUP_FILE "group.txt"
#define BLAD_PLIKOW "\nDane zostały uszkodzone! Za chwilę nastąpi zamknięcie programu!\n"

enum bool {NO, YES};
void zarzadzanie(char *wlaczony, int idKolejkiKomunikatow);
void znakZachety();
int wczytajUzytkownikow(struct Uzytkownik *uzytkownicy);
int wczytajGrupy(struct Grupa *grupy);
void blad(char *tekst);
void informacje(char *tekst);
void nazwa();
void obslugaStanu(int idKolejkiKomunikatow, struct Uzytkownik *uzytkownicy, int liczbaUzytkownikow, int liczbaGrup, struct Grupa *grupy, struct msgbuf *msg);
void zarzadzaniePolaczeniami(int idKolejki, int liczbaUzytkownikow, struct Uzytkownik *uzytkownicy, struct msgbuf *msg);
void zarzadzanieGrupami(int idKolejki, int liczbUzytkownikow, int liczbaGrup, struct Uzytkownik *uzytkownicy,
                        struct Grupa *grupy, struct msgbuf *msg);
void wiadomosciPrywatne(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int idKolejkiPriorytetowej, int liczbaUzytkownikow,
                        struct Uzytkownik *uzytkownicy, struct msgbuf *msg);
void komunikaty(int idKolejkiKomunikatow, int liczbaUzytkownikow, struct Uzytkownik *uzytkownicy, struct msgbuf *msg);
void wiadomosciPubliczne(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int idKolejkiPriorytetowej,
                         int liczbaUzytkownikow, int liczbaGrup, struct Uzytkownik *uzytkownicy, struct Grupa *grupy,
                         struct msgbuf *msg);

void wyswietlUzytkownikow(int idKolejkiWiadomosci);

void odblokujUzytkownikow(int idKolejkiWiadomosci, int tryb);

int main() {
    struct Uzytkownik uzytkownicy[100];
    struct Grupa grupy[100];
    int liczbaUzytkownikow, liczbaGrup;
    int id_pamieci = shmget(IPC_PRIVATE, sizeof(char), IPC_CREAT | 0644);
    int idKolejkiWiadomosci = msgget(1240, IPC_CREAT|IPC_EXCL|0644);
    if (idKolejkiWiadomosci == -1)
        idKolejkiWiadomosci = msgget(1240,IPC_CREAT|0644);

    if (fork() != 0){
        char* wlaczony = (char*)shmat(id_pamieci,NULL,0);
        //printf("ustawiam wlaczony\n");
        *wlaczony = YES;

        if (fork() == 0)
            execlp("clear","clear",NULL);

        char dzialanie = YES;
        sleep(1);
        nazwa();
        printf("Witaj administratorze! \nAby wyświetlić możliwości zarządzania serwerem wpisz: 'help'\n");
        znakZachety();
        while(dzialanie == YES){
            zarzadzanie(wlaczony, idKolejkiWiadomosci);
            sleep(1);
            dzialanie = *wlaczony;
        }
    }
    else {
        char* wlaczony = (char*)shmat(id_pamieci,NULL,IPC_NOWAIT);
        struct msgbuf msg;

        int idKolejkiKomunikatow = msgget(5120, IPC_CREAT|IPC_EXCL|0644);
        if (idKolejkiKomunikatow == -1)
            idKolejkiKomunikatow = msgget(5120,IPC_CREAT|0644);
        int idKolejkiPriorytetowej = msgget(2480, IPC_CREAT|IPC_EXCL|0644);
        if (idKolejkiPriorytetowej == -1)
            idKolejkiPriorytetowej = msgget(2480,IPC_CREAT|0644);


        liczbaUzytkownikow = wczytajUzytkownikow(uzytkownicy);
        liczbaGrup = wczytajGrupy(grupy);

        sleep(2);
        char dzialanie = YES;
        if (liczbaGrup == -1 || liczbaUzytkownikow == -1){
            blad(BLAD_PLIKOW);
            sleep(2);
            *wlaczony = NO;
            informacje("Proszę wpisać dowolny ciąg znaków oraz potwierdzić enterem, aby zamknąć serwer!\n");
            znakZachety();
        }
        else {
            while (dzialanie == YES) {
                if (msgrcv(idKolejkiWiadomosci, &msg, sizeof(msg) - sizeof(long), 1l, IPC_NOWAIT) != -1) {
                    switch (msg.typ) {
                        case 0:
                            obslugaStanu(idKolejkiKomunikatow, uzytkownicy, liczbaUzytkownikow, liczbaGrup, grupy,
                                         &msg);
                            break;
                        case 1:
                            zarzadzaniePolaczeniami(idKolejkiKomunikatow, liczbaUzytkownikow, uzytkownicy, &msg);
                            break;
                        case 2:
                            zarzadzanieGrupami(idKolejkiKomunikatow, liczbaGrup, liczbaUzytkownikow, uzytkownicy, grupy,
                                               &msg);
                            break;
                        case 3:
                            wiadomosciPrywatne(idKolejkiKomunikatow, idKolejkiWiadomosci, idKolejkiPriorytetowej,
                                               liczbaUzytkownikow, uzytkownicy, &msg);
                            break;
                        case 4:
                            wiadomosciPubliczne(idKolejkiKomunikatow, idKolejkiWiadomosci, idKolejkiPriorytetowej,
                                                liczbaUzytkownikow, liczbaGrup, uzytkownicy, grupy, &msg);
                            break;
                        case 5:
                            komunikaty(idKolejkiKomunikatow, liczbaUzytkownikow, uzytkownicy, &msg);
                            break;
                    }
                }
                sleep(1);
                dzialanie = *wlaczony;
            }
            for (int i = 0; i < liczbaUzytkownikow; i++){
                if (uzytkownicy[i].pid > 0){
                    kill(uzytkownicy[i].pid,9);
                }
            }
        }
        msgctl(idKolejkiPriorytetowej,IPC_RMID,NULL);
        msgctl(idKolejkiKomunikatow,IPC_RMID,NULL);
    }
    msgctl(idKolejkiWiadomosci,IPC_RMID,NULL);
    shmctl(id_pamieci,IPC_RMID,NULL);
}


int wczytajUzytkownikow(struct Uzytkownik *uzytkownicy){
    FILE* file = fopen(USERS_FILE,"r");

    if (file == NULL)
        return -1;

    int licznik = 0;
    while (!feof(file)){
        if (fscanf(file, "%s %s", uzytkownicy[licznik].login, uzytkownicy[licznik].haslo) < 2)
            return -1;
        uzytkownicy[licznik].zalogowany = 0;
        uzytkownicy[licznik].pid = 0;
        uzytkownicy[licznik].zablokowany = 0;

        licznik++;
        if (licznik == 100)
            break;
    }
    fclose(file);
    return licznik;
}

int wczytajGrupy(struct Grupa *grupy){
    FILE* file = fopen(GROUP_FILE,"r");

    if (file == NULL)
        return -1;

    int licznik = 0;
    while (!feof(file)){
        if (fscanf(file, "%s %d", grupy[licznik].nazwa, &grupy[licznik].iloscUzytkownikow) < 2)
            return -1;

        for (int i = 0; i < grupy[licznik].iloscUzytkownikow; i++){
            if (fscanf(file, "%s", grupy[licznik].uzytkownicy[i]) < 1)
                return -1;
        }
        licznik++;
        if (licznik == 100)
            break;
    }
    fclose(file);
    return licznik;
}

void zarzadzanie(char *wlaczony, int idKolejkiKomunikatow){
    char wybor[16];
    //znakZachety();
    scanf("%s", wybor);

    if (strcmp(wybor,"help") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        else {
            sleep(1);
            nazwa();
            informacje("Przydatne komendy serwera: 'help'\n");
            printf("    info - Informacje o programie\n\n    users - wyswietl liste aktywnych uzytkownikow\n    unlock 'login' - Odblokowuje wskazanego użytkownika\n    unlock all - Odblokowuje wszystkich zablokowanych użytkowników\n\n    exit - zamknij\n");
            znakZachety();
        }
    } else if (strcmp(wybor, "users") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        else {
            sleep(1);
            nazwa();
            wyswietlUzytkownikow(idKolejkiKomunikatow);
            sleep(1);
            znakZachety();
        }
    } else if (strcmp(wybor, "unlock") == 0) {
        if (fork() == 0)
            execlp("clear","clear",NULL);
        else {
            sleep(1);
            nazwa();
            sleep(1);
            odblokujUzytkownikow(idKolejkiKomunikatow, 0);
            sleep(1);
            znakZachety();
        }
    } else if (strcmp(wybor,"info") == 0) {
        if (fork() == 0)
            execlp("clear", "clear", NULL);
        else {
            sleep(1);
            nazwa();
            informacje("Program DarkPUT Serwer 1.0.1\n");
            printf("Program ten ma za zadanie symulować chat \n(czyt. uzależnić od siebie użytkowników i przejąć nad nimi kontrolę) - prowadzić\nwymianę informacji między użytkownikami podłączonymi do tego serwera.\n\n(s)Twórca: Paweł Bubak \nIdentyfikowany numerem: 132197\n");
            znakZachety();
        }
    } else if (strcmp(wybor, "exit") == 0){
        *wlaczony = NO;
    } else {
        blad("Błędna komenda\n");
        znakZachety();
    }
}

void odblokujUzytkownikow(int idKolejkiWiadomosci, int tryb) {
    struct msgbuf msg;
    scanf("%s",msg.poleDanych);
    char pidc[16];
    msg.mType = 1;
    msg.typ = 0;
    msg.podtyp = 8;
    sprintf(pidc,"%d",1);
    strcpy(msg.wiadomosc,pidc);
    if (strcmp(msg.poleDanych,"all") == 0)
        sprintf(msg.poleDanych2,"%d",tryb);
    else
        sprintf(msg.poleDanych2,"%d",1);
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
}

void wyswietlUzytkownikow(int idKolejkiWiadomosci) {
    struct msgbuf msg;
    informacje("Lista użytkowników na serwerze\n");
    sleep(1);
    char pidc[16];
    msg.mType = 1;
    msg.typ = 0;
    msg.podtyp = 7;
    sprintf(pidc,"%d",1);
    strcpy(msg.wiadomosc,pidc);
    strcpy(msg.poleDanych,"admin");
    msgsnd(idKolejkiWiadomosci, &msg, sizeof(struct msgbuf)- sizeof(long),0);
}

void znakZachety(){
    printf("%c[%dm", 0x1B, GREEN);
    printf("%c[%dmadmin$ ", 0x1B, BOLD);
    printf("%c[%dm",0x1B,0);
}

void blad(char *tekst){
    printf("%c[%dm", 0x1B, RED);
    printf("%c[%dm", 0x1B, BOLD);
    printf("%s", tekst);
    printf("%c[%dm",0x1B,0);
}

void obslugaStanu(int idKolejkiKomunikatow, struct Uzytkownik *uzytkownicy, int liczbaUzytkownikow, int liczbaGrup, struct Grupa *grupy, struct msgbuf *msg) {
    switch (msg->podtyp){
        case 0:
            msg->mType = (long)atoi(msg->wiadomosc);
            msg->typ = 0;
            msg->podtyp = 0;
            sprintf(msg->poleDanych2,"%d",1);
            strcpy(msg->wiadomosc,"");
            int koniec = 0;
            int licznik2 = 0;
            for (int i = 0; i < liczbaUzytkownikow; i++){
                if (uzytkownicy[i].zalogowany){
                    licznik2++;
                    strcpy(msg->wiadomosc,uzytkownicy[i].login);
                    if (i == liczbaUzytkownikow - 1){
                        koniec = 1;
                        sprintf(msg->poleDanych2,"%s","koniec");
                    }
                    msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
                }
            }
            if (licznik2 == 0){
                strcpy(msg->wiadomosc,"Nie ma aktualnie zalogowanych użytkowników na serwerze!\n");
                sprintf(msg->poleDanych2,"%s","koniec");
                msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
            } else if (!koniec){
                strcpy(msg->wiadomosc, "");
                strcpy(msg->poleDanych2,"koniec");
                msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
            }
            break;
        case 1:
            msg->mType = (long)atoi(msg->wiadomosc);
            msg->typ = 0;
            msg->podtyp = 0;
            //printf("%d %s",liczbaGrup,msg->poleDanych2);
            int liczniku2 = 0;
            int liczniku = 0;
            for (int i = 0; i < liczbaGrup; i++){
                if (strcmp(grupy[i].nazwa,msg->poleDanych2) == 0){
                    liczniku2++;
                    for (int j = 0; j < grupy[i].iloscUzytkownikow; j++) {
                        liczniku++;
                        strcpy(msg->wiadomosc, grupy[i].uzytkownicy[j]);
                        if (j == grupy[i].iloscUzytkownikow - 1) {
                            sprintf(msg->poleDanych2, "%s", "koniec");
                        }
                        msgsnd(idKolejkiKomunikatow, msg, sizeof(struct msgbuf) - sizeof(long), 0);
                    }
                }
            }
            sprintf(msg->poleDanych2, "%s", "koniec");
            if (liczniku2 == 0){
                strcpy(msg->wiadomosc,"Nie ma takiej grupy na serwerze!\n");
                msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
            }
            else if (liczniku == 0) {
                strcpy(msg->wiadomosc, "Do grupy nie należy żaden użytkownik!\n");
                msgsnd(idKolejkiKomunikatow, msg, sizeof(struct msgbuf) - sizeof(long), 0);
            }
            break;
        case 2:
            msg->mType = (long)atoi(msg->wiadomosc);
            msg->typ = 0;
            msg->podtyp = 0;
            sprintf(msg->poleDanych2,"%d",1);
            //printf("%d", liczbaGrup);
            int licznikg2 = 0;
            for (int i = 0; i < liczbaGrup; i++){
                strcpy(msg->wiadomosc,grupy[i].nazwa);
                licznikg2++;
                if (i == liczbaGrup - 1){
                    sprintf(msg->poleDanych2,"%s","koniec");
                }
                msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
            }
            if (licznikg2 == 0){
                strcpy(msg->wiadomosc,"Nie ma aktualnie żadnych grup na serwerze!\n");
                sprintf(msg->poleDanych2,"%s","koniec");
                msgsnd(idKolejkiKomunikatow,msg, sizeof(struct msgbuf)- sizeof(long),0);
            }
            break;
        case 7:
            for (int i = 0; i < liczbaUzytkownikow; i++) {
                if (uzytkownicy[i].zalogowany) {
                    informacje("    * ");
                    informacje(uzytkownicy[i].login);
                    informacje("\n");
                } else {
                    printf("%c[%dm",0x1B,0);
                    printf("      ");
                    printf("%s\n", uzytkownicy[i].login);
                }
            }
            //znakZachety();
            break;
        case 8:
            if (!atoi(msg->poleDanych2)){
                for (int i = 0; i < liczbaUzytkownikow; i++){
                    uzytkownicy[i].zablokowany = 0;
                }
                informacje("Wszyscy użytkownicy zostali odblokowani\n");
            } else {
                int sprawdzam = 0;
                for (int i = 0; i < liczbaUzytkownikow; i++){
                    if (strcmp(uzytkownicy[i].login,msg->poleDanych) == 0) {
                        uzytkownicy[i].zablokowany = 0;
                        sprawdzam++;
                        informacje("Użytkownik ");
                        informacje(uzytkownicy[i].login);
                        informacje(" został odblokowany\n");
                    }
                }
                if (!sprawdzam){
                    blad("Nie istnieje taki użytkownik!\n");
                }
            }
            //znakZachety();
            break;
    }
}

void zarzadzaniePolaczeniami(int idKolejki, int liczbaUzytkownikow, struct Uzytkownik *uzytkownicy, struct msgbuf *msg) {
    int weryfikacjaI = 0;
    int weryfikacjaII = 0;
    //printf("jestem w zarzadzaniu polaczeniami\n");
    switch (msg->podtyp){
        case 0:
            //printf("dostalem prosbe logowania\n");
            for (int i = 0; i < liczbaUzytkownikow; i++){
                if (strcmp(msg->poleDanych, uzytkownicy[i].login) == 0) {
                    //printf("znalazlem uzytkoniwka\n");
                    if (uzytkownicy[i].zablokowany){
                        //printf("jest zablokowany\n");
                        msg->mType = (long)atoi(msg->wiadomosc);
                        msg->typ = 5;
                        msg->podtyp = -1;
                        strcpy(msg->wiadomosc, "Nie możesz się zalogować, ponieważ Twoje konto zostało zablokowane!\n");
                        msgsnd(idKolejki, msg, sizeof(struct msgbuf)-sizeof(long), 0);
                        return;
                    }
                    //printf("test\n");
                    weryfikacjaI = 1;
                    if (weryfikacjaI){
                        if (strcmp(msg->poleDanych2,uzytkownicy[i].haslo) == 0) {
                            weryfikacjaII = 1;
                            if (uzytkownicy[i].zalogowany){
                                msg->mType = (long)atoi(msg->wiadomosc);
                                msg->typ = 5;
                                msg->podtyp = -1;
                                strcpy(msg->wiadomosc, "Użytkownik o podanym loginie jest już zalogowany!\n");
                                msgsnd(idKolejki, msg, sizeof(struct msgbuf)- sizeof(long), 0);
                                return;
                            }
                        }
                        if (weryfikacjaII){
                            //printf("loguje uzytkownika\n");
                            uzytkownicy[i].zalogowany = 1;
                            uzytkownicy[i].pid = atoi(msg->wiadomosc);
                            msg->mType = uzytkownicy[i].pid;
                            msg->typ = 5;
                            msg->podtyp = 0;
                            strcpy(msg->wiadomosc, "Witaj! Zostałeś zalogowany!\n");
                            msgsnd(idKolejki, msg, sizeof(struct msgbuf)-sizeof(long), 0);
                            return;
                        }

                    }
                }
            }
            //printf("bledne haslo uzytkownika\n");
            if (!weryfikacjaI || !weryfikacjaII){
                msg->mType = (long)atoi(msg->wiadomosc);
                msg->typ = 5;
                msg->podtyp = -1;
                strcpy(msg->wiadomosc, "Błędny login lub hasło!\n");
                msgsnd(idKolejki, msg, sizeof(struct msgbuf)- sizeof(long), 0);
            }
            //printf("wyslalem\n");
            break;
        case 1:
            for (int i = 0; i < liczbaUzytkownikow; i++){
                if (strcmp(msg->poleDanych,uzytkownicy[i].login) == 0){
                    msg->mType = (long)atoi(msg->wiadomosc);
                    msg->typ = 5;
                    msg->podtyp = 0;
                    strcpy(msg->wiadomosc, "Zostałeś wylogowany!\n");
                    msgsnd(idKolejki,msg,sizeof(struct msgbuf)- sizeof(long), 0);
                    uzytkownicy[i].zalogowany = 0;
                    uzytkownicy[i].pid = 0;
                    //printf("wylogowalem\n");
                }
            }
            break;
    }
}

void zarzadzanieGrupami(int idKolejki, int liczbUzytkownikow, int liczbaGrup, struct Uzytkownik *uzytkownicy,
                        struct Grupa *grupy, struct msgbuf *msg) {
    //printf("wszedlem do zarzadzania grupami\n");
    int przesun = 0, ograniczenie;
    struct msgbuf odpmsg;
    switch (msg->podtyp){
        case 0:
            for (int i = 0; i < liczbaGrup; i++){
                if (strcmp(msg->poleDanych2,grupy[i].nazwa) == 0){
                    //printf("znalazlem grupe\n");
                    for (int k = 0; k < grupy[i].iloscUzytkownikow; k++){
                        if (strcmp(msg->poleDanych,grupy[i].uzytkownicy[k]) == 0){
                            odpmsg.mType = (long)atoi(msg->wiadomosc);
                            odpmsg.typ = 5;
                            odpmsg.podtyp = -1;
                            strcpy(odpmsg.wiadomosc, "Użytkownik jest już zapisany do tej grupy!\n");
                            msgsnd(idKolejki, &odpmsg, sizeof(struct msgbuf)- sizeof(long), 0);
                            return;
                        }
                    }
                    for (int j = 0; j < liczbUzytkownikow; j++){
                        if (strcmp(msg->poleDanych,uzytkownicy[j].login) == 0){
                            strcpy(grupy[i].uzytkownicy[grupy[i].iloscUzytkownikow],uzytkownicy[j].login);
                            grupy[i].iloscUzytkownikow++;
                            odpmsg.mType = (long)uzytkownicy[j].pid;
                            odpmsg.typ = 5;
                            odpmsg.podtyp = 0;
                            strcpy(odpmsg.wiadomosc, "Zostałeś dodany do grupy!\n");
                            msgsnd(idKolejki, &odpmsg, sizeof(struct msgbuf)- sizeof(long),0);
                            return;
                        }
                    }
                }
            }
            //printf("nie znalzlem grupy\n");
            odpmsg.mType = (long)atoi(msg->wiadomosc);
            odpmsg.typ = 5;
            odpmsg.podtyp = -1;
            strcpy(odpmsg.wiadomosc, "Wystąpił błąd, nie można dołączyć do grupy - podana grupa nie istnieje lub limit miejsc został wyczerpany!\n");
            msgsnd(idKolejki, &odpmsg, sizeof(struct msgbuf)- sizeof(long), 0);
            break;
        case 1:
            for (int i = 0; i < liczbaGrup; i++){
                if (strcmp(msg->poleDanych2,grupy[i].nazwa) == 0){
                    ograniczenie = grupy[i].iloscUzytkownikow;
                    for (int j = 0; j < ograniczenie; j++){
                        if (przesun && j < ograniczenie - 1){
                            strcpy(grupy[i].uzytkownicy[j-1],grupy[i].uzytkownicy[j]);
                        }
                        if (strcmp(msg->poleDanych,grupy[i].uzytkownicy[j]) == 0){
                            //printf("znalazlem grupe\n");
                            strcpy(grupy[i].uzytkownicy[j],"");
                            grupy[i].iloscUzytkownikow--;
                            przesun = 1;
                            msg->mType = (long)atoi(msg->wiadomosc);
                            //printf("%s",msg->wiadomosc);
                            msg->typ = 5;
                            msg->podtyp = 0;
                            strcpy(msg->wiadomosc, "Zostałeś usunięty z grupy!\n");
                            msgsnd(idKolejki,msg, sizeof(struct msgbuf)- sizeof(long),0);
                            //printf("wyslalem komunikat\n");
                        }
                    }
                }
            }
            msg->mType = (long)atoi(msg->wiadomosc);
            msg->typ = 5;
            msg->podtyp = -1;
            strcpy(msg->wiadomosc, "Wystąpił błąd!\n");
            msgsnd(idKolejki, msg, sizeof(struct msgbuf)- sizeof(long), 0);

            break;
    }
}

void wiadomosciPrywatne(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int idKolejkiPriorytetowej, int liczbaUzytkownikow,
                        struct Uzytkownik *uzytkownicy, struct msgbuf *msg) {
    struct msgbuf odpmsg;
    //printf("odebralem wiadomosc priorytetowa\n");
    switch (msg->podtyp){
        case 0:
            for (int i = 0; i < liczbaUzytkownikow; i++) {
                if (strcmp(msg->poleDanych2, uzytkownicy[i].login) == 0) {
                    if (uzytkownicy[i].zalogowany) {
                        //printf("znalazlem uzytkownika - jest zalogowany\n");
                        msg->mType = (long) uzytkownicy[i].pid;
                        msgsnd(idKolejkiPriorytetowej, msg, sizeof(struct msgbuf) - sizeof(long), 0);
                        for (int j = 0; j < liczbaUzytkownikow; j++) {
                            if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                                odpmsg.mType = (long) uzytkownicy[j].pid;
                                odpmsg.typ = 5;
                                odpmsg.podtyp = 0;
                                strcpy(odpmsg.wiadomosc, "Wiadomość została wysłana\n");
                                msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                                return;
                            }
                        }
                    } else {
                        //printf("znalazlem uzytkownika - nie jest zalogowany\n");
                        for (int j = 0; j < liczbaUzytkownikow; j++) {
                            if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                                odpmsg.mType = (long)uzytkownicy[j].pid;
                                odpmsg.typ = 5;
                                odpmsg.podtyp = -1;
                                strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana, ponieważ użytkownik nie jest zalogowany!\n");
                                msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                                return;
                            }
                        }
                    }
                }
            }
            //printf("nie znalazlem uzytkownika\n");
            for (int j = 0; j < liczbaUzytkownikow; j++) {
                if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                    odpmsg.mType = uzytkownicy[j].pid;
                    odpmsg.typ = 5;
                    odpmsg.podtyp = -1;
                    strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana, ponieważ podany użytkownik nie istnieje!\n");
                    msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                }
            }
            break;
        case 1:
            for (int i = 0; i < liczbaUzytkownikow; i++) {
                if (strcmp(msg->poleDanych2, uzytkownicy[i].login) == 0) {
                    if (uzytkownicy[i].zalogowany) {
                        //printf("znalazlem uzytkownika - jest zalogowany\n");
                        msg->mType = (long) uzytkownicy[i].pid;
                        msgsnd(idKolejkiWiadomosci, msg, sizeof(struct msgbuf) - sizeof(long), 0);
                        for (int j = 0; j < liczbaUzytkownikow; j++) {
                            if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                                odpmsg.mType = (long) uzytkownicy[j].pid;
                                odpmsg.typ = 5;
                                odpmsg.podtyp = 0;
                                strcpy(odpmsg.wiadomosc, "Wiadomość została wysłana\n");
                                msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                                return;
                            }
                        }
                    } else {
                        //printf("znalazlem uzytkownika - nie jest zalogowany\n");
                        for (int j = 0; j < liczbaUzytkownikow; j++) {
                            if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                                odpmsg.mType = (long)uzytkownicy[j].pid;
                                odpmsg.typ = 5;
                                odpmsg.podtyp = -1;
                                strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana, ponieważ użytkownik nie jest zalogowany!\n");
                                msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                                return;
                            }
                        }
                    }
                }
            }
            //printf("nie znalazlem uzytkownika\n");
            for (int j = 0; j < liczbaUzytkownikow; j++) {
                if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
                    odpmsg.mType = uzytkownicy[j].pid;
                    odpmsg.typ = 5;
                    odpmsg.podtyp = -1;
                    strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana, ponieważ podany użytkownik nie istnieje!\n");
                    msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
                }
            }
            break;
    }
}


void wiadomosciPubliczne(int idKolejkiKomunikatow, int idKolejkiWiadomosci, int idKolejkiPriorytetowej,
                         int liczbaUzytkownikow, int liczbaGrup, struct Uzytkownik *uzytkownicy, struct Grupa *grupy,
                         struct msgbuf *msg) {
    //printf("odbieram wiadomosc grupowa\n");
    int licznik = 0;
    int licznik2 = 0;
    int idKolejki = 0;
    struct msgbuf odpmsg;
    switch (msg->podtyp){
        case 0:
            idKolejki = idKolejkiPriorytetowej;
            break;
        case 1:
            idKolejki = idKolejkiWiadomosci;
            break;
    }
    for (int i = 0; i < liczbaGrup; i++){
        if (strcmp(msg->poleDanych2,grupy[i].nazwa) == 0){
            licznik2++;
            //printf("znalazlem grupe\n");
            for (int j = 0; j < grupy[i].iloscUzytkownikow; j++){
                for (int k = 0; k < liczbaUzytkownikow; k++){
                    if(strcmp(grupy[i].uzytkownicy[j],uzytkownicy[k].login) == 0) {
                        if(uzytkownicy[k].zalogowany) {
                            //printf("wysylam wiadomosc\n");
                            msg->mType = (long) uzytkownicy[k].pid;
                            msgsnd(idKolejki, msg, sizeof(struct msgbuf) - sizeof(long), 0);
                            licznik++;
                        }
                    }
                }
            }
        }
    }
    //printf("odpowiadam\n");
    for (int j = 0; j < liczbaUzytkownikow; j++) {
        if (strcmp(msg->poleDanych, uzytkownicy[j].login) == 0) {
            odpmsg.mType = (long) uzytkownicy[j].pid;
            odpmsg.typ = 5;
            if (licznik > 0) {
                odpmsg.podtyp = 0;
                strcpy(odpmsg.wiadomosc, "Wiadomość została wysłana\n");
            } else if (licznik2 == 0) {
                odpmsg.podtyp = -1;
                strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana, ponieważ nie istnieje grupa o tej nazwie!\n");
            } else {
                odpmsg.podtyp = -1;
                strcpy(odpmsg.wiadomosc, "Wiadomość nie mogła zostać wysłana\n");
            }
            msgsnd(idKolejkiKomunikatow, &odpmsg, sizeof(struct msgbuf) - sizeof(long), 0);
        }
    }
}

void komunikaty(int idKolejkiKomunikatow, int liczbaUzytkownikow, struct Uzytkownik *uzytkownicy, struct msgbuf *msg) {
    switch (msg->podtyp){
        case 0:
            if (atoi(msg->poleDanych2) == 3){
                for (int i = 0; i < liczbaUzytkownikow; i++){
                    if (strcmp(msg->poleDanych, uzytkownicy[i].login) == 0){
                        uzytkownicy[i].zablokowany = 1;
                        msg->mType = (long)atoi(msg->wiadomosc);
                        strcpy(msg->wiadomosc, "Podano 3 raz błędne hasło! Konto zostało zablokowane.\n");
                        msgsnd(idKolejkiKomunikatow, msg, sizeof(struct msgbuf)- sizeof(long),0);
                        return;
                    }
                }
            }
            break;
    }
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
