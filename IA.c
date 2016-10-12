#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>

#include "IA.h"


//cf heuristique
char VALEUR_PIECES[] = {-100, -10, -5, -3, -3, -1, 0, 1, 3, 3, 5, 10, 100};
Int NB_APPELS_HEURISTIQUES;
int COUPURES_HASH;
float* MEMOIRE_TRI;
Uint8* MEMOIRE_TRI_2;

Int ALEATOIRE;
Int *HASH_ELEMENTAIRES;
Liste_pos** TABLE_HASH; //TODO stocker aussi le meilleur coup dans la table pour pouvoir optimiser alpha beta en cas de recherche approfondie plus tard

float CASE_VALUE[8][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, .1, .1, .1, .1, 0, 0 },
    { 0, 0, .1, .2, .2, .1, 0, 0 },
    { 0, 0, .1, .2, .2, .1, 0, 0 },
    { 0, 0, .1, .1, .1, .1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }};

float BONUS_PION[8][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 2., 2., 2., 2., 2., 2., 2., 2. },
    { .6, .7, .9, 1., 1., .9, .7, .6, },
    { .2, .3, .4, .5, .5, .4, .3, .2 },
    { .1, .2, .3, .5, .5, .3, .2, .1 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, -.1, -.1, .3, .1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }};


float BONUS_CAVALIER[8][8] = {
    { -.5, 0, 0, 0, 0, 0, 0, -.5 },
    { -.1, 0, 0, 0, 0, 0, 0, -.1 },
    { -.1, 0, 0, 0, 0, 0, 0, -.1 },
    { -.1, .2, 0, .8, .8, 0, .2, -.1 },
    { -.1, 0, 0, .5, .5, 0, 0, -.1 },
    { -.1, 0, .3, 0, 0, .3, 0, -.1 },
    { -.1, 0, 0, 0, 0, 0, 0, -.1 },
    { -.5, -.2, -.2, -.2, -.2, -.2, -.2, -.5 }};



int init_ia()
{
    MEMOIRE_TRI = malloc(sizeof(float) * (NB_MAX_COUPS_POSSIBlES + 1));
    MEMOIRE_TRI_2 = malloc(sizeof(Uint8) * (NB_MAX_COUPS_POSSIBlES + 1));

    ALEATOIRE = 1000 * clock();

    init_hash();

    return (MEMOIRE_TRI != NULL && MEMOIRE_TRI_2 != NULL);
}

void free_ia()
{
    free(MEMOIRE_TRI);
    free(MEMOIRE_TRI_2);
    free_hash();
}


void maj_aleat()
{
    Int big_premier = 16108604210150446087;
    ALEATOIRE = (ALEATOIRE * ALEATOIRE + 1) % big_premier;
}

float nb_aleat(float b_inf, float b_sup)
{
    /*
        renvoie un nombre aleatoire compris entre 'b_inf' et 'b_sup'
    */

    Int big_premier = 16108604210150446087;
    maj_aleat();
    return b_inf + (b_sup - b_inf) * (((float) ALEATOIRE) / big_premier);
}

