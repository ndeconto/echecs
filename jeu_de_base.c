#include <stdlib.h>
#include <stdio.h>

#include "jeu_de_base.h"
#include "bitsboards.h"
#include "lister_coups.h"
#include "IA.h"

#include <time.h>
#include <math.h>
#include <string.h>

Echiquier echiquier; //TODO reflechir si avoir l'echiquier en variable globale est une betise ou non...


void init_jeu_de_base(void)
{
    /*
        alloue la memoire necessaire, cree un echiquier avec une position de depart pour toutes les pieces, etc...
    */

    int i, j;
    float t1, t2;
    char tab_pieces[] = {TOUR, CAVALIER, FOU, DAME, ROI, FOU, CAVALIER, TOUR};

    //initialisation de l'echiquier : (position classique, on verra pour le 960 ensuite)

    //echiquier.plateau = malloc(sizeof(char*) * 8);
    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++) echiquier.plateau[i][j] = 0;    //echiquier.plateau[i] = calloc(8, sizeof(char));

    for (i = 0; i < 8; i++)
    {
        echiquier.plateau[1][i] = PION * NOIR;
        echiquier.plateau[6][i] = PION * BLANC;

        echiquier.plateau[0][i] = tab_pieces[i] * NOIR;
        echiquier.plateau[7][i] = tab_pieces[i] * BLANC;
    }

    //il faudrait verifier que l'initialisation se passe bien...
    init_bitsboards(&echiquier);
    init_ia();

    echiquier.pos_rois[INDEX_BLANC].x = 4;
    echiquier.pos_rois[INDEX_BLANC].y = 7;
    echiquier.pos_rois[INDEX_NOIR].x = 4;
    echiquier.pos_rois[INDEX_NOIR].y = 0;

    echiquier.dernier_coup.case_dep.x = 8;
    echiquier.dernier_coup.case_fin.y = 8;

    for (i = 0; i < 2; i++) for (j = 0; j < 2; j++) echiquier.tour_a_bouge[i][j] = 0;


    /*Coup test;
    test.case_dep.x = 4;
    test.case_dep.y = 6;
    test.case_fin.x = 4;
    test.case_fin.y = 4;
    jouer_coup(&test, &echiquier, INDEX_BLANC);*/

    /*Coup* l = malloc(sizeof(Coup) * 1000);
    t1 = clock();
    for (i = 0; i < 10000000; i++) pseudo_legaux(&echiquier, INDEX_BLANC, l);
    t2 = clock();
    fprintf(stderr, "temps pour lister les coups = %.3f s\n", (t2 - t1) / CLOCKS_PER_SEC);
    print_l_coup(l, pseudo_legaux(&echiquier, INDEX_BLANC, l));


    free(l);*/


}


void quit_jeu_de_base(void)
{
    int i;
    //for (i = 0; i < 8; i++) free(echiquier.plateau[i]);
    //free(echiquier.plateau);
    free_bitsboards();
    free_ia();
}

char** get_plateau(void)
{
    /*
        ATTENTION, c'est un tableau 2D 8x8 qui est retourne !!
        il doit donc etre traite comme telle ensuite par l'appelant de la fonction
        privilegier la fonction get_echiquier
        //TODO renvoyer cela proprement via une structure
    */
    return echiquier.plateau;
}

Echiquier* get_echiquier(void)
{
    return &echiquier;
}

