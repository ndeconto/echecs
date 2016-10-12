#ifndef JEU_DE_BASE
#define JEU_DE_BASE

#include <stdlib.h>
#include <stdio.h>
#include "commun.h"
#include "bitsboards.h"

#define PRISE_CLASSIQUE     1
#define PRISE_EN_PASSANT    2

#define LARGEUR_COLONNE_PNG 11
void init_jeu_de_base(void);
void quit_jeu_de_base(void);

void jouer_coup(Coup*, Echiquier*, char);
void ecrire_coup(FILE*, Coup, int, char);
void ecrire_coup2(const char*, Coup, int, char);

char** get_plateau(void);
Echiquier* get_echiquier(void);
char capture(Echiquier*, Coup*);


#endif // JEU_DE_BASE