char bit_aleat()
{
    /*les bonnes proprietes statistiques de cette fonction ont ete verifiees*/
    maj_aleat();
    return ALEATOIRE & 1;
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

Coup_IA meilleur_coup(const Echiquier* ech, int profondeur, char id_couleur, float best_blanc, float best_noir)
{
    Coup* liste = malloc(sizeof(Coup) * T_MAX_COUPS);
    int i, T;
    Coup_IA best, essai;
    Echiquier ech2;

    best.score = -INFINI;
    best.id_couleur = id_couleur;

    T = liste_coups_valides(ech, id_couleur, liste);

    if (T == PAT || T == MAT)
    {
        //if (est_attaque(ech, !id_couleur, INDEX(ech->pos_rois[id_couleur].y, ech->pos_rois[id_couleur].x), NULL))      //MAT
        //{
            if (T == MAT) best.score = -SCORE_MAT;
        //}
        else best.score = 0;                                                            //PAT

        best.coup.case_dep.x = PARTIE_FINIE;
        return best;
    }


    for (i = 0; i < T; i++)
    {

        ech2 = *ech;
        jouer_coup(liste + i, &ech2, id_couleur);

        if (profondeur) {
            //essai = meilleur_coup(ech2, profondeur - 1, !id_couleur, id_couleur == BLANC ? best.score : best_blanc,
            //                                                        id_couleur == NOIR ? best.score : best_noir);
            essai = meilleur_coup(&ech2, profondeur - 1, !id_couleur, best_blanc, best_noir);
            essai.score = -essai.score;

            //pour que les mats les plus courts aient un meilleur score que les mats longs
            if (essai.score >= SCORE_MAT - LONGUEUR_MAX_MAT) essai.score -= 1;
        }
        else {
            essai.score = heuristique(&ech2, !id_couleur);
            if (id_couleur == INDEX_NOIR) essai.score = -essai.score;
        }

        //fprintf(stderr, "%f ", essai.score);


        if (essai.score > best.score) {
            best.coup = liste[i];
            best.score = essai.score;

            if (id_couleur == INDEX_BLANC && best.score > best_blanc) best_blanc = best.score;
            if (id_couleur == INDEX_NOIR && best.score > best_noir) best_noir = best.score;

            if ((id_couleur == INDEX_BLANC && -best.score <= best_noir)
                || (id_couleur == INDEX_NOIR && -best.score <= best_blanc)) break;
        }



    }




    free(liste);

    return best;
}


Coup_IA meilleur_coup_avec_memoire(const Echiquier* ech,  int profondeur, Noeud* arbre, char id_couleur, Int hash_pos, float best_blanc, float best_noir)
{
    Coup* liste = malloc(sizeof(Coup) * T_MAX_COUPS);
    int i, j, T;
    float* l_score = malloc(sizeof(float) * T_MAX_COUPS), *score_connu;
    Noeud copie, *copie_liste_fils;
    Uint8 temp, *data_ref, *copie_coups;
    Coup_IA best, essai;
    Echiquier ech2;
    Int hash_essai, new_hash;

    best.score = -INFINI;
    best.id_couleur = id_couleur;

    T = liste_coups_valides(ech, id_couleur, liste); //memoriser la liste de coups dans l'arbre ?


    if (T == PAT || T == MAT)
    {
        if (T == MAT) best.score = -SCORE_MAT;
        else best.score = 0;                                                            //PAT

        best.coup.case_dep.x = PARTIE_FINIE;
        return best;
    }

    if (arbre->profondeur < 0) {
        if (allouer_noeud(arbre, T) == FAILURE) profondeur = 0; //TODO gerer une explosion de la memoire mieux que ca !
    }

    for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++)
    {

        ech2 = *ech;
        hash_essai = hash_coup(liste + arbre->liste_coups[i], &ech2);
        new_hash = hash_pos ^ hash_essai;

        if (is_in_hashtable(new_hash, profondeur, &(essai.score)) == 0)
        {

            jouer_coup(liste + arbre->liste_coups[i], &ech2, id_couleur);

            if (profondeur) {
                //essai = meilleur_coup_avec_memoire(&ech2, profondeur - 1, arbre->liste_fils + i, i < arbre->profondeur ? NULL : &(arbre->profondeur), !id_couleur, best_blanc, best_noir);
                essai = meilleur_coup_avec_memoire(&ech2, profondeur - 1, arbre->liste_fils + i, !id_couleur, new_hash, best_blanc, best_noir);
                essai.score = -essai.score;

                //pour que les mats les plus courts aient un meilleur score que les mats longs
                if (essai.score >= SCORE_MAT - LONGUEUR_MAX_MAT) essai.score -= 1;
            }
            else {
                essai.score = heuristique(&ech2, !id_couleur);  //TODO tirer parti du fait que tous les appels a heuristique concernent des positions "soeurs" ?
                if (id_couleur == INDEX_NOIR) essai.score = -essai.score;
            }

            ajouter_hash(new_hash, essai.score, profondeur);

        }
        else {
            COUPURES_HASH++;
        }

        l_score[i] = essai.score;


        if (essai.score > best.score) {
            best.coup = liste[arbre->liste_coups[i]];
            best.score = essai.score;

            if (id_couleur == INDEX_BLANC && best.score > best_blanc) best_blanc = best.score;
            if (id_couleur == INDEX_NOIR && best.score > best_noir) best_noir = best.score;

            if ((id_couleur == INDEX_BLANC && -best.score <= best_noir)
                || (id_couleur == INDEX_NOIR && -best.score <= best_blanc)) break;
        }



    }

    //tous les coups coupes sont inferieurs, trier les i premiers elements est suffisant
    /*fprintf(stderr, "profondeur = %d\n", profondeur);
    for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++) fprintf(stderr, "%d ", arbre->liste_coups[i]);
    fprintf(stderr, "\n");
    for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++) fprintf(stderr, "%.3f ", l_score[i]);
    fprintf(stderr, "\n");*/

    data_ref = malloc(sizeof(Uint8) * i);
    copie_coups = malloc(sizeof(Uint8) * i);
    if (profondeur) copie_liste_fils = malloc(sizeof(Noeud) * i);
    for (j = 0; j < i ; j++) {
        data_ref[j] = j;
        copie_coups[j] = arbre->liste_coups[j];
        if (profondeur) copie_liste_fils[j] = arbre->liste_fils[j];
    }

    tri_fusion(l_score, i, data_ref);
    /*for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++) fprintf(stderr, "%d ", data_ref[i]);
    fprintf(stderr, "\n");*/

    for (j = 0; j < i ; j++)
    {
        assert (data_ref[j] < i);
        arbre->liste_coups[j] = copie_coups[data_ref[j]];
        if (profondeur) arbre->liste_fils[j] = copie_liste_fils[data_ref[j]];
    }


    /*for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++) fprintf(stderr, "%d ", arbre->liste_coups[i]);
    fprintf(stderr, "\n"); for (i = 0; arbre->liste_coups[i] != FIN_LISTE_COUPS; i++) fprintf(stderr, "%.3f ", l_score[i]);
    fprintf(stderr, "\n\n");*/

    free(data_ref);
    free(copie_coups);
    if (profondeur) free(copie_liste_fils);
    free(liste);
    free(l_score);

    if (profondeur == 0) liberer_arbre(arbre);  //permet de limiter grandement l'usage de la memoire sans trop de perte de performances

    return best;
}

