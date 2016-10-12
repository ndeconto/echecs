#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>

#include "IA.h"


//cf heuristique
char VALEUR_PIECES[] = {-100, -10, -5, -3, -3, -1, 0, 1, 3, 3, 5, 10, 100};
Int NB_APPELS_HEURISTIQUES;
int COUPURES_HASH;

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
    return (init_outils() && init_hash());
}

void free_ia()
{
    free_outils();
    free_hash();
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
