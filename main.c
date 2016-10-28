#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>

#include "graphiques.h"
#include "jeu_de_base.h"
#include "lister_coups.h"
#include "IA.h"
#include "fonctions_pratiques.h"


#define JOUEUR_IA 1

int main ( int argc, char** argv )
{
    int i, a_faire, x1, x2, y1, y2, temp, joueur = INDEX_BLANC, numero_coup = 1;
    char id_couleur, sens = BLANCS_EN_BAS, jeu_libre = 0, prise, couleur_ia = JOUEUR_IA;
    float t;
    Coup a_jouer, *l_coup = malloc(sizeof(Coup) * NB_MAX_COUPS_POSSIBlES);
    Echiquier* ech;
    Coup_IA trouve;
    char txt_menu[NB_MOTS][M_MAX] = {"Bonjour", "Ouaiiis", "Ca marche"};

    mkdir("png");
    char nom_png[100] = "png/png#1.txt";
    trouver_nouveau_nom(nom_png);

    init_graphiques();
    atexit(quit_graphiques);
    init_jeu_de_base();
    atexit(quit_jeu_de_base);


    //menu(SDL_GetVideoSurface(), txt_menu, 3, 200, COULEUR_NOIR, COULEUR_ROUGE);

    SDL_WM_SetCaption("Jean-Pion", NULL);
    afficher_tout(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens, nom_png);
    //afficher_echiquier(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens);
    //fprintf(stderr, "%d\n", message("Echec et mat", 30, "Monaco.ttf", 80, 1));
    //afficher_echiquier(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens);

    Int hash_dep = hash_aleat();        //TODO initialiser les extras bits

    while (1)
    {
        a_faire = gestion_entrees(&sens);

        switch (a_faire)
        {
        case QUITTER:
            goto END_MAIN_LOOP;

        case JOUER_COUP:
            ech = get_echiquier();
            get_coup_joue(&x1, &y1, &x2, &y2);

            if (ech->plateau[y1][x1] == VIDE) continue;
            id_couleur = (ech->plateau[y1][x1] * BLANC > 0 ? INDEX_BLANC : INDEX_NOIR);
            if (jeu_libre) joueur = id_couleur;
            if (id_couleur != joueur) continue;

            //print_l_coup(&(ech->dernier_coup), 1);
            a_jouer.case_dep.x = x1;
            a_jouer.case_dep.y = y1;
            a_jouer.case_fin.x = x2;
            a_jouer.case_fin.y = y2;

            //promotion
            if ((id_couleur == INDEX_BLANC && y2 == 0 && ech->plateau[y1][x1] == PION * BLANC)
                || (id_couleur == INDEX_NOIR && y2 == 7 && ech->plateau[y1][x1] == PION * NOIR))
            {
                a_jouer.case_fin.y = 7 + choix_promotion();

            }
            temp = liste_coups_valides(ech, id_couleur, l_coup);
            //print_l_coup(l_coup, temp);
            //print_bb(ech->bb_par_piece[INDEX_BLANC][TOUR]);
            //print_bb(ech->bb_par_piece[INDEX_BLANC][FOU]);
            if (is_in_liste(l_coup, temp, a_jouer)) {

                prise = capture(ech, &a_jouer);
                maj_hash_pos(hash_dep, &a_jouer, ech);
                jouer_coup(&a_jouer, ech, id_couleur);
                ecrire_coup2(nom_png, a_jouer, numero_coup++, prise);



                afficher_tout(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens, nom_png);
                joueur = (joueur == INDEX_BLANC ? INDEX_NOIR : INDEX_BLANC);
            }

            break;

        case RAFRAICHIR_ECHIQUIER:
            afficher_echiquier(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens);
            break;

        case ACTIVER_IA:

            if (message("Voulez-vous activer l'ia pour les noirs? Elle peut etre desactivee lors de votre tour de jeu en appuyant sur o",
                        30, NULL, TRANSPARENCE_PAR_DEFAUT, FLAG_OK | FLAG_NO) == BOUTON_OUI)
                        couleur_ia = JOUEUR_IA;         //TODO l'ia doit pouvoir jouer dans les 2 camps
            break;

        case DESACTIVER_IA:
            couleur_ia = -1;
            message("L'ia a ete desactivee ; pour la reactiver, appuyez sur i", 30, NULL, TRANSPARENCE_PAR_DEFAUT, FLAG_OK);
            break;

        case JEU_LIBRE:         //ie l'alternance des coups blancs / noirs n'est plus imposee
            jeu_libre = 1;
            break;

        case JEU_ALTERNE:
            jeu_libre = 0;
            break;

        case AFFICHER_COMMANDES:
            afficher_commandes();
            break;

        default:
            SDL_Delay(10);
            break;
        }



        if (joueur == couleur_ia)
        {
            id_couleur = joueur;
            //fprintf(stderr, "evaluation heuristique = %.3f\n", -heuristique(get_echiquier(), id_couleur));
            t = clock();
            ech = get_echiquier();
            trouve = meilleur_coup_profondeur_auto(ech, id_couleur, 1, hash_dep);
            fprintf(stderr, "coup joue en %.2f\n", (clock() - t) / CLOCKS_PER_SEC);
            //fprintf(stderr, "score apres analyse = %.3f\n", trouve.score);


            prise = capture(ech, &(trouve.coup));
            hash_dep = maj_hash_pos(hash_dep, &(trouve.coup), ech);
            jouer_coup(&(trouve.coup), ech, id_couleur);
            ecrire_coup2(nom_png, trouve.coup, numero_coup++, prise);


            afficher_tout(get_plateau(), POS_ECHIQUIER_X, POS_ECHIQUIER_Y, sens, nom_png);
            joueur = (joueur == INDEX_BLANC ? INDEX_NOIR : INDEX_BLANC);
        }

    }

    END_MAIN_LOOP:
    //quit_graphiques();
    free(l_coup);

    fprintf(stderr, "Le programme s'est execute correctement");
    return 0;

}