float heuristique(const Echiquier* ech, char trait)
{
    /*
        renvoie un nombre entre -MAXI_HEURISTIQUE (-10) et +MAXI_HEURISTIQUE (10):
            10 etant la victoire certaine pour les blancs
            -10 la victoire certaine pour les noirs
            0 l'egalite
        MAJ : le retour de la fonction n'est plus borne
            sinon des que le score de 10 est atteint pour l'ia, celle-ci joue n'importe quoi
            qui conserve l'avantage, mais ne cherche pas a gagner encore plus ou a mater

        trait designe le joueur qui va jouer en premier sur cette position

    */

    //la valeur theorique de chaque piece est dans ce tableau
    //les index sont definis dans commun.h
    //pour eviter des indexerrors dues au signe de la piece (selon sa couleur), faire + NB_PIECE a l'index


    NB_APPELS_HEURISTIQUES++;

    int i, j, y;
    char attaquant[NB_MAX_ATTAQUANTS], defenseur[NB_MAX_ATTAQUANTS], t, t2, v, a_v, plus_grosse_perte = 0, temp;
    float score = 0, bonus;

    int materiel_present = 0;

    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++)
    {
        v = ech->plateau[i][j];
        score += VALEUR_PIECES[v + NB_PIECES];
        attaquant[0] = 0;


        if (v && GET_COULEUR(v) != trait)
        {
            t = est_attaque(ech, trait, INDEX(i, j), attaquant);
            if (t)
            {
                t2 = est_attaque(ech, !trait, INDEX(i, j), defenseur);
                temp = gain_perte(attaquant, t, defenseur, t2, ABS(v));
                if (temp > plus_grosse_perte) plus_grosse_perte = temp;
            }


        }


        if (CASE_VALUE[i][j])
        {
            if (v == 0 || GET_COULEUR(v) == trait) t = est_attaque(ech, INDEX_BLANC, INDEX(i, j), attaquant);
            t2 = est_attaque(ech, INDEX_NOIR, INDEX(i, j), defenseur);

            if (t > t2) score += CASE_VALUE[i][j];
            if (t < t2) score -= CASE_VALUE[i][j];
        }

        if (v)
        {
            a_v = abs(v);
            y = (v > 0 ? i : 7 - i);
            bonus = 0;

            if (a_v == PION) bonus = BONUS_PION[y][j];
            if (a_v == CAVALIER) bonus = BONUS_CAVALIER[y][j];
            //TODO accorder des bonus en fonction de la position des pieces

            if (v > 0) score += bonus;
            else score -= bonus;
        }

    }

    if (trait == INDEX_BLANC) score += (float) plus_grosse_perte;
    else score -= (float) plus_grosse_perte;

    //if (score > MAXI_HEURISTIQUE) score = MAXI_HEURISTIQUE;
    //if (score < -MAXI_HEURISTIQUE) score = -MAXI_HEURISTIQUE;

    score += nb_aleat(-.05, .05);
    return score;
}

