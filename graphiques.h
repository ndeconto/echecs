#ifndef GRAPHIQUES
#define GRAPHIQUES

#include "commun.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>


#define LARGEUR_FENETRE     1000
#define HAUTEUR_FENETRE     750
#define BPP                 32

//le coin en haut a gauche de l'echiquier n'a pas forcement les memes coordonnees que le coin en haut a gauche de de a8
//le decalage eventuel est :
#define OFFSET_PIECES_X     22
#define OFFSET_PIECES_Y     21
#define OFFSET_TEXTE        5

#define POS_ECHIQUIER_X     0
#define POS_ECHIQUIER_Y     0

#define ECART_PROMOTION     0       //ecart en pixel des images lors du choix de la promotion
#define TRANPARENCE_PREMIER_PLAN 180

//les dimensions en pixels d'une case de l'echiquier sont :
#define DIM_CASE_X          84
#define DIM_CASE_Y          84

#define BLANCS_EN_BAS       1
#define NOIRS_EN_BAS        -1

#define T_POLICE_ECHIQUIER  25
#define T_POLICE_PNG        35


#define QUITTER             0
#define JOUER_COUP          1
#define RAFRAICHIR_ECHIQUIER 2
#define ACTIVER_IA          3
#define DESACTIVER_IA       4
#define JEU_LIBRE           5
#define JEU_ALTERNE         6
#define AFFICHER_COMMANDES  7

#define RETOURNER(sens_echiquier, nb) ((sens_echiquier) == BLANCS_EN_BAS ? (nb) : 7 - (nb))



struct Images
{
    SDL_Surface* pieces_blanches[NB_PIECES + 1];
    SDL_Surface* pieces_noires[NB_PIECES + 1];

    SDL_Surface* echiquier_blancs_en_bas;
    SDL_Surface* echiquier_noirs_en_bas;

    TTF_Font* police_png;

};

typedef struct Images Images;

char init_graphiques(void);
void quit_graphiques(void);

void afficher_echiquier(char[8][8], int, int, char);
void afficher_png(const char*);
void afficher_aide();
void afficher_commandes();
void afficher_tout(char[8][8], int, int, char, const char*);

char recuperer_coup(SDL_Event*, int*, int*, int*, int*, char);
int gestion_entrees(char*);
void get_coup_joue(int*, int*, int*, int*);

char choix_promotion();

void pause(void);


#endif // GRAPHIQUES
