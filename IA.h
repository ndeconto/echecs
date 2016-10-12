#ifndef IA
#define IA

#include "bitsboards.h"
#include "lister_coups.h"
#include "jeu_de_base.h"

#define INFINI              9999
#define SCORE_MAT           1000
#define LONGUEUR_MAX_MAT    100
#define MAXI_HEURISTIQUE    10
#define T_MAX_COUPS         100

#define NB_MAX_ATTAQUANTS   10

#define PARTIE_FINIE        127
#define FIN_LISTE_COUPS     255
#define PROFONDEUR_MAX_ARBRE 12


#define NB_EXTRA_BITS       8
#define TAILLE_TABLE_HASH   1048576 //2**20, TAILLE_TABLE_HASH = 2**NB_BITS_TABLE_HASH
#define NB_BITS_TABLE_HASH  20
#define POSITIONS_PAR_RESTE 4

#define MATOS_EN_FINALE     42

#define GET_COULEUR(a)((a) > 0 ? INDEX_BLANC : INDEX_NOIR)


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


int  init_ia();
void free_ia();

void maj_aleat();
float nb_aleat(float, float);
char bit_aleat();

char allouer_noeud(Noeud*, int);
void liberer_arbre(Noeud*);
void tri_fusion(float*, int, Uint8*);

Coup_IA meilleur_coup(const Echiquier*, int, char, float, float);
Coup_IA meilleur_coup_avec_memoire(const Echiquier*, int, Noeud*, char, Int, float, float);
Coup_IA meilleur_coup_profondeur_auto(const Echiquier*, char, float, Int);
float   heuristique(const Echiquier*, char);
char    gain_perte(const char*, int, const char*, int, char);


char init_hash();
void free_hash();
int index_hash(char, int, char);
Int hash_coup(const Coup*, const Echiquier*);
char is_in_hashtable(Int, unsigned char, float*);
void ajouter_hash(Int, float, unsigned char);
void maj_hash_pos(Int*, const Coup*, const Echiquier*);


#define ABS(a) ((a) > 0 ? (a) : -(a))

#endif // IA
