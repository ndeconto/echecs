#ifndef STRUCTURES_JEU
#define STRUCTURES_JEU

#include "bitsboards.h"

struct Coup_IA
{
    Coup coup;
    char id_couleur;
    float score;
};

typedef struct Coup_IA Coup_IA;

typedef unsigned char Uint8;
typedef struct Noeud Noeud;
struct Noeud
{
    /*
        liste_coups contient les index des coups renvoyes par liste_coups_valides
        faire en sorte que ces indices soient tries pour que l'elagage soit efficace
        un couple liste_coups[i], liste_fils[i] represente ainsi une branche

        une profondeur de n signifie que les n premiers fils du noeud existent et sont alloues ; les suivants ne le sont pas
        une profondeur de 0 signifie que les fils du noeud ne sont pas alloués
        une profondeur de -1 signifie que liste_coups et liste_fils ne sont pas des pointeurs valides
        la profondeur est valable pour ... ??
    */
    short int profondeur;
    Uint8* liste_coups;
    Noeud* liste_fils;

    //int exhaustive;
};


struct Position
{
    Int v_hash;
    float score;
    unsigned char profondeur_analyse;
    short id_meilleur_coup;
};
typedef struct Position Position;


typedef struct Liste_pos Liste_pos;
struct Liste_pos
{
    Position pos;
    Liste_pos* suivant;
};

struct Compteur_pos
{
    /*
    permet de compter le nombre de fois ou une position a ete atteinte
    */
    Int hah_pos;
    unsigned char nb_fois_atteinte;
};


#endif // STRUCTURES_JEU
