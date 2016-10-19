#ifndef IA
#define IA

#include "bitsboards.h"
#include "lister_coups.h"
#include "jeu_de_base.h"
#include "structures_jeu.h"
#include "hash.h"
#include "outils.h"

#define INFINI              9999
#define SCORE_MAT           1000
#define LONGUEUR_MAX_MAT    100
#define MAXI_HEURISTIQUE    10
#define T_MAX_COUPS         100

#define NB_MAX_ATTAQUANTS   10

#define PARTIE_FINIE        127
#define PROFONDEUR_MAX_ARBRE 12


#define MATOS_EN_FINALE     42

#define GET_COULEUR(a)((a) > 0 ? INDEX_BLANC : INDEX_NOIR)



int  init_ia();
void free_ia();

Coup_IA meilleur_coup(const Echiquier*, int, char, float, float);
Coup_IA meilleur_coup_avec_memoire(const Echiquier*, int, Noeud*, char, Int, float, float);
Coup_IA meilleur_coup_profondeur_auto(const Echiquier*, char, float, Int);
float   heuristique(const Echiquier*, char);
char    gain_perte(const char*, int, const char*, int, char);


#define ABS(a) ((a) > 0 ? (a) : -(a))

#endif // IA
