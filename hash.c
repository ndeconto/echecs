#include <stdlib.h>
#include "hash.h"


Int *HASH_ELEMENTAIRES;
Liste_pos** TABLE_HASH; //TODO stocker aussi le meilleur coup dans la table pour pouvoir optimiser alpha beta en cas de recherche approfondie plus tard


char init_hash(void)
{
    /*
        genere les hashs de tous les types d'action possibles (ce sont les hashs elementaires) :

        3 bits pour le type de piece concerne ; card (tour, cavalier, fou, dame, roi, pion) = 6
        6 bits pour indiquer la case concernee
        1 bit pour indiquer s'il s'agit d'un ajout (ie on pose la piece -> bit = 1) ou d'une suppression (ie on enleve la piece -> bit = 0)
        1 bit de couleur (bit = INDEX_COULEUR)

        => 2**11 hashs elementaires possibles

        chaque hash elementaire est un entier de 64 bits de la forme :
            NB_EXTRA_BITS bits de poids forts tous a 0
            54 bits restants choisis aleatoirement

        ========================================================================================================

        le hash d'une position fait 64 bits
        les NB_EXTRA_BITS bits de poids forts sont "speciaux" : (par poids decroissant):
            1 bit de trait : INDEX_BLANC si les blancs ont le trait, INDEX_NOIR sinon
            4 bits indiquant respectivement si chaque roque est possible (ie, si les tours ou rois ont bouge)
            3 bits indiquant quel est l'eventuel pion pouvant etre pris "en passant"

        les autres bits sont mis a jour (maj) par des xor avec des hashs elementaires
    */


    int nb_hashs_possibles = 6 * (1 << 8), i;
    HASH_ELEMENTAIRES = malloc(sizeof(Int) * nb_hashs_possibles);
    TABLE_HASH = malloc(sizeof(Liste_pos*) * TAILLE_TABLE_HASH);

    if (HASH_ELEMENTAIRES == NULL || TABLE_HASH == NULL) return FAILURE;

    for (i = 0; i < TAILLE_TABLE_HASH; i++) TABLE_HASH[i] = NULL;

    for (i = 0; i < nb_hashs_possibles; i++)
    {
        HASH_ELEMENTAIRES[i] = hash_aleat();
    }

    return SUCCESS;

}

int index_hash(char type_piece, int index_case, char ajout)
{
    /*
     type piece doit etre strictement negatif s'il s'agit d'une piece noire
     strictement positif sinon
     ajout = 1 si on pose une piece
     ajout = 0 si on enleve une piece

     0 <= index_case < 64; utiliser la maccro INDEX pour l'obtenir a partir des coordonnees

     cf init_hash pour la description de la forme de l'index
     */

    // 3 bits de type, mais ABS(type_piece) - 1 < 6 d'apres commun.h
    int retour = ABS(type_piece) - 1;
    retour <<= 6;

    // 6 bits de position
    retour ^= (Int) index_case;
    retour <<= 1;

    //bit d'ajout
    retour ^= (Int) ajout & 1;
    retour <<= 1;

    //bit de couleur
    retour ^= ((Int) GET_COULEUR(type_piece) & 1);

    return retour;


}

Int hash_coup(const Coup* coup, const Echiquier* ech)
{
    /*
        renvoie le hashage du coup
        la fonction doit etre appelee AVANT de jouer le coup !!!

        par exemple, pour coup = Cb1 - c3, la fonction renvoie
        hash(enlever C en b1) ^ hash(placer C en c3)

        la fonction a besoin de l'echiquier pour savoir s'il s'agit d'une prise, et quel type de piece est en train de bouger

        il y a un probleme pour les extrabits... quand sont-ils geres ?
        pour les modifier par xor, il faut connaitre la position precedente
        sinon il faut que ces bits speciaux soient egalement modifiables par & et |...
        seul le bit de trait ne pose pas de probleme
        meilleur_coup peut aussi s'en occupper...
        a mediter

    */

    //semble renvoyer des resultats incorrects...

    int x1 = coup->case_dep.x, x2 = coup->case_fin.x, y1 = coup->case_dep.y, y2 = coup->case_fin.y;
    char type_piece = ech->plateau[y1][x1];
    int b = index_hash(type_piece, INDEX(y1, x1), 0);
    Int h = HASH_ELEMENTAIRES[index_hash(type_piece, INDEX(y1, x1), 0)];

    assert(h < 72057594037927936);

    int id = INDEX(y2, x2);
    h ^= HASH_ELEMENTAIRES[index_hash(type_piece, id, 1)];

    if (ech->plateau[y2][x2]) h ^= HASH_ELEMENTAIRES[index_hash(ech->plateau[y2][x2], id, 0)];

    return h;


}

