#ifndef OUTILS
#define OUTILS

#include "structures_jeu.h"


#define FIN_LISTE_COUPS     255
#define BIG_PREMIER 16108604210150446087

char init_outils();
void free_outils();

void maj_aleat();
float nb_aleat(float, float);
char bit_aleat();

char allouer_noeud(Noeud*, int);
void liberer_arbre(Noeud*);
void tri_fusion(float*, int, Uint8*);

#endif // OUTILS
