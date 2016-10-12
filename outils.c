#include <stdlib.h>
#include <time.h>
#include "outils.h"

float* MEMOIRE_TRI;
Uint8* MEMOIRE_TRI_2;

Int ALEATOIRE;


char init_outils()
{
    MEMOIRE_TRI = malloc(sizeof(float) * (NB_MAX_COUPS_POSSIBlES + 1));
    MEMOIRE_TRI_2 = malloc(sizeof(Uint8) * (NB_MAX_COUPS_POSSIBlES + 1));

    ALEATOIRE = 1000 * clock();

    return (MEMOIRE_TRI != NULL && MEMOIRE_TRI_2 != NULL);
}

void free_outils()
{
    free(MEMOIRE_TRI);
    free(MEMOIRE_TRI_2);
}

void maj_aleat()
{
    Int big_premier = (Int) BIG_PREMIER;
    ALEATOIRE = (ALEATOIRE * ALEATOIRE + 1) % big_premier;
}

float nb_aleat(float b_inf, float b_sup)
{
    /*
        renvoie un nombre aleatoire compris entre 'b_inf' et 'b_sup'
    */

    Int big_premier = (Int) BIG_PREMIER;
    maj_aleat();
    return b_inf + (b_sup - b_inf) * (((float) ALEATOIRE) / big_premier);
}

char bit_aleat()
{
    /*les bonnes proprietes statistiques de cette fonction ont ete verifiees*/
    maj_aleat();
    return ALEATOIRE & 1;
}


char allouer_noeud(Noeud* a_allouer, int taille)
{
    int i;

    a_allouer->liste_coups = malloc(sizeof(Uint8) * (taille + 1));
    a_allouer->liste_fils = malloc(sizeof(Noeud) * taille);

    if (a_allouer->liste_coups == NULL || a_allouer->liste_fils == NULL) return FAILURE;

    for (i = 0; i < taille; i++) {
        a_allouer->liste_fils[i].profondeur = -1;
        a_allouer->liste_coups[i] = i;
    }

    a_allouer->liste_coups[taille] = FIN_LISTE_COUPS;
    a_allouer->profondeur = 0;

    return SUCCESS;

}


void liberer_arbre(Noeud* pere)
{
    int i;

    if (pere->profondeur < 0) return;

    //les deux conditions des for devraient etre equivalentes grace a la ligne ci dessus...
    for (i = 0; pere->liste_coups[i] != FIN_LISTE_COUPS; i++) {//for (i = 0; i < pere->profondeur; i++) {
        liberer_arbre(pere->liste_fils + i); //pere->liste_coups[i] != FIN_LISTE_COUPS &&
    }
    if (pere->profondeur >= 0) {
        free(pere->liste_coups);
        free(pere->liste_fils);
    }

    pere->profondeur = -1;

}


void tri_fusion(float* tab, int taille, Uint8* a_modif)
{
    /*
        trie tab par ordre decroissant
        les elements de a_modif sont modifies en parallele, mais n'ont aucune influence sur le tri
    */
    int a = 0, b = 0, i = 0, dim, dim2;
    Uint8 *tab_bis2;
    float *tab_bis;
    if (taille < 2) return;

    dim = taille >> 1; dim2 = taille - dim;
    tab_bis = tab + dim;
    tab_bis2 = a_modif + dim;
    tri_fusion(tab, dim, a_modif);
    tri_fusion(tab + dim, dim2, tab_bis2);

    while (a < dim && b < dim2)
    {
        if (tab[a] < tab_bis[b])
        {
            MEMOIRE_TRI[i] = tab_bis[b];
            MEMOIRE_TRI_2[i++] = tab_bis2[b++];
        }
        else
        {
            MEMOIRE_TRI_2[i] = a_modif[a];
            MEMOIRE_TRI[i++] = tab[a++];
        }
    }

    while (a < dim)
    {
        MEMOIRE_TRI_2[i] = a_modif[a];
        MEMOIRE_TRI[i++] = tab[a++];
    }
    while (b < dim2)
    {
        MEMOIRE_TRI_2[i] = tab_bis2[b];
        MEMOIRE_TRI[i++] = tab_bis[b++];
    }
    for (i = 0; i < taille; i++)
    {
        a_modif[i] = MEMOIRE_TRI_2[i];
        tab[i] = MEMOIRE_TRI[i];
    }
}