void free_hash()
{
    int i;
    Liste_pos* act, *old;
    free(HASH_ELEMENTAIRES);

    for (i = 0; i < TAILLE_TABLE_HASH; i++)
    {
        act = TABLE_HASH[i];
        while (act != NULL)
        {
            old = act;
            act = old->suivant;
            free(old);
        }
    }
    free(TABLE_HASH);
}


char is_in_hashtable(Int hash_pos, unsigned char p_min, float* score)
{
    /*
    renvoie 1 si la position de hash 'hash_pos' est trouvee dans la table de hashage avec une profondeur d'analyse superieure ou egale a 'p_min'
    *score vaut alors le score de cette position

    la fonction renvoie 0 sinon.

    */

    Int reste = hash_pos & (((Int) 1 << NB_BITS_TABLE_HASH) - 1);
    Liste_pos* act = TABLE_HASH[reste];

    while (act != NULL)
    {
        if (act->pos.v_hash == hash_pos)
        {
            if (act->pos.profondeur_analyse >= p_min)
            {
                *score = act->pos.score;
                return 1;
            }

            return 0;
        }

        act = act->suivant;
    }

    return 0;
}

void ajouter_hash(Int hash_pos, float score, unsigned char p)
{
    /*
        ajoute hash_pos dans la table de hash ; 'p' est la profondeur a laquelle la position a ete analysee
        supprime eventuellement un vieux hash dans la table ou remplace une analyse moins profonde
        les positions les plus recentes sont au debut, les plus anciennes a la fin de la liste.
        lorsqu'une position est modifiee en augmentant la profondeur d'analyse, l'element n'est pas replace en debut de liste

        si une analyse plus profonde est deja presente, la fonction ne fait rien
    */

   Int reste = hash_pos & (((Int) 1 << NB_BITS_TABLE_HASH) - 1);
   Liste_pos* act = TABLE_HASH[reste], *old;
   int i;

   for (i = 0; i < POSITIONS_PAR_RESTE; i++)
   {
        if (act == NULL || act->pos.v_hash == hash_pos)
        {
            if (act == NULL) act = malloc(sizeof(Liste_pos));
            else if (act->pos.profondeur_analyse >= p) return;

            act->pos.score   = score;
            act->pos.profondeur_analyse = p;

            if (act->pos.v_hash != hash_pos){
                act->pos.v_hash  = hash_pos;
                act->suivant     = TABLE_HASH[reste];
                TABLE_HASH[reste]= act;
            }
            return;
        }

        if (i != POSITIONS_PAR_RESTE - 1){
            old = act;
            act = act->suivant;
        }
   }

   free(act);
   old->suivant = NULL;
   act = malloc(sizeof(Liste_pos));
   act->pos.score = score;
   act->pos.profondeur_analyse = p;
   act->pos.v_hash = hash_pos;
   act->suivant = TABLE_HASH[reste];
   TABLE_HASH[reste] = act;

}

void maj_hash_pos(Int* hash_pos, const Coup* coup, const Echiquier* ech)
{
    //TODO il faut gerer les extras bits !!!!!
    *hash_pos = *hash_pos ^ hash_coup(coup, ech);
}


Int hash_aleat()
{
    int j;
    Int x = 0;

    for (j = 0; j < 64 - NB_EXTRA_BITS; j++) {
        x ^= ((Int) bit_aleat() << j);
    }

    return x;
}
