#include "wyniki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Sortowanie graczy pod wzgledem wyniku
void bsort(struct rankingList *r) {
	int i, j;
	struct player temp;

	for (i = 0; i < r->cPlayers - 1; i++) {
		for (j = 0; j < r->cPlayers - i - 1; j++) {
			if (r->pList[j].score < r->pList[j + 1].score) {
				temp = r->pList[j];
				r->pList[j] = r->pList[j + 1];
				r->pList[j + 1] = temp;
			}
		}
	}
}

void printList(struct rankingList *r){
	for (int i = 0; i < r->cPlayers; i++) {
		printf("%s: %d\n", r->pList[i].nick, r->pList[i].score);
	}
}

int lookPlayer(struct rankingList *r, char s[]){
	for(int i=0; i<(r->cPlayers); i++){
		if(strcmp(r->pList[i].nick, s) == 0) return 1;
	}
	return 0;
}

//Dodanie danych do gracza
void addPlayerData(struct player* p, const char n[], int s) {
	strncpy(p->nick, n, MAX_NICK_LENGTH - 1);
	p->nick[MAX_NICK_LENGTH - 1] = '\0';
	p->score = s;
}

void addPlayerToList(struct rankingList *r, struct player *p) {
	if (r->cPlayers < MAX_PLAYERS && lookPlayer(r, p->nick) == 0) {
		r->pList[r->cPlayers] = *p;
		r->cPlayers++;
		bsort(r);
	} else if(r->cPlayers == MAX_PLAYERS ){
		printf("Liczba graczy zostala przekroczona!\n");
	}
}

int calculatePoints(int x, int y){
	if(y==0) return x*100;
	else return (x*100)/y;
}
