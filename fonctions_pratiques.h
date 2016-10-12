#ifndef FONCTIONS_PRATIQUES
#define FONCTIONS_PRATIQUES


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>


#define NB_MOTS 2000            //nombre maximum de mots dans un menu, une boite de dialogue, etc..
#define M_MAX 200               //longueur maximale d'un mot

#define TAILLE_BOX              500
#define COEFF_ECRITURE          .8
#define TAILLE_TEXTE_DEFAUT     20
#define SAUT_DE_LIGNE           20
#define ESPACE_TAB_MAX          70
#define POLICE_PAR_DEFAUT       "Monaco.ttf"

#define COLLE_A_DROITE            1
#define COLLE_A_GAUCHE          2
#define COLLE_EN_BAS            3
#define COLLE_EN_HAUT           4

#define TRANSPARENCE_PAR_DEFAUT 80
#define NB_MAX_BOUTONS          32
#define FLAG_OK                 0x00000001
#define BOUTON_OUI              1
#define FLAG_NO                 0x00000002
#define BOUTON_NON              2
#define BOUTON_FERMETURE        3

Uint32 getpixel(const SDL_Surface*, int, int);
void putpixel(SDL_Surface*, int, int, Uint32);
void blitRGBA(const SDL_Surface*, const SDL_Rect*, SDL_Surface*, SDL_Rect*);
char is_in_rect(int, int, const SDL_Rect*);

void draw_menu(SDL_Surface*, TTF_Font*, char choix[NB_MOTS][M_MAX], int, SDL_Color, SDL_Rect*);
int in_rect(SDL_Rect*, int, int);
void afficher_bouton(SDL_Surface*, char*, SDL_Rect*, TTF_Font*, SDL_Color);
int menu(SDL_Surface*, char choix[NB_MOTS][M_MAX], int, int, SDL_Color, SDL_Color);

SDL_Surface* masquer_fond(unsigned char);
SDL_Surface* render_auto_intelligent(TTF_Font*, const char*, SDL_Color, int);
void fusionner_surface(SDL_Surface**, const SDL_Surface*, int, char);
int message(const char*, int, const char*, Uint8, Uint32);

char existe_fichier(const char*);
void trouver_nouveau_nom(char*);

int taille_fichier(const char*);
char print_from_file(const char*, SDL_Color, TTF_Font*, int, int, int);

#define MIN(a, b)((a) < (b) ? (a) : (b))
#define MAX(a, b)((a) > (b) ? (a) : (b))

#endif // FONCTIONS_PRATIQUES
