#include <stdio.h>
#include <stdlib.h>

#include "lister_coups.h"
#include <math.h>


int pseudo_legaux(const Echiquier* echiquier, char id_couleur, Coup* liste_coup)
{
    /*
        ecrit dans liste_coup tous les coups pseudo-possibles pour le joueur id_couleur
        id_couleur == INDEX_BLANC ou INDEX_NOIR (definis dans bitsboards.h)

        on ne verifie pas que le roi n'est pas en echec apres le coup

        renvoie le nombre de coups pseudo-possibles

        moyen d'optimiser eventuellement ?
            tester si une case est rempli par un piece alliee / adverse
            en utilisant les bitsboards echiquier->pieces_couleur
    */

    int i, j, k, l, nb = 0;
    char couleur = (id_couleur == INDEX_BLANC ? BLANC : NOIR), temp;
    Coordonnees dep;
    L_cases c;
    L_cases_x4 c2;



    for (i = 0; i < 8; i++) for (j = 0; j < 8; j++)
    {
        if (echiquier->plateau[i][j] * couleur <= 0) continue;

        dep.x = j;
        dep.y = i;

        switch (abs(echiquier->plateau[i][j]))
        {

        case CAVALIER:
            c = L_CASES_CAVALIER[INDEX(i, j)];

            PARCOURS_L(echiquier, c, liste_coup, couleur, nb, dep, k);

            /*for (k = 0; k < c.taille; k++) if (echiquier->plateau[c.liste[k].y][c.liste[k].x] * couleur <= 0) {
                liste_coup[nb].case_dep = dep;
                liste_coup[nb++].case_fin = c.liste[k];
            }*/


            break;


        case FOU:
            c2 = L_CASES_FOU[INDEX(i, j)];

            PARCOURS_Lx4(echiquier, c2, liste_coup, couleur, nb, dep, k, l, temp);

            /*for (l = 0; l < 4; l++)
            {
                for (k = 0; k < c.taille[l]; k++)
                {
                    contenu = echiquier->plateau[c.liste[l][k].y][c.liste[l][k].x] * couleur;
                    if (contenu <= 0) liste_coup[nb++].case_fin = c.liste[l][k];
                    if (contenu) break;
                }
            }*/

            break;

        case TOUR:
            c2 = L_CASES_TOUR[INDEX(i, j)];

            PARCOURS_Lx4(echiquier, c2, liste_coup, couleur, nb, dep, k, l, temp);

            break;


        case ROI:
            c = L_CASES_ROI[INDEX(i, j)];
            PARCOURS_L(echiquier, c, liste_coup, couleur, nb, dep, k);

            if (echiquier->roi_a_bouge[id_couleur] == 0)
            {
                temp = (couleur == BLANC ? 7 : 0);

                /*
                    pour coder les roques, coder uniquement le mouvement du roi ?
                    la fonction bouger pourrait facilement reconnaitre le roque :
                    si le roi a bouge de 2 cases en x, alors il faut aussi bouger la tour

                    autre idee ?
                */

                //roques :
                liste_coup[nb].case_dep = dep;
                liste_coup[nb].case_fin.y = temp;

                if (est_attaque(echiquier, echiquier->plateau[i][j] > 0 ? INDEX_NOIR : INDEX_BLANC, INDEX(i, j), NULL)) break;

                //grand roque
                if (abs(echiquier->plateau[temp][0]) == TOUR && echiquier->tour_a_bouge[id_couleur][INDEX_AILE_DAME] == 0          //la tour peut avoir ete prise sans qu'elle ait bouge...
                    && echiquier->plateau[temp][1] == VIDE && echiquier->plateau[temp][2] == VIDE && echiquier->plateau[temp][3] == VIDE
                    && est_attaque(echiquier, !id_couleur, INDEX(temp, 3), NULL) == 0)
                {
                    liste_coup[nb++].case_fin.x = 2;
                }

                liste_coup[nb].case_dep = dep;                  //NB les deux roques peuvent etre possibles en meme temps
                liste_coup[nb].case_fin.y = temp;

                //petit roque
                if (abs(echiquier->plateau[temp][7]) == TOUR && echiquier->tour_a_bouge[id_couleur][INDEX_AILE_ROI] == 0
                    && echiquier->plateau[temp][5] == VIDE && echiquier->plateau[temp][6] == VIDE
                    && est_attaque(echiquier, !id_couleur, INDEX(temp, 5), NULL) == 0)
                {
                    liste_coup[nb++].case_fin.x = 6;
                }
            }


            break;

        case PION:
            /*
                pour la promotion, le coup "normal" est ecrit
                mais coup.case_fin.y est modifie de sorte que coup.case_fin.y - 7 == la_nature_de_la_promotion
                on a donc coup.case_fin.y > 7, et c'est normal

                la macro PROMOTION fait la modification du coup si la poussee de pion est une promotion
                NB : 4 coups sont ajoutes au lieu d'un
            */

            temp = (couleur == BLANC ? -1 : 1);         //variable utilisee dans tout "case PION:"

            //poussee sans prise
            if (echiquier->plateau[i + temp][j] == VIDE)
            {
                //poussee classique
                ADD_COUP(liste_coup, nb, dep, j, i + temp);
                PROMOTION(liste_coup, i, k, nb, couleur, 0);

                //poussee initiale de 2 pas
                if (i == (couleur == BLANC ? 6 : 1) && echiquier->plateau[i + (temp << 1)][j] == VIDE)
                {
                    ADD_COUP(liste_coup, nb, dep, j, i + (temp << 1));
                }
            }


            //prise classique
            for (k = -1; k <= 1; k += 2) if (j + k >= 0 && j + k < 8 && echiquier->plateau[i + temp][j + k] * couleur < 0)
            {
                ADD_COUP(liste_coup, nb, dep, j + k, i + temp);
                PROMOTION(liste_coup, i, l, nb, couleur, k);
            }



            //prise en passant
            if (i == (couleur == BLANC ? 3 : 4)
                && echiquier->plateau[echiquier->dernier_coup.case_fin.y][echiquier->dernier_coup.case_fin.x] * couleur == -PION
                && abs(echiquier->dernier_coup.case_fin.x - j) == 1
                && abs(echiquier->dernier_coup.case_dep.y - echiquier->dernier_coup.case_fin.y) == 2)
            {
                ADD_COUP(liste_coup, nb, dep, echiquier->dernier_coup.case_fin.x, i + temp);
            }

            break;

        case DAME:
            c2 = L_CASES_FOU[INDEX(i, j)];
            PARCOURS_Lx4(echiquier, c2, liste_coup, couleur, nb, dep, k, l, temp);
            c2 = L_CASES_TOUR[INDEX(i, j)];
            PARCOURS_Lx4(echiquier, c2, liste_coup, couleur, nb, dep, k, l, temp);

            break;

        }
    }

    //print_l_coup(liste_coup, nb);

    return nb;
}