char gain_perte(const char* l_attaquant, int nb_attaquant, const char* l_defenseur, int nb_defenseur, char occupant)
{
    /*
        renvoie le materiel que peut perdre le joueur qui a la piece occupant
        en fonction des soutiens et des attaquants de la piece en question

        ne verifie pas si les attaquants defenseurs sont valides / cloues

        toutes les valeurs sont en valeur absolue
        l_attaquant et l_defenseur doivent etre tries par ordre croissant de valeur
        (ie les pions en premier, le roi en dernier)
        nb_attaquant et nb_defenseur representent les tailles de l_attaquant et l_defenseur respectivement
        occupant est la valeur de la piece cible de tant de convoitises...
    */

    if (nb_attaquant == 0) return 0;        //pour eviter qu'un appel racine bidon (ie un appel ne provenant pas de cette fonction) fasse tout planter
    if (nb_defenseur == 0)
    {
        if (nb_attaquant) return occupant;
        return 0;
    }

    char a = gain_perte(l_defenseur, nb_defenseur, l_attaquant + 1, nb_attaquant - 1, *l_attaquant);
    if (occupant - a < 0) return 0;
    return occupant - a;

}


Coup_IA meilleur_coup_profondeur_auto(const Echiquier* ech, char id_couleur, float temps_max, Int hash_dep)
{
    /*
        calcule le meilleur coup en determinant automatiquement la profondeur
        pour que le temps de calcul ne depasse pas temps_max

        temps_max doit etre en secondes
    */

    Coup_IA test;
    Noeud arbre, bidon;
    int i;
    float coeff, t, t2;

    NB_APPELS_HEURISTIQUES = 0; COUPURES_HASH = 0;

    t2 = clock();
    arbre.profondeur = -1;//allouer_noeud(&arbre, NB_MAX_COUPS_POSSIBlES);
    test = meilleur_coup_avec_memoire(ech, 0, &arbre, id_couleur, hash_dep, -INFINI, -INFINI);
    //test = meilleur_coup(ech, 0, id_couleur, -INFINI, -INFINI);
    t2 = clock() - t2;

    for (i = 1, coeff = 0; coeff * t2 / CLOCKS_PER_SEC < temps_max; i++)
    {
        t = clock();
        //on peut definir une PROFONDEUR_MAX_ARBRE
        test = meilleur_coup_avec_memoire(ech, i, &arbre, id_couleur, hash_dep, -INFINI, -INFINI);
        //test = meilleur_coup(ech, i, id_couleur, -INFINI, -INFINI);
        t = clock() - t;

        if (test.score >= SCORE_MAT - i) break;

        if (t2 != 0) coeff = t / t2;
        else coeff = 1;
        t2 = t;
    }


    fprintf(stderr, "profondeur de calcul = %d soit %llu evaluations\t %d positions non recalculees\t", i - 1, NB_APPELS_HEURISTIQUES, COUPURES_HASH);
    liberer_arbre(&arbre);

    return test;


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



