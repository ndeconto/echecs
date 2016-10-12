#ifndef LISTER_COUPS
#define LISTER_COUPS


#include "bitsboards.h"
#include "jeu_de_base.h"
#include "commun.h"

#define PAT     -1
#define MAT     -2

int pseudo_legaux(const Echiquier*, char, Coup*);
int liste_coups_valides(const Echiquier*, char, Coup*);

void print_l_coup(const Coup*, int);
char is_in_liste(const Coup*, int, Coup);
char est_attaque(const Echiquier*, char, char, char*);


//cf bitsboards.h pour la def du type L_cases_x4
//cf lister_coups.c pour l'utilisation de cette macro
#define PARCOURS_Lx4(echiquier, ref, l_coup, couleur, nb, dep, var1, var2, var3) \
    for ((var1) = 0; (var1) < 4; (var1)++){ \
        for (var2 = 0; var2 < ref.taille[var1]; var2++){\
            var3 = (echiquier)->plateau[ref.liste[var1][var2].y][ref.liste[var1][var2].x] * couleur;\
            if ((var3) <= VIDE){\
                l_coup[nb].case_dep = dep;\
                l_coup[(nb)++].case_fin = ref.liste[var1][var2];\
            }\
            if (var3) break;\
        }\
    }\


#define PARCOURS_L(echiquier, ref, l_coup, couleur, nb, dep, var1)\
    for (var1 = 0; (var1) < (ref).taille; (var1)++) if (echiquier->plateau[(ref).liste[var1].y][(ref).liste[var1].x] * couleur <= 0) {\
        l_coup[nb].case_dep = dep;\
        l_coup[(nb)++].case_fin = (ref).liste[var1];\
    }


#define ADD_COUP(l_coup, nb, dep, x_fin, y_fin)\
    (l_coup)[nb].case_dep = dep;\
    (l_coup)[nb].case_fin.x = (x_fin);\
    (l_coup)[(nb)++].case_fin.y = (y_fin);

#define PROMOTION(l_coup, i, i2, nb, couleur, delta_x)\
    char choix[] = {DAME, TOUR, CAVALIER, FOU};\
    if (((i) == 1 && (couleur) == BLANC) || ((i) == 6 && (couleur) == NOIR)) \
    {   \
        for (i2 = 0; i2 < 4; i2++) {\
            l_coup[(nb) + i2 - 1].case_dep.x = l_coup[(nb) - 1].case_dep.x;\
            l_coup[(nb) + i2 - 1].case_dep.y = l_coup[(nb) - 1].case_dep.y;\
            l_coup[(nb) + i2 - 1].case_fin.x = l_coup[(nb) - 1].case_dep.x + delta_x;\
            l_coup[(nb) + i2 - 1].case_fin.y = 7 + choix[i2];\
        }\
        nb += 3;\
    }

#endif // LISTER_COUPS
