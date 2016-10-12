#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "bitsboards.h"

Int* MOUV_TOUR;
Int* MOUV_CAVALIER;
Int* MOUV_FOU;
Int* MOUV_DAME;
Int* MOUV_ROI;
Int* MOUV_PION;

Int** ENTRE_LIGNE;
Int** ENTRE_DIAGONALE;

L_cases* L_CASES_CAVALIER;
L_cases* L_CASES_ROI;
L_cases_x4* L_CASES_FOU;
L_cases_x4* L_CASES_TOUR;


//voir fonction bb_scan_droite
char INDEX_BIT_SCAN[] = { 0, 1, 2, 7, 3, 13, 8, 19,
                    4, 25, 14, 28, 9, 34, 20, 40,
                    5, 17, 26, 38, 15, 46, 29, 48,
                    10, 31, 35, 54, 21, 50, 41, 57,
                    63, 6, 12, 18, 24, 27, 33, 39,
                    16, 37, 45, 47, 30, 53, 49, 56,
                    62, 11, 23, 32, 36, 44, 52, 55,
                    61, 22, 43, 51, 60, 42, 59, 58
                };



void init_bitsboards(Echiquier* echiquier)
{
    //initialisation des bitboards

    int i, j, p, q;
    Int mask;

    MOUV_TOUR = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);
    MOUV_FOU  = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);
    MOUV_DAME = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);
    MOUV_CAVALIER = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);

    L_CASES_CAVALIER    = malloc(sizeof(L_cases) * NB_CASES_ECHIQUIER);
    L_CASES_FOU         = malloc(sizeof(L_cases_x4) * NB_CASES_ECHIQUIER);
    L_CASES_TOUR        = malloc(sizeof(L_cases_x4) * NB_CASES_ECHIQUIER);
    L_CASES_ROI         = malloc(sizeof(L_cases) * NB_CASES_ECHIQUIER);

    for (i = 0; i < 2; i++) for (j = 0; j < NB_PIECES; j++) echiquier->bb_par_piece[i][j] = 0;


    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++)
    {
        peut_aller_tour_bb(j, i, MOUV_TOUR + INDEX(i, j));
        peut_aller_fou_bb(j, i, MOUV_FOU + INDEX(i, j));
        MOUV_DAME[INDEX(i, j)] = MOUV_FOU[INDEX(i, j)] | MOUV_TOUR[INDEX(i, j)];
        peut_aller_cavalier_bb(j, i, MOUV_CAVALIER + INDEX(i, j));

        init_l_cases_cavalier(j, i);
        init_l_cases_fou_tour(j, i);
        init_l_cases_roi(j, i);


        p = echiquier->plateau[i][j];
        q = (p > 0 ? INDEX_BLANC : INDEX_NOIR);
        if (abs(p) == FOU || abs(p) == DAME)    echiquier->bb_par_piece[q][FOU]     |= ((Int) 1 << INDEX(i, j));
        if (abs(p) == TOUR || abs(p) == DAME)   echiquier->bb_par_piece[q][TOUR]    |= ((Int) 1 << INDEX(i, j));
        if (abs(p) == CAVALIER)                 echiquier->bb_par_piece[q][CAVALIER]|= ((Int) 1 << INDEX(i, j));

    }

    init_entre();

    echiquier->pieces_couleur[INDEX_NOIR] = (Int) 0xffff;
    echiquier->pieces_couleur[INDEX_BLANC] = (Int) 0xffff << (6 * 8);


}

void free_bitsboards()
{
    int i, id;

    for (id = 0; id < 64; id++)
    {
        free(ENTRE_LIGNE[id]);
        free(ENTRE_DIAGONALE[id]);

        for (i = 0; i < 4; i++)
        {
            free(L_CASES_FOU[id].liste[i]);
            free(L_CASES_TOUR[id].liste[i]);
        }

        free(L_CASES_ROI[id].liste);
        free(L_CASES_CAVALIER[id].liste);
    }

    free(ENTRE_LIGNE);
    free(ENTRE_DIAGONALE);

    free(L_CASES_CAVALIER);
    free(L_CASES_FOU);
    free(L_CASES_TOUR);

    free(MOUV_CAVALIER);
    free(MOUV_DAME);
    free(MOUV_FOU);
    free(MOUV_TOUR);
}

