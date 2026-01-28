#ifndef WYNIKI_H
#define WYNIKI_H

#include <stdint.h>

#define MAX_PLAYERS 30
#define MAX_NICK_LENGTH 30

// Struktura gracza
struct player {
    char nick[MAX_NICK_LENGTH];
    int score;
};

// Struktura listy rankingowej
struct rankingList {
    int cPlayers;
    struct player pList[MAX_PLAYERS];
};

// Funkcje
void bsort(struct rankingList *r);
void printList(struct rankingList *r);
int lookPlayer(struct rankingList *r, char s[]);
void addPlayerData(struct player* p, const char n[], int s);
void addPlayerToList(struct rankingList *r, struct player *p);
int calculatePoints(int x, int y);

#endif
