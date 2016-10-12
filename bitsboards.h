#ifndef BITSBOARDS
#define BITSBOARDS

//index des diagonales (relatives) des fous
#define HAUT_GAUCHE     0
#define HAUT_DROITE     1
#define BAS_GAUCHE      2
#define BAS_DROITE      3



#define INDEX_BLANC     0
#define INDEX_NOIR      1
#define INDEX_AILE_DAME 0
#define INDEX_AILE_ROI  1

#include "commun.h"

typedef unsigned long long int Int;

struct Coordonnees
{
    unsigned char x;
    unsigned char y;
};
typedef struct Coordonnees Coordonnees;



struct L_cases
{
    unsigned char taille;
    Coordonnees* liste;
};
typedef struct L_cases L_cases;


struct L_cases_x4
{
    unsigned char taille[4];
    Coordonnees *liste[4];
};
typedef struct L_cases_x4 L_cases_x4;


struct Coup
{
    Coordonnees case_dep;
    Coordonnees case_fin;
};
typedef struct Coup Coup;

#define MEME_COUP(a, b) ((a).case_dep.x == (b).case_dep.x && (a).case_dep.y == (b).case_dep.y && (a).case_fin.x == (b).case_fin.x && (a).case_fin.y == (b).case_fin.y)

struct Echiquier
{
    char plateau[8][8];             //et non pas char** plateau, sinon echiquier_2 = echiquier_1, et c'est le drame
                                    // plateau[0][0] represente a8, plateau[7][7] h7, etc...

    Int pieces_couleur[2];          //bitsboard contenant des 1 la ou une piece est posee
                                    //l'echiquier total s'obtient par un simple |
    Coordonnees pos_rois[2];

    char roi_a_bouge[2];
    char tour_a_bouge[2][2];           //indexer [couleur][cote] ; par la tour blanche de l'aile dame est tour_a_bouge[INDEX_BLANC][INDEX_AILE_DAME]

    Coup dernier_coup;

    Int bb_par_piece[2][NB_PIECES];//par ex, bb_par_piece[INDEX_COULEUR][FOU] contient des 1 la ou il y a des fous "COULEUR"
                                    // les seules cases valides sont pour CAVALIER FOU et TOUR ; DAME etant inclue dans TOUR et FOU



};
typedef struct Echiquier Echiquier;



/*
    dans un bitsboard, le bit de poids le plus faible represente la case (x = 0, y = 0) du tableau 2D corresondant
    le bit de poids le plus fort represente (x = 7, y = 7)
    plus generalement, la case (x, y) est representee par le bit (8 * y + x) en partant de la droite quand on ecrit le bitsboard sous forme de nombre

    les MOUV_PIECE sont des simples tableaux de bitboards et non pas des tableaux 2D pour des raisons memoire
    par exemple, MOUV_TOUR[(y << 3) + x] contient des 1 pour toutes les cases ou la tour peut aller depuis la case (x, y)
    raisonner de la meme maniere, mais indexer avec la macro INDEX (cf un peu plus bas) :
*/

extern Int* MOUV_TOUR;
extern Int* MOUV_CAVALIER;
extern Int* MOUV_FOU;
extern Int* MOUV_DAME;
extern Int* MOUV_ROI;
extern Int* MOUV_PION;

extern Int** ENTRE_LIGNE;
extern Int** ENTRE_DIAGONALE;

void init_bitsboards(Echiquier*);
void free_bitsboards();

//NB : bb -> bitsboards
void print_bb(Int);
void peut_aller_tour_bb(int, int, Int*);
void peut_aller_fou_bb(int, int, Int*);
void peut_aller_cavalier_bb(int, int, Int*);

char bb_scan_droite(Int);

void init_entre();


/*
    memoriser toutes les cases ou une piece peut aller depuis chaque case de depart possible
    par exemple, pour une tour en (x, y), avoir 4 listes (cf structure L_cases_x4):
    mouvements_vers_le_haut, vers_le_bas, la gauche et la droite.
    cela permet de lister ses coups pseudo-possibles tres rapidement
    il suffit en effet de parcourir chaque liste, et de breaker des qu'on rencontre une case occupee
    la fin de la chaine etant connue

    idem pour les fous

    Les cavaliers n'ont besoin que d'une liste simple, d'ou le type L_cases

*/

extern L_cases* L_CASES_CAVALIER;
extern L_cases_x4* L_CASES_TOUR;
extern L_cases_x4* L_CASES_FOU;
extern L_cases* L_CASES_ROI;


void init_l_cases_cavalier(int, int);
void init_l_cases_fou_tour(int, int);
void init_l_cases_roi(int, int);

void print_l_cases(L_cases*);
void print_l_cases_x4(L_cases_x4*);


#define INDEX(y, x) (((y) << 3) + (x))

#endif // BITSBOARDS
