#ifndef HASH_HEAD
#define HASH_HEAD

#include <assert.h>

#include "structures_jeu.h"
#include "IA.h"

char init_hash();
void free_hash();
int index_hash(char, int, char);
Int hash_coup(const Coup*, const Echiquier*);
char is_in_hashtable(Int, unsigned char, float*);
void ajouter_hash(Int, float, unsigned char);
void maj_hash_pos(Int*, const Coup*, const Echiquier*);
Int hash_aleat();


#endif