void print_bb(Int bb)
{
    int i;

    fprintf(stderr, "%llu = \n", bb);

    for (i = 0; i < 64; i++)
    {
        fprintf(stderr, "%d ", (int) bb & 1);
        bb >>= 1;

        if ((i & 7) == 7) fprintf(stderr, "\n");
    }
}

void peut_aller_tour_bb(int x_dep, int y_dep, Int* bb)
{
    /*
        les bits correspondants aux cases ou la tour peut aller depuis la case de coordonnees (x_dep, y_dep) sont mis a 1
        les autres sont mis a 0
        NB : on considere un echiquier vide
    */

    int i;

    *bb = 0;
    for (i = 0; i < 8; i++) *bb |= ((Int) 1 << INDEX(i, x_dep)) | ((Int) 1 << INDEX(y_dep, i));

    *bb ^= ((Int) 1 << INDEX(y_dep, x_dep));

    //fprintf(stderr, "tour en x = %d, y = %d : %llu\n", x_dep, y_dep, *bb);
    //print_bb(*bb);
}

void peut_aller_fou_bb(int x_dep, int y_dep, Int* bb)
{
    int i, j;

    *bb = 0;
    for (i = y_dep - MIN(x_dep, y_dep), j = x_dep - MIN(x_dep, y_dep); i < 8 && j < 8; i++, j++) *bb |= (Int) 1 << INDEX(i, j);
    for (i = y_dep + MIN(x_dep, 7 - y_dep), j = x_dep - MIN(x_dep, 7 - y_dep); i >= 0 && j < 8; i--, j++) *bb |= (Int) 1 << INDEX(i, j);

    *bb ^= ((Int) 1 << INDEX(y_dep, x_dep));
}

void peut_aller_cavalier_bb(int x_dep, int y_dep, Int* bb)
{
    int i, j = MIN(2, 7 - x_dep);

    *bb = 0;

    for (i = MAX(-2, -x_dep); i <= j; i++)
    {
        if (!i) continue;
        if (y_dep - (3 - abs(i)) >= 0) *bb |= (Int) 1 << INDEX(y_dep - (3 - abs(i)), x_dep + i);
        if (y_dep + (3 - abs(i)) < 8)  *bb |= (Int) 1 << INDEX(y_dep + (3 - abs(i)), x_dep + i);
    }

}

void init_l_cases_cavalier(int x, int y)
/*
    ecrit dans L_CASES_CAVALIER toutes cases qu'un cavalier place en (x, y) peut atteindre en 1 coup
*/
{
    int i, j, id = INDEX(y, x);

    L_CASES_CAVALIER[id].liste = malloc(sizeof(Coordonnees) * 8);

    for (i = 0, j = 0; i < 64; i++)
    {
        if (MOUV_CAVALIER[id] & ((Int) 1 << i))
        {
            L_CASES_CAVALIER[id].liste[j].x = (i & 7);
            L_CASES_CAVALIER[id].liste[j++].y = (i >> 3);
        }
    }

    L_CASES_CAVALIER[id].taille = j;

}

void init_l_cases_fou_tour(int x, int y)
{
    int i, j, k, l, dx, dy, id = INDEX(y, x);

    //il y a au plus 7 coups possibles sur une meme diagonale pour un fou
    //et un fou a 4 diagonales "relatives" ; idem pour la tour
    for (i = 0; i < 4; i++)
    {
        L_CASES_FOU[id].liste[i] = malloc(sizeof(Coordonnees) * 7);
        L_CASES_TOUR[id].liste[i] = malloc(sizeof(Coordonnees) * 7);
    }

    for (dy = -1, l = 0; dy < 2; dy += 2) for (dx = -1; dx < 2; dx += 2, l++)
    {
        for (i = y + dy, j = x + dx, k = 0; i >= 0 && i < 8 && j >= 0 && j < 8; i += dy, j += dx)
        {
            L_CASES_FOU[id].liste[l][k].x = j;
            L_CASES_FOU[id].liste[l][k++].y = i;
        }
        L_CASES_FOU[id].taille[l] = k;
    }

    for (dy = -1, l = 0; dy < 2; dy++) for (dx = -1; dx < 2; dx++)
    {
        if (!((dy != 0) ^ (dx != 0))) continue;
        for (i = y + dy, j = x + dx, k = 0; i >= 0 && i < 8 && j >= 0 && j < 8; i += dy, j += dx)
        {
            L_CASES_TOUR[id].liste[l][k].x = j;
            L_CASES_TOUR[id].liste[l][k++].y = i;
        }
        L_CASES_TOUR[id].taille[l++] = k;
    }
}