void jouer_coup(Coup* a_jouer, Echiquier* ech, char id_couleur)
{
    /*
        joue le coup demande, sans se demander si celui ci est legal
        le coup n'est pas modifie
        (plus precisement, il est modifie 2 fois en cas de promotion de pion, mais sa valeur d'origine est retablie)

        pour le roque, seul le coup du roi est passe en parametre (la fonction se debrouille apres)
        pour la promotion:
            a_jouer->case_fin.y doit etre > 7
            a_jouer->case_fin.y - 7 represente la nature en valeur absolue de la piece "creee"
            par ex, pour a_jouer = {[6, 2], [7 + DAME, 2]} represente la promotion du pion noir en dame en c1


    */

    Int mask, mask2;
    char y, p = ech->plateau[a_jouer->case_dep.y][a_jouer->case_dep.x], promo = 0;

    //PROMOTION DU PION
    if (a_jouer->case_fin.y > 7)
    {
        y = p;
        p = a_jouer->case_fin.y - 7;
        if (y < 0) p = -p;
        a_jouer->case_fin.y = (id_couleur == INDEX_BLANC ? 0 : 7);
        promo = 1;

    }

    //prise en passant a traiter a part
    if ((p == PION || p == -PION) && a_jouer->case_dep.x - a_jouer->case_fin.x != 0
        &&  ech->plateau[a_jouer->case_fin.y][a_jouer->case_fin.x] == VIDE)
    {
        ech->plateau[a_jouer->case_dep.y][a_jouer->case_fin.x] = VIDE;
        ech->pieces_couleur[!id_couleur] ^= (Int) 1 << INDEX(a_jouer->case_dep.y, a_jouer->case_fin.x);
    }



    mask = (Int) 1 << INDEX(a_jouer->case_fin.y, a_jouer->case_fin.x);
    Int not_mask = ~mask;
    mask2 = ((Int) 1 << INDEX(a_jouer->case_dep.y, a_jouer->case_dep.x));
    ech->pieces_couleur[id_couleur]     &= (~mask2);        //^= mask2;
    ech->pieces_couleur[id_couleur]     |= mask;

    if (ech->plateau[a_jouer->case_fin.y][a_jouer->case_fin.x]){
        ech->pieces_couleur[!id_couleur]        &= not_mask;         //^= mask;
        ech->bb_par_piece[!id_couleur][FOU]     &= not_mask;   //^= mask;
        ech->bb_par_piece[!id_couleur][TOUR]    &= not_mask;   //^= mask;
        ech->bb_par_piece[!id_couleur][CAVALIER]&= not_mask;   //^= mask;
    }


    ech->plateau[a_jouer->case_fin.y][a_jouer->case_fin.x] = p;
    ech->plateau[a_jouer->case_dep.y][a_jouer->case_dep.x] = VIDE;

    if (promo) mask2 = 0;
    if (p < 0) p = -p;
    switch (p)
    {

    case TOUR:

        y = (id_couleur == INDEX_BLANC ? 7 : 0);
        if (a_jouer->case_dep.x == 0 && a_jouer->case_dep.y == y) ech->tour_a_bouge[id_couleur][INDEX_AILE_DAME] = 1;
        else if (a_jouer->case_dep.x == 7 && a_jouer->case_dep.y == y) ech->tour_a_bouge[id_couleur][INDEX_AILE_ROI] = 1;

        ech->bb_par_piece[id_couleur][TOUR] ^= mask2 ^ mask;

        break;

    case FOU:
        ech->bb_par_piece[id_couleur][FOU] ^= mask2 ^ mask;
        break;

    case DAME:
        ech->bb_par_piece[id_couleur][TOUR] ^= mask2 ^ mask;
        ech->bb_par_piece[id_couleur][FOU] ^= mask2 ^ mask;
        break;

    case CAVALIER:
        ech->bb_par_piece[id_couleur][CAVALIER] ^= mask2 ^ mask;
        break;

    case ROI:
        ech->roi_a_bouge[id_couleur] = 1;
        ech->pos_rois[id_couleur] = a_jouer->case_fin;
        y = (id_couleur == INDEX_BLANC ? 7 : 0);


        Coup m_tour;
        switch (a_jouer->case_fin.x - a_jouer->case_dep.x)
        {

        case 2:                     //petit roque

            m_tour.case_dep.x = 7;
            m_tour.case_dep.y = y;
            m_tour.case_fin.x = 5;
            m_tour.case_fin.y = y;
            jouer_coup(&m_tour, ech, id_couleur);       //ech->dernier_coup sera faux du coup, mais pas d'importance et bien pratique

            break;

        case -2:                    //grand roque

            m_tour.case_dep.x = 0;
            m_tour.case_dep.y = y;
            m_tour.case_fin.x = 3;
            m_tour.case_fin.y = y;
            jouer_coup(&m_tour, ech, id_couleur);

            break;
        }

        break;


    }

    if (promo) a_jouer->case_fin.y = 7 + abs(p);
    ech->dernier_coup = *a_jouer;

}


void ecrire_coup2(const char* pathname, Coup a_ecrire, int numero, char prise)
{
    FILE* fichier = fopen(pathname, "a");
    ecrire_coup(fichier, a_ecrire, numero, prise);
    fclose(fichier);
}


