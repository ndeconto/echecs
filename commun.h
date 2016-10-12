#ifndef COMMUN
#define COMMUN

/*
    tous les fichiers (interface graphique et moteur de jeu) ont besoin de ce header
    il contient toutes les constantes et maccros de base indispensables a toute fonction
*/

#define SUCCESS     1
#define FAILURE     0


#define NB_PIECES   6


//eviter d'y toucher, sinon heuristique de IA.c ne va pas aimer
#define VIDE        0
#define PION        1
#define CAVALIER    2
#define FOU         3
#define TOUR        4
#define DAME        5
#define ROI         6

#define BLANC       1
#define NOIR        -1

#define COTE_BLANC  7 //ie les pions blancs commencent en y = 6
#define COTE_NOIR   0 //les pions nooirs commencent en y = 1

#define NB_CASES_ECHIQUIER  64
#define NB_MAX_COUPS_POSSIBlES 218


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
    si l'echiquier est represente par un tableau 8x8, alors :
    echiquier[0][0] represente a8, echiquier[0][1] represente b8, ...
    echiquier[1][0] -> a7, echiquier[1][1] -> b7, ...
*/


#endif // COMMUN