void init_l_cases_roi(int x, int y)
{
    int i, j, n = 0, id = INDEX(y, x);

    L_CASES_ROI[id].liste = malloc(sizeof(Coordonnees) * 8);

    for (i = -1; i <= 1; i++) for (j = -1; j <= 1; j++)
    {
        if ( ((!i) && (!j)) || y + i < 0 || x + j < 0 || y + i >= 8 || x + j >= 8) continue;

        L_CASES_ROI[id].liste[n].x = j + x;
        L_CASES_ROI[id].liste[n++].y = i + y;
    }

    L_CASES_ROI[id].taille = n;
}

void print_l_cases(L_cases* l)
{
    int i;
    fprintf(stderr, "format (y, x) : [");
    for (i = 0; i < l->taille; i++) fprintf(stderr, "(%d %d) ", l->liste[i].y, l->liste[i].x);
    fprintf(stderr, "]\n");
}

void print_l_cases_x4(L_cases_x4* l)
{
    int i, j;
    fprintf(stderr, "format (y, x) : \n\t");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < l->taille[i]; j++) fprintf(stderr, "(%d %d) ", l->liste[i][j].y, l->liste[i][j].x);
        fprintf(stderr, "\n");
        if (i != 3) fprintf(stderr, "\t");
    }
}

void init_entre()
{
    /*
        initialise ENTRE_LIGNE et ENTRE_DIAGONALE
        ENTRE_LIGNE[case1][case2] doit etre le masque des cases situees entre case1 et case2
        par exemple pour "entre[(0, 0)][(0, 4)]", on a le masque 0111000...

        idem pour les diagonales

        bien entendu, entre_truc[c1][c2] == entre_truc[c2][c1]
        mais l'idee est de faire rapide, pas d'economiser quelques octets en memoire

        NB : en tout, cette fonction a besoin en memoire de 2 * (64 * sizeof(Int*) + 64**2 * 8) = 66 ko
    */
    int i, j, k, dx, dy;

    ENTRE_LIGNE = malloc(sizeof(Int*) * NB_CASES_ECHIQUIER);
    ENTRE_DIAGONALE = malloc(sizeof(Int*) * NB_CASES_ECHIQUIER);

    for (i = 0; i < NB_CASES_ECHIQUIER; i++)
    {
        ENTRE_LIGNE[i] = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);
        ENTRE_DIAGONALE[i] = malloc(sizeof(Int) * NB_CASES_ECHIQUIER);

        for (j = 0; j < NB_CASES_ECHIQUIER; j++)
        {
            ENTRE_DIAGONALE[i][j] = 0;
            ENTRE_LIGNE[i][j] = 0;

            if (i == j) continue;

            if ((i >> 3) == (j >> 3) || (i & 7) == (j & 7))
            {
                dx = (j & 7) - (i & 7);
                if (dx) dx /= abs(dx);
                dy = ((j >> 3) - (i >> 3));
                if (dy) dy = (dy / abs(dy)) << 3;

                for (k = i + dx + dy; k != j; k += dx + dy) ENTRE_LIGNE[i][j] |= (Int) 1 << k;

            }

            else if (abs((i >> 3) - (j >> 3)) == abs((i & 7) - (j & 7)))
            {
                dx = ((j & 7) > (i & 7) ? 1 : -1);
                dy = ((j >> 3) > (i >> 3) ? 8 : -8);

                for (k = i + dx + dy; k != j; k += dx + dy) ENTRE_DIAGONALE[i][j] |= (Int) 1 << k;

                //fprintf(stderr, "entre y = %d, x = %d et y2 = %d et x2 = %d : ", i >> 3, i & 7, j >> 3, j & 7);
                //print_bb(ENTRE_DIAGONALE[i][j]);
            }




        }
    }
}


char bb_scan_droite(Int nb)
{
    /*
        renvoie l'index du bit le plus a droite (ie de poids le plus faible) de nb
        par exemple, pour nb = 00..000101000100000, bb_scan_droite renvoie 5

        voir le programme python de_bruijn.py pour la generation de la table et de la clef

        une autre possibilite serait d'utiliser l'instruction asm BSF, mais pour ca il faut etre en 64 bits obligatoirement
    */

    return INDEX_BIT_SCAN[((nb & -nb) * 151050438420815295) >> 58];


}