void print_l_coup(const Coup* liste, int taille)
{
    int i;

    fprintf(stderr, "liste des coups format (y, x) : \n");
    for (i = 0; i < taille; i++)
    {
        fprintf(stderr, "%d : \t(%d, %d) -> (%d, %d)\n", i, liste[i].case_dep.y, liste[i].case_dep.x, liste[i].case_fin.y, liste[i].case_fin.x);
    }

    fprintf(stderr, "\n");
}


char is_in_liste(const Coup* liste, int taille, Coup a_chercher)
{
    int i;
    for (i = 0; i < taille; i++) if (MEME_COUP(liste[i], a_chercher)) return 1;
    return 0;
}


char est_attaque(const Echiquier* ech, char id_couleur, char id_case, char* value_attaquant)
{
    /*
     renvoie 1 si la case d'index id_case est attaquee par une piece de couleur id_couleur
     0 sinon

     value_attaquant donne la nature (en valeur absolue) de la piece la plus faible attaquant la case concernee
     le tableau doit etre alloue et suffisament grand
     la taille de ce tableau est alors retournee
     mettre a NULL si on s'en fout (les performances sont alors meilleures)
    */


    //PIONS
    char temp = 0, temp2, x = id_case & 7, y = id_case >> 3, nb_attaquant = 0;
    if (id_couleur == INDEX_BLANC && id_case < 48) temp = 1;
    else if (id_couleur == INDEX_NOIR && id_case > 15) temp = -1;
    if (temp &&
            (((id_case & 7) > 0 && ech->plateau[y + temp][x - 1] == temp * PION)
            ||((id_case & 7) < 7 && ech->plateau[y + temp][x + 1] == temp * PION)
            ))
    {
        if (value_attaquant != NULL) value_attaquant[nb_attaquant++] = PION;
        else return 1;
    }

    //CAVALIERS
    if (MOUV_CAVALIER[id_case] & ech->bb_par_piece[id_couleur][CAVALIER])
    {
        if (value_attaquant != NULL) value_attaquant[nb_attaquant++] = CAVALIER;
        else return 1;
    }


    Int total = ech->pieces_couleur[INDEX_BLANC] | ech->pieces_couleur[INDEX_NOIR];
    char id_1;
    Int ref = ech->bb_par_piece[id_couleur][FOU] & MOUV_FOU[id_case];

    //FOUS + DAME
    while (ref)
    {
        id_1 = bb_scan_droite(ref);
        if ((ENTRE_DIAGONALE[id_case][id_1] & total) == 0) {
            if (value_attaquant != NULL) value_attaquant[nb_attaquant++] = abs(ech->plateau[id_1 >> 3][id_1 & 7]);
            else return 1;
        }
        ref ^= (Int) 1 << id_1;
    }

    ref = ech->bb_par_piece[id_couleur][TOUR] & MOUV_TOUR[id_case];
    //TOURS + DAME
    while (ref)             //les 2 boules reunies font au plus 3 tours
    {
        id_1 = bb_scan_droite(ref);
        if ((ENTRE_LIGNE[id_case][id_1] & total) == 0)
        {
            if (value_attaquant != NULL) value_attaquant[nb_attaquant++] = abs(ech->plateau[id_1 >> 3][id_1 & 7]);
            else return 1;
        }
        ref ^= (Int) 1 << id_1;
    }




    //ROI
    temp = ech->pos_rois[id_couleur].x - x;
    if (temp == 1 || temp == 0 || temp == -1)
    {
        temp2 = ech->pos_rois[id_couleur].y - y;
        if (temp2 == 1 || (temp2 == 0 && temp) || temp2 == -1)
        {
            if (value_attaquant != NULL) value_attaquant[nb_attaquant++] = ROI;
            else return 1;
        }
    }



    return nb_attaquant;

}