void ecrire_coup(FILE* fichier, Coup a_ecrire, int numero, char prise)
/*
    il faut appeler cette fonction apres avoir joue le coup

    numero est le numero du coup a jouer
    le premier coup des noirs est le deuxieme coup, le deuxieme coup des blancs est le troisieme, ...
    un numero impair signifie un coup blanc, un numero pair signifie un coup noir

    prise = 0 signifie que le coup joue ne prend pas de piece
    prise = PRISE_EN_PASSANT indique une prise en passant
    remarque : PRISE_EN_PASSANT != 0
*/
{
    int pos, x1, x2, y1, y2, T;
    char sortie[30] = "", lignes[] = "87654321", colonnes[] = "abcdefgh", id, suffixe[5] = "", roque = 0;
    Echiquier ech = *get_echiquier();
    Coup* l_coup;


    x1 = a_ecrire.case_dep.x;
    x2 = a_ecrire.case_fin.x;
    y1 = a_ecrire.case_dep.y;
    y2 = a_ecrire.case_fin.y;

    id = (numero & 1 ? INDEX_BLANC: INDEX_NOIR);
    pos = 0;
    if (numero & 1) {
        sprintf(sortie, "%d. ", numero / 2 + 1);
        pos += strlen(sortie);
    }

    if (y2 > 7)         //en cas de promotion
    {
        switch (y2 - 7)
        {
        case DAME:
            sprintf(suffixe, "=D");
            break;

        case CAVALIER:
            sprintf(suffixe, "=C");
            break;

        case FOU:
            sprintf(suffixe, "=F");
            break;

        case TOUR:
            sprintf(suffixe, "=T");
            break;

        default:
            sprintf(suffixe, "=?");
            break;
        }

        y2 = (id == INDEX_BLANC ? COTE_NOIR : COTE_BLANC);

    }

    else {
        switch (abs(ech.plateau[y2][x2]))
        {
        case ROI:

            if (x2 - x1 == 2){      //petit roque
                sprintf(sortie + pos, "O-O");
                pos += 3;
                roque = 1;
            }
            else if (x2 - x1 == -2){    //grand roque
                sprintf(sortie + pos, "O-O-O");
                pos += 5;
                roque = 1;
            }
            else
            {
                sortie[pos++] = 'R';
            }
            break;

        case DAME:
            sortie[pos++] = 'D';
            break;

        case FOU:
            sortie[pos++] = 'F';
            break;

        case CAVALIER:
            sortie[pos++] = 'C';
            break;

        case TOUR:
            sortie[pos++] = 'T';
            break;

        case PION:
            if (prise == PRISE_EN_PASSANT) sprintf(suffixe, " e.p.");
            break;

        default:
            fprintf(stderr, "jeu_de_base.c/ecrire_coup : piece de type inconnu\n");
            return;
        }
    }

    if (roque == 0){
        sortie[pos++] = colonnes[x1];
        sortie[pos++] = lignes[y1];
        if (prise) sortie[pos++] = 'x';
        else sortie[pos++] = '-';
        sortie[pos++] = colonnes[x2];
        sortie[pos++] = lignes[y2];
    }

    sortie[pos] = '\0';
    strcat(sortie, suffixe);
    pos += strlen(suffixe);

    l_coup = malloc(sizeof(Coup) * NB_MAX_COUPS_POSSIBlES);
    T = liste_coups_valides(&ech, !id, l_coup);
    free(l_coup);

    if (T == MAT) sortie[pos++] = '#';
    else if (T == PAT) {
        sprintf(sortie + pos, "\tpat");
        pos += 4;
    }
    else if (est_attaque(&ech, id, INDEX(ech.pos_rois[!id].y, ech.pos_rois[!id].x), NULL))
        sortie[pos++] = '+';


    if (id == INDEX_BLANC) {
        sortie[pos++] = '\t'; //'\t';
        //while (pos < LARGEUR_COLONNE_PNG) sortie[pos++] = ' ';
    }
    else sortie[pos++] = '\n';

    sortie[pos] = '\0';

    fprintf(fichier, "%s", sortie);


}


char capture(Echiquier* ech, Coup* cp)
{
    /*
        cette fonction doit etre appelee avant de jouer le coup sur l'echiquier

        renvoie vrai si le coup passe en parametre capture une piece
        S'il s'agit d'une prise classique, PRISE_CLASSIQUE (!= 0) est renvoye
        s'il s'agit d'une prise en passant, PRISE_EN_PASSANT (!= 0) est renvoye

        renvoie 0 sinon
    */

    if (abs(ech->plateau[cp->case_dep.y][cp->case_dep.x]) == PION
        && cp->case_dep.x != cp->case_fin.x
        && ech->plateau[cp->case_fin.y][cp->case_fin.x] == VIDE
        )
    {

        return PRISE_EN_PASSANT;
    }

    return (ech->plateau[cp->case_fin.y][cp->case_fin.x] == VIDE ? 0 : PRISE_CLASSIQUE);

}


