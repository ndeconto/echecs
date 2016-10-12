#include <stdlib.h>
#include <stdio.h>

#include "graphiques.h"
#include "fonctions_pratiques.h"
#include <math.h>

Images imgs;
int X_DEP;
int Y_DEP;
int X_FIN;
int Y_FIN;

char init_graphiques(void)
{
    /*
        charge en memoire toutes les images dont les fonctions de graphiques.c ont besoin pour fonctionner
        renvoie 0 si tout s'est bien passe, -1 si au moins un fichier image n'a pas pu etre lu

        charge aussi SDL_TTF
    */

    int i, flags;
    const char coordonnees[] = "abcdefgh87654321";
    TTF_Font* police_echiquier;
    SDL_Color noir = {0, 0, 0};
    SDL_Surface* lettre, *lettre2;
    SDL_Rect rect;

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    if (SDL_SetVideoMode(LARGEUR_FENETRE, HAUTEUR_FENETRE, BPP, SDL_HWSURFACE | SDL_SRCALPHA | SDL_RESIZABLE) == NULL)
    {
        fprintf(stderr, "Impossible d'ouvrir une fenetre SDL : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if ((IMG_Init(flags) & flags) != flags)
    {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL_image : %s", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1)
    {
        fprintf(stderr, "Erreur lors de l'initialisation de TTF : %s", TTF_GetError());
        exit(EXIT_FAILURE);
    }


    imgs.pieces_blanches[VIDE] = NULL;
    imgs.pieces_noires[VIDE] = NULL;

    imgs.pieces_blanches[PION]       = IMG_Load("images/pion_blanc.png");
    imgs.pieces_blanches[CAVALIER]   = IMG_Load("images/cavalier_blanc.png");
    imgs.pieces_blanches[FOU]        = IMG_Load("images/fou_blanc.png");
    imgs.pieces_blanches[TOUR]       = IMG_Load("images/tour_blanche.png");
    imgs.pieces_blanches[DAME]       = IMG_Load("images/dame_blanche.png");
    imgs.pieces_blanches[ROI]        = IMG_Load("images/roi_blanc.png");

    imgs.pieces_noires[PION]         = IMG_Load("images/pion_noir.png");
    imgs.pieces_noires[CAVALIER]     = IMG_Load("images/cavalier_noir.png");
    imgs.pieces_noires[FOU]          = IMG_Load("images/fou_noir.png");
    imgs.pieces_noires[TOUR]         = IMG_Load("images/tour_noire.png");
    imgs.pieces_noires[DAME]         = IMG_Load("images/dame_noire.png");
    imgs.pieces_noires[ROI]          = IMG_Load("images/roi_noir.png");

    imgs.echiquier_blancs_en_bas     = IMG_Load("images/echiquier.png");
    imgs.echiquier_noirs_en_bas      = IMG_Load("images/echiquier.png");


    for (i = 1; i <= NB_PIECES; i++) if (imgs.pieces_blanches[i] == NULL || imgs.pieces_noires[i] == NULL) return -1;
    if (imgs.echiquier_blancs_en_bas == NULL) return -1;

    police_echiquier = TTF_OpenFont("polices/Monaco.ttf", T_POLICE_ECHIQUIER);
    TTF_Font* police_png       = TTF_OpenFont("polices/Monaco.ttf", T_POLICE_PNG);
    if (police_echiquier == NULL || police_png == NULL)
    {
        fprintf(stderr, "Impossible de lire l'une des polices de caracteres : \n%s", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    imgs.police_png = police_png;

    for (i = 0; i < 8 * 2; i++)
    {

        lettre = TTF_RenderGlyph_Blended(police_echiquier, coordonnees[i], noir);
        lettre2 = TTF_RenderGlyph_Blended(police_echiquier, coordonnees[i >= 8 ? 15 - (i - 8) : 7 - i], noir);

        rect.x = (i >= 8 ? (OFFSET_PIECES_X - lettre->w) / 2 : OFFSET_PIECES_X + i * DIM_CASE_X + (DIM_CASE_X - lettre->w) / 2);
        rect.y = (i >= 8 ? OFFSET_PIECES_Y + (i - 8) * DIM_CASE_Y + (DIM_CASE_Y - lettre->h) / 2: (OFFSET_PIECES_Y - lettre->h) / 2);
        SDL_BlitSurface(lettre, NULL, imgs.echiquier_blancs_en_bas, &rect);
        SDL_BlitSurface(lettre2, NULL, imgs.echiquier_noirs_en_bas, &rect);

        if (i >= 8) rect.x += 8 * DIM_CASE_X + OFFSET_PIECES_X;
        else rect.y += 8 * DIM_CASE_Y + OFFSET_PIECES_Y;
        SDL_BlitSurface(lettre, NULL, imgs.echiquier_blancs_en_bas, &rect);
        SDL_BlitSurface(lettre2, NULL, imgs.echiquier_noirs_en_bas, &rect);

        SDL_FreeSurface(lettre);
        SDL_FreeSurface(lettre2);
    }

    TTF_CloseFont(police_echiquier);



    return 0;


}

void quit_graphiques()
{
    /*
        libere la memoire allouee pour les images utilisees dans graphiques.c
        quitte aussi SDL_TTF
    */

    int i;

    IMG_Quit();


    for (i = 1; i <= NB_PIECES; i++)
    {
        SDL_FreeSurface(imgs.pieces_blanches[i]);
        SDL_FreeSurface(imgs.pieces_noires[i]);
    }

    SDL_FreeSurface(imgs.echiquier_blancs_en_bas);
    SDL_FreeSurface(imgs.echiquier_noirs_en_bas);
    TTF_CloseFont(imgs.police_png);

    TTF_Quit();

    SDL_Quit();
}

void afficher_echiquier(char echiquier[8][8], int x, int y, char sens)
{
    /*
        affiche a l'ecran l'echiquier

    Parametres :

        echiquier doit etre un tableau de char 8x8
        imgs doit etre initialisee avec init_images avant
        le coin en haut a gauche de l'echiquier a pour coordonnees (x, y) en pixels
        si sens == BLANCS_EN_BAS, les noirs sont affiches en haut, ie, a8 est en haut a gauche de l'ecran, h1 en bas a droite
        si sens == BLANCS_EN_HAUT, les noirs sont affiches en bas, ie, h1 est en haut a gauche de l'ecran, a8 en bas a droite

        Par convention (definie dans commun.h), les pieces blanches sont representees par un nombre entier strictement positif
                                                les pieces noires par un nombre entier strictement negatif


    */


    SDL_Surface* ecran = SDL_GetVideoSurface();
    SDL_Rect rect;
    int i, j;
    char ma_case;

    rect.x = x; rect.y = y;
    SDL_BlitSurface(sens == BLANCS_EN_BAS ? imgs.echiquier_blancs_en_bas : imgs.echiquier_noirs_en_bas, NULL, ecran, &rect);

    for (i = 0, rect.y = y + OFFSET_PIECES_Y; i < 8; i++,  rect.y += DIM_CASE_Y)
    {
        for (rect.x = x + OFFSET_PIECES_X, j = 0; j < 8; j++, rect.x += DIM_CASE_X)
        {
            ma_case = echiquier[RETOURNER(sens, i)][RETOURNER(sens, j)];
            if (ma_case == VIDE) continue;

            SDL_BlitSurface((ma_case * BLANC > 0 ? imgs.pieces_blanches : imgs.pieces_noires)[abs(ma_case)], NULL, ecran, &rect);
        }
    }

    SDL_Flip(ecran);


}

void afficher_png(const char* nom_png)
{
    int x = POS_ECHIQUIER_X + 2 * OFFSET_PIECES_X + 8 * DIM_CASE_X, y = 0;
    SDL_Color blanc = {255, 255, 255};
    SDL_Rect rect;
    SDL_Surface* f = SDL_GetVideoSurface();

    rect.x = x; rect.y = y;
    rect.w = LARGEUR_FENETRE - x;
    rect.h = HAUTEUR_FENETRE - y;
    SDL_FillRect(f, &rect, SDL_MapRGB(f->format, 0, 0, 0));

    print_from_file(nom_png, blanc, imgs.police_png, x, y, LARGEUR_FENETRE - x);
}

void afficher_aide()
{
    SDL_Rect rect;
    SDL_Surface* screen = SDL_GetVideoSurface();
    rect.x = POS_ECHIQUIER_X;
    rect.y = POS_ECHIQUIER_Y + imgs.echiquier_blancs_en_bas->h + OFFSET_TEXTE;
    SDL_Surface* s = TTF_RenderText_Blended(imgs.police_png,
                                           "Pour afficher les commandes possibles, appuyez sur A",
                                           (SDL_Color) {255, 255, 255});
    SDL_BlitSurface(s, NULL, screen, &rect);
    SDL_UpdateRect(screen, rect.x, rect.y, rect.w, rect.h);
    SDL_FreeSurface(s);
}

void afficher_commandes()
{
    message("A : retourne l'echiquier\nI : active l'IA\nO: desactive l'IA\nL: desactive l'alternance \
            des coups blanc/noir\nM: force l'alternance un coup blanc / un coup noir\nZ: affiche ce rappel des commandes",
            T_POLICE_PNG, NULL, 128, FLAG_OK);
}

void afficher_tout(char echiquier[8][8], int x, int y, char sens, const char* nom_png)
{
    SDL_Surface* screen = SDL_GetVideoSurface();
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    afficher_echiquier(echiquier, x, y, sens);
    //afficher_png("un essai.txt");
    afficher_png(nom_png);
    afficher_aide();
}

void pause(void)
{
    SDL_Event ev;

    do SDL_WaitEvent(&ev); while (ev.type != SDL_KEYDOWN && ev.type != SDL_QUIT);
}


char recuperer_coup(SDL_Event* ev, int* x_dep, int* y_dep, int* x_fin, int* y_fin, char sens)
{
    /*
        renvoie 1 si la fonction a recupere quelque chose de potentiellement valide, 0 sinon

        le potentiellement valide est ecrit dans les pointeurs passes en parametres
        ces coordonnees sont donnes en case, ie 0 <= x_dep < 8 (idem pour les autres parametres)
    */

    static int select_x = -1, select_y = -1;

    int x = ev->button.x, y = ev->button.y;
    x -= POS_ECHIQUIER_X + OFFSET_PIECES_X;
    y -= POS_ECHIQUIER_Y + OFFSET_PIECES_Y;

    if ((ev->type != SDL_MOUSEBUTTONDOWN && ev->type != SDL_MOUSEBUTTONUP)
        || x < 0 || ev->button.x >= 8 * DIM_CASE_X || y < 0 || ev->button.y >= 8 * DIM_CASE_Y
        )
    {
        return 0;
    }

    if (ev->type == SDL_MOUSEBUTTONDOWN)
    {
        select_x = ev->button.x;
        select_y = ev->button.y;

        return 0;
    }

    if (select_x < 0 || select_y < 0) return 0;

    *x_dep = select_x / DIM_CASE_X;
    *y_dep = select_y / DIM_CASE_Y;
    *x_fin = x / DIM_CASE_X;
    *y_fin = y / DIM_CASE_Y;

    if (sens == NOIRS_EN_BAS)
    {
        *x_dep = 7 - *x_dep;
        *y_dep = 7 - *y_dep;
        *x_fin = 7 - *x_fin;
        *y_fin = 7 - *y_fin;
    }

    return 1;

}


int gestion_entrees(char* sens_echiquier)
{
    /*
        renvoie un code selon ce qui s'est passe
        les codes sont definis dans graphiques.h
    */

    SDL_Event ev;

    do
    {
        SDL_WaitEvent(&ev);


        switch (ev.type)
        {

        case SDL_QUIT:
            return QUITTER;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (recuperer_coup(&ev, &X_DEP, &Y_DEP, &X_FIN, &Y_FIN, *sens_echiquier)) return JOUER_COUP;
            break;

        case SDL_KEYDOWN:
            switch (ev.key.keysym.sym)
            {
            case SDLK_q:
                if (*sens_echiquier == BLANCS_EN_BAS) *sens_echiquier = NOIRS_EN_BAS;
                else *sens_echiquier = BLANCS_EN_BAS;
                return RAFRAICHIR_ECHIQUIER;

            case SDLK_i:
                return ACTIVER_IA;

            case SDLK_o:
                return DESACTIVER_IA;

            case SDLK_l:
                return JEU_LIBRE;

            case SDLK_m:
                return JEU_ALTERNE;

            case SDLK_w:
                return AFFICHER_COMMANDES;

            default:
                break;
            }

        }



    } while (1);
}


void get_coup_joue(int* x_dep, int* y_dep, int* x_fin, int* y_fin)
{
    *x_dep = X_DEP;
    *y_dep = Y_DEP;
    *x_fin = X_FIN;
    *y_fin = Y_FIN;
}


char choix_promotion()
{
    /*
        demande a l'utilisateur la nature de la promotion
        renvoie la nature de la nouvelle piece en valeur absolue

        renvoie -1 si il y a eu un probleme (l'utilisateur a voulu quitter par exemple)
    */

    SDL_Surface* copie, *copie2, *screen = SDL_GetVideoSurface();
    SDL_Rect rect;
    SDL_Event ev;
    char l_choix[] = {DAME, FOU, CAVALIER, TOUR}, retour;
    int i, l_x[5], d_x = imgs.pieces_blanches[DAME]->w, d_y = imgs.pieces_blanches[DAME]->h;

    rect.x = 0; rect.y = 0;
    copie = SDL_CreateRGBSurface(screen->flags, screen->w, screen->h, BPP,
                        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    copie2 = SDL_CreateRGBSurface(screen->flags, screen->w, screen->h, BPP,
                        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_BlitSurface(screen, NULL, copie, &rect);
    rect.w = screen->w;
    rect.h = screen->h;
    SDL_FillRect(copie2, &rect, SDL_MapRGBA(screen->format, 0, 0, 0, 0));
    SDL_SetAlpha(copie2, SDL_SRCALPHA, TRANPARENCE_PREMIER_PLAN);
    SDL_BlitSurface(copie2, NULL, screen, &rect);

    rect.y = (imgs.echiquier_blancs_en_bas->h - POS_ECHIQUIER_Y - d_y) / 2;
    rect.x = (imgs.echiquier_blancs_en_bas->w - POS_ECHIQUIER_X - 4 * (d_x + ECART_PROMOTION)) / 2;
    l_x[0] = rect.x;
    for (i = 0; i < 4; i++)
    {
        SDL_BlitSurface(imgs.pieces_blanches[l_choix[i]], NULL, screen, &rect);
        rect.x += d_x + ECART_PROMOTION;
        l_x[i + 1] = rect.x;
    }

    SDL_Flip(screen);

    retour = -1;
    do
    {
        SDL_WaitEvent(&ev);

        if (ev.type == SDL_QUIT) {
            retour = -1;
            break;
        }

        if (ev.type != SDL_MOUSEBUTTONUP) continue;

        if (ev.button.y < rect.y || ev.button.y > rect.y + d_y) continue;
        for (i = 0; i < 4; i++)
        {
            if (ev.button.x >= l_x[i] && ev.button.x < l_x[i + 1])  retour = l_choix[i];
        }

    } while (retour == - 1);

    rect.x = 0; rect.y = 0;
    SDL_BlitSurface(copie, NULL, screen, &rect);
    SDL_Flip(screen);
    SDL_FreeSurface(copie);
    SDL_FreeSurface(copie2);

    return retour;
}