int liste_coups_valides(const Echiquier* ech, char id_couleur, Coup* l_coup)
{
    /*
        calcule la liste des coups possibles pour le joueur id_couleur
        ecrit le resultat dans l_coup
        renvoie le nombre de coups possibles

        si aucun coup n'est possible, la fonction renvoie MAT ou PAT
        rem : MAT, PAT < 0
            ie si le retour de la fonction n'est pas strictement positif,
            la partie est terminee

    */

    Coup* a_tester = malloc(sizeof(Coup) * NB_MAX_COUPS_POSSIBlES);
    Echiquier virtuel;
    int i, compteur = 0, T;


    T = pseudo_legaux(ech, id_couleur, a_tester);
    for (i = 0; i < T; i++)
    {
        virtuel = *ech;
        jouer_coup(a_tester + i, &virtuel, id_couleur);
        if (est_attaque(&virtuel, !id_couleur, INDEX(virtuel.pos_rois[id_couleur].y, virtuel.pos_rois[id_couleur].x), NULL)) continue;

        l_coup[compteur++] = a_tester[i];

    }

    free(a_tester);


    if (compteur) return compteur;
    if (est_attaque(ech, !id_couleur, INDEX(ech->pos_rois[id_couleur].y, ech->pos_rois[id_couleur].x), NULL)) return MAT;
    return PAT;

}
