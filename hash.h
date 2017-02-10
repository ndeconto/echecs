#ifndef HASH_HEAD
#define HASH_HEAD

#include <assert.h>

#include "structures_jeu.h"
#include "IA.h"


#define NB_EXTRA_BITS       1       //cf commentaire sur les hashbits plus bas
#define TAILLE_TABLE_HASH   4194304 //2**22, TAILLE_TABLE_HASH = 2**NB_BITS_TABLE_HASH
#define NB_BITS_TABLE_HASH  22      // ATTENTION : modifier les deux lignes en meme temps !!
#define POSITIONS_PAR_RESTE 4


#define HASHBIT_TRAIT               63
/*NB : les extrabits de roque et de prise en passant (PEP) ne sont pas pris en
compte, pour diminuer le risque de collision. Car on considère que confondre 2
positions qui diffèrent seulement de part les droits de roque ou le droit de PEP
n'est pas trop grave
*/
#define HASHBIT_ROQUE_DAME_NOIR     62
#define HASHBIT_ROQUE_ROI_NOIR      61
#define HASHBIT_ROQUE_DAME_BLANC    60
#define HASHBIT_ROQUE_ROI_BLANC     59
#define HASH_FIRST_BIT_PEP          56  //prise en passant
#define NB_HASHBIT_PEP              3

char init_hash();
void free_hash();
int index_hash(char, int, char);
Int hash_coup(const Coup*, const Echiquier*);
char is_in_hashtable(Int, unsigned char, float*, short*);
void ajouter_hash(Int, float, unsigned char, short);
Int maj_hash_pos(Int, const Coup*, const Echiquier*);
Int hash_aleat();


#endif
