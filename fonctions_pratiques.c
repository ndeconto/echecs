#include "fonctions_pratiques.h"

SDL_Color COULEUR_NOIR = {0, 0, 0};
SDL_Color COULEUR_ROUGE = {255, 0, 0};
SDL_Color COULEUR_BLANC = {255, 255, 255};
SDL_Color COULEUR_GRIS = {170, 170, 170};

Uint32 getpixel(const SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {

    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
            return *(Uint32 *)p;

    default:
        return 0; /* shouldn’t happen, but avoids warnings */
    }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;

    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }

        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;

    default:
        return;
    }
}

void blitRGBA(const SDL_Surface* source, const SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect)
{
    /*
        colle source sur dest en copiant pixel par pixel ; la transparence est aussi recopiee
        ce que ne fait pas SDL_BlitSurface (qui remplace les pixels)
    */

    int x, y, T_y, T_x, offset_x, offset_y;
    Uint32 pixel_source;
    Uint8 r, g, b, a;

    //SDL_BlitSurface(source, NULL, SDL_GetVideoSurface(), NULL);
    //SDL_Flip(SDL_GetVideoSurface());
    SDL_LockSurface(dest);
    SDL_LockSurface(source);

    offset_y = (srcrect != NULL ? srcrect->y : 0) + (destrect != NULL ? destrect->y : 0);
    offset_x = (srcrect != NULL ? srcrect->x : 0) + (destrect != NULL ? destrect->x : 0);
    T_y = (srcrect != NULL ? srcrect->h : source->h);
    T_x = (srcrect != NULL ? srcrect->w : source->w);



    for (y = offset_y; y < T_y + offset_y; y++) for (x = offset_x; x < T_x + offset_x; x++)
    {
        pixel_source = getpixel(source, x - offset_x, y - offset_y);
        SDL_GetRGBA(pixel_source, source->format, &r, &g, &b, &a);
        putpixel(dest, x, y, getpixel(source, x - offset_x, y - offset_y));

    }


    SDL_UnlockSurface(dest);
    SDL_UnlockSurface(source);

}

void draw_menu(SDL_Surface* screen, TTF_Font* police, char choix[NB_MOTS][M_MAX], int nb, SDL_Color color, SDL_Rect* result)
/*  calcule les coordonnees de chaque texte clicable, et stocke dans le tab de Rect result qui doit etre initialise */
{
    int i, espace, y;

    SDL_Surface* texte = TTF_RenderText_Blended(police, choix[0], color);

    espace = (screen->h - texte->h * nb) / (nb + 1) ;
    y = espace;
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));

    for (i = 0; i != nb; i++, y += espace)
    {
        texte = TTF_RenderText_Blended(police, choix[i], color);
        result[i].x = (screen->w - texte->w) >> 1;
        result[i].y = y;
        result[i].w = texte->w;
        result[i].h = texte->h;

        SDL_BlitSurface(texte, NULL, screen, result + i);
        y += texte->h;
    }

    SDL_Flip(screen);
    SDL_FreeSurface(texte);
}

int in_rect(SDL_Rect* rect, int x, int y)
{
    return (rect->x <= x && rect->x + rect->w >= x && rect->y <= y && rect->y + rect->h >= y);
}

void afficher_bouton(SDL_Surface* screen, char* txt, SDL_Rect* rect, TTF_Font* police, SDL_Color color)
{
    SDL_FillRect(screen, rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
    SDL_Surface* texte = TTF_RenderText_Blended(police, txt, color);
    SDL_BlitSurface(texte, NULL, screen, rect);
    SDL_UpdateRect(screen, rect->x, rect->y, rect->w, rect->h);
    SDL_FreeSurface(texte);
}

int menu (SDL_Surface* screen, char choix[NB_MOTS][M_MAX], int nb, int size, SDL_Color color1, SDL_Color color2)
{
    /*
        affiche un menu (bloquant) et le gere (evenements, etc...)
        renvoie le numero du bouton clique
        par exemple, si choix = ["a", "b", "c"], et "c" a ete clique, renvoie 2

        le fond est en blanc...
        //TODO pouvoir changer le fond des menus
    */
    SDL_Rect    *positions = malloc(sizeof(SDL_Rect) * nb);
    TTF_Font    *police = TTF_OpenFont("monaco.ttf", size);
    SDL_Event   ev;
    int         i, in = -1;

    draw_menu(screen, police, choix, nb, color1, positions);

    while (1)
    {
        SDL_WaitEvent(&ev);
        switch (ev.type)
        {
            case SDL_QUIT:      //TODO ne pas quitter directement, mais dire a l'appelant que l'utilisateur veut quitter
                free(positions);
                TTF_CloseFont(police);
                SDL_Quit();
                TTF_Quit();
                exit(EXIT_SUCCESS);
                return -1;
            case SDL_MOUSEBUTTONDOWN:
                for (i = 0; i < nb; i++)
                {
                    if (in_rect(positions + i, ev.button.x, ev.button.y)) {
                        free(positions);
                        TTF_CloseFont(police);
                        return i;
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < nb; i++)
                {
                    if (in != i && in_rect(positions + i, ev.motion.x, ev.motion.y)){
                        if (i != -1) afficher_bouton(screen, choix[in], positions + in, police, color1);
                        afficher_bouton(screen, choix[i], positions + i, police, color2);
                        in = i;
                        break;
                    }
                }
                if (in != -1 && i == nb && in_rect(positions + in, ev.motion.x, ev.motion.y) == 0){
                    afficher_bouton(screen, choix[in], positions + in, police, color1);
                    in = -1;
                }
                break;
        }
    }
}


SDL_Surface* masquer_fond(unsigned char transparence)
{
    /*
        colle une image noire et partiellement opaque sur l'ecran entier
        place ainsi celui ci en arriere plan
        un pointeur sur une copie de l'ecran d'origine est renvoyee
        ce pointeur devra etre "free" apres usage
        plus transparence est proche de 0, plus l'image est opaque
        plus elle est proche de 255, plus l'image est transparente
    */

    SDL_Surface* screen = SDL_GetVideoSurface(), *copie, *copie2;
    SDL_Rect rect;

    copie = SDL_CreateRGBSurface(screen->flags, screen->w, screen->h, screen->format->BitsPerPixel,
                screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_BlitSurface(screen, NULL, copie, NULL);
    copie2 = SDL_CreateRGBSurface(screen->flags, screen->w, screen->h, screen->format->BitsPerPixel,
                screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

    rect.x = 0;
    rect.y = 0;
    rect.w = screen->w;
    rect.h = screen->h;
    SDL_FillRect(copie2, &rect, SDL_MapRGBA(screen->format, 0, 0, 0, 0));
    SDL_SetAlpha(copie2, SDL_SRCALPHA, 0xff - transparence);
    SDL_BlitSurface(copie2, NULL, screen, &rect);
    SDL_FreeSurface(copie2);

    return copie;


}

void fusionner_surface(SDL_Surface** fusion, const SDL_Surface* s2, int espace, char sens)
{
    /*
        "colle" les 2 surfaces l'une a cote de l'autre avec espace pixels de libre en les 2
        sens peut etre COLLE_A_DROITE, COLLE_A_GAUCHE, COLLE_EN_BAS ou COLLE_EN_HAUT
        ie : pour sens == COLLE_A_DROITE, s2 est collee a droite de fusion
        la surface resultante est ecrite dans *fusion (*fusion est donc remplacee)

        les 2 surfaces doivent avoir le meme format
    */

    SDL_Surface* result;
    SDL_Rect rect;
    SDL_PixelFormat* f = (*fusion)->format;


    result = SDL_CreateRGBSurface((*fusion)->flags,
                (sens == COLLE_A_DROITE || sens == COLLE_A_GAUCHE) ? (*fusion)->w + s2->w + espace : MAX((*fusion)->w, s2->w),
                (sens == COLLE_A_DROITE || sens == COLLE_A_GAUCHE) ?  MAX((*fusion)->h, s2->h) : (*fusion)->h + s2->h + espace,
                f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);

    rect.x = 0; rect.y = 0; rect.w = result->w; rect.h = result->h;
    SDL_FillRect(result, &rect, SDL_MapRGBA(f, 0, 255, 0, SDL_ALPHA_TRANSPARENT));


    if (sens == COLLE_A_GAUCHE) rect.x = s2->w + espace;
    if (sens == COLLE_EN_HAUT) rect.y = s2->h + espace;

    blitRGBA(*fusion, NULL, result, NULL);

    if (sens == COLLE_A_DROITE) rect.x = (*fusion)->w + espace;
    if (sens == COLLE_EN_BAS) rect.y = (*fusion)->h + espace;
    if (sens == COLLE_A_GAUCHE || sens == COLLE_EN_HAUT)
    {
        rect.x = 0;
        rect.y = 0;
    }
    blitRGBA(s2, NULL, result, &rect);


    SDL_FreeSurface(*fusion);
    *fusion = result;

}

SDL_Surface* render_auto_intelligent(TTF_Font* font, const char* texte, SDL_Color couleur, int longueur_ligne)
{
    /*
    fait les retours a la ligne automatiquement afin que la taille de chaque ligne ne depasse pas longueur ligne
    les coupures sont faites au niveau des espaces ou des '\n'
    */

    int i, j, k, l, dep, n, largeur_espace, somme, bonus, bonus2, *saut, nb_mots_max, *tabulations, nb_tabu, tabu_act, nb_espace_supp;
    char** tab_mot, *ligne;
    char espace[] = " ";
    SDL_Surface* s1, *result, **tab_surface;

    for (nb_mots_max = 1, nb_tabu = 0, i = 0; texte[i] != '\0'; i++) {
        if (texte[i] == ' '  || texte[i] == '\n' || texte[i] == '\t') nb_mots_max++;
        if (texte[i] == '\t') nb_tabu++;
    }
    tab_mot = malloc(sizeof(char*) * nb_mots_max);
    tab_surface = malloc(sizeof(SDL_Surface*) * nb_mots_max);
    tabulations = malloc(sizeof(int) * (nb_tabu + 1));
    tabulations[nb_tabu] = nb_mots_max + 1;
    saut = malloc(sizeof(int) * NB_MOTS); //remplacer NB_MOTS par nb_mots_max ?

    for (n = 0, dep = 0, tabu_act = 0, i = 0; 1; i++)
    {

        if (texte[i] != ' ' && texte[i] != '\n' && texte[i] != '\0' && texte[i] != '\t') continue;
        tab_mot[n] = malloc(sizeof(char) * (i - dep + 2)); //+1 pour le chr de fin de mot, +1 pour le '\0'; * M_MAX
        for (j = dep; j < i; j++) tab_mot[n][j - dep] = texte[j];
        if (texte[i] == ' ' || texte[i] == '\t') {
            tab_mot[n][i - dep] = ' ';
            tab_mot[n][i - dep + 1] = '\0';
            if (texte[i] == '\t') tabulations[tabu_act++] = n;
        }
        else tab_mot[n][i - dep] = '\0';

        saut[n] = (texte[i] == '\n');       //force un retour a la ligne apres le mot

        if (tab_mot[n][0] != '\0')
        {
            tab_surface[n] = TTF_RenderText_Blended(font, tab_mot[n], couleur);

            if (tab_surface[n] == NULL){
                fprintf(stderr, "render_auto_intelligent a rencontre un probleme :\n%s", TTF_GetError());
            }
        }

        dep = i + 1;
        n++;
        if (texte[i] == '\0') break;
    }

    s1 = TTF_RenderText_Blended(font, espace, couleur);
    largeur_espace = s1->w;
    SDL_FreeSurface(s1);
    ligne = malloc(sizeof(char) * NB_MOTS);

    for (somme = 0, tabu_act = 0, i = 0, dep = 0; i <= n; i++)
    {
        //on teste si il y avait un saut force a la fin du mot precedent
        if (i < n && (i == 0 || (1 <= i && saut[i - 1] == 0))) {
            bonus = (tab_mot[i][0] == '\0' ? 0: tab_surface[i]->w) + largeur_espace;
            if (i == tabulations[tabu_act]) bonus += ESPACE_TAB_MAX;
            bonus2 = bonus;
        }
        else {
            bonus = M_MAX * NB_MOTS;                   //force l'affichage de la derniere ligne, meme si le bout graphique n'est pas atteint
            bonus2 = 0;
        }

        if ((somme == 0 && bonus >= longueur_ligne) || (somme + bonus >= longueur_ligne))
        {
            for (j = dep, l = 0; j < i; j++)
            {
                for (k = 0; tab_mot[j][k] != '\0'; k++) ligne[l++] = tab_mot[j][k];
                //ligne[l++] = ' ';         //double espace entre les mots ?
                if (j == tabulations[tabu_act])
                {
                    ligne[l] = '\0';

                    s1 = TTF_RenderText_Blended(font, ligne, couleur);
                    if (s1 == NULL)
                    {
                        fprintf(stderr, TTF_GetError());
                        return NULL;
                    }

                    nb_espace_supp = (ESPACE_TAB_MAX - (s1->w % ESPACE_TAB_MAX)) / largeur_espace;
                    SDL_FreeSurface(s1);
                    for (k = 0; k < nb_espace_supp; k++) ligne[l++] = ' ';
                    tabu_act++;
                }
            }
            if (l != 0) ligne[l] = '\0';
            else {
                ligne[0] = ' ';
                ligne[1] = '\0';
            }
            s1 = TTF_RenderText_Blended(font, ligne, couleur);
            if (dep == 0) result = s1;
            else
            {
                fusionner_surface(&result, s1, largeur_espace, COLLE_EN_BAS);      //l'espace entre deux mots est le meme qu'entre 2 lignes
                SDL_FreeSurface(s1);
            }

            dep = i;
            somme = 0;
        }

        somme += bonus2;
    }


    for (i = 0; i < n; i++)
    {
        if (tab_mot[i][0] != '\0') SDL_FreeSurface(tab_surface[i]);
        free(tab_mot[i]);
    }
    free(tab_mot);
    free(tab_surface);
    free(ligne);
    free(saut);
    free(tabulations);

    return result;

}

char is_in_rect(int x, int y, const SDL_Rect* rect)
{
    return (x >= rect->x && x < rect->x + rect->w && y >= rect->y && y < rect->y + rect->h);
}


int message(const char* texte, int taille, const char* chemin_police, Uint8 transparence, Uint32 boutons)
{
    /*
        affiche une boite de dialogue avec un message ecrit a l'interieur
        renvoie une valeur differente selon le bouton clique

        la boite de dialogue est automatiquement centree
        l'ecran est mis en arriere plan (il l'est d'autant plus masque que transparence est proche de 0)

        taille est la taille de la police de caracteres

        bouton est un masque des boutons pouvant etre affiches sous le message
        bouton peut prendre les valeurs suivantes, combinables avec des | :
            FLAG_OK -> bouton de validation -> renvoie BOUTON_OUI
            FLAG_NO -> croix rouge (refus)  -> renvoie BOUTON_NON

            si l'utilisateur ferme la fenetre, BOUTON FERMETURE est renvoye

        Note :
        les images des boutons correspondants contiennent 3 "sous images" (respectivement de gauche a droite):
            lorsque la souris est sur le bouton et que le bouton est actif
            lorsque la souris n'est pas sur le bouton et que le bouton est actif
            lorsque le bouton est inactif (ie, incliquable)

        les 3 sous images sont de la meme taille

    */

    Uint32 l_bouton[] = {BOUTON_OUI, BOUTON_NON, 0}, boutons_actifs[NB_MAX_BOUTONS];
    int somme, i, j, nb_boutons, select, nouveau_select, retour;
    SDL_Surface *screen, *copie, *img_texte, *l_img_bouton[NB_MAX_BOUTONS];
    SDL_Rect rect_box, rect_blit, l_rect[NB_MAX_BOUTONS];
    TTF_Font* police = TTF_OpenFont(chemin_police != NULL ? chemin_police : POLICE_PAR_DEFAUT, taille);
    SDL_Color couleur_fond = COULEUR_GRIS;
    SDL_Event ev;
    screen = SDL_GetVideoSurface();
    copie = masquer_fond(transparence);


    if (boutons == 0) boutons = BOUTON_OUI;

    img_texte = render_auto_intelligent(police, texte, COULEUR_NOIR, (int) TAILLE_BOX * COEFF_ECRITURE);
    l_img_bouton[0] = IMG_Load("valid.png");
    l_img_bouton[1] = IMG_Load("cancel.png");
    l_img_bouton[2] = NULL;
    //TODO gerer les erreurs d'ouvertures de fichier

    rect_box.w = TAILLE_BOX;
    rect_box.h = (img_texte->h + l_img_bouton[0]->h + SAUT_DE_LIGNE) / COEFF_ECRITURE;
    rect_box.x = (screen->w - rect_box.w) / 2;
    rect_box.y = (screen->h - rect_box.h) / 2;
    SDL_FillRect(screen, &rect_box, SDL_MapRGB(screen->format, couleur_fond.r, couleur_fond.g, couleur_fond.b));
    rect_blit.x = rect_box.x + (rect_box.w - img_texte->w) / 2;
    rect_blit.y = rect_box.y + rect_box.h  * (1 - COEFF_ECRITURE) / 2;
    SDL_BlitSurface(img_texte, NULL, screen, &rect_blit);


    for (somme = 0, i = 0; l_bouton[i]; i++) if (l_bouton[i] & boutons) somme += l_img_bouton[i]->w;
    somme /= 3;
    for (i = 0, j = 0; l_bouton[i]; i++)
    {
        if (l_bouton[i] & boutons)
        {
            rect_blit.x = 0; rect_blit.y = 0;
            rect_blit.w = l_img_bouton[j]->w / 3; rect_blit.h = l_img_bouton[j]->h;
            if (j == 0) l_rect[j].x = rect_box.x + (TAILLE_BOX - somme) / 2;
            else l_rect[j].x = l_rect[j - 1].x + l_img_bouton[j - 1]->w / 3;

            //l_rect[j].y = rect_box.y + rect_box.h * (.5 - COEFF_ECRITURE / 2) + l_img_bouton[j]->h / 2;
            l_rect[j].y = rect_box.y + rect_box.h * (1 - COEFF_ECRITURE) / 2 + img_texte->h + SAUT_DE_LIGNE;
            SDL_BlitSurface(l_img_bouton[j], &rect_blit, screen, &(l_rect[j]));

            boutons_actifs[j] = l_bouton[i];

            j++;
        }


    }
    nb_boutons = j;


    SDL_Flip(screen);


    select = -1;
    retour = -1;
    while (retour == -1)
    {
        SDL_WaitEvent(&ev);

        switch (ev.type)
        {
        case SDL_MOUSEMOTION:
            for (i = 0; i < nb_boutons; i++)
            {

                rect_blit = l_rect[i];

                if (is_in_rect(ev.motion.x, ev.motion.y, &(l_rect[i])))
                {
                    SDL_FillRect(screen, &(l_rect[i]), SDL_MapRGB(screen->format, couleur_fond.r, couleur_fond.g, couleur_fond.b));
                    rect_blit.x = l_img_bouton[i]->w / 3; rect_blit.y = 0;
                    nouveau_select = i;

                }

                else if (select == i)
                {
                    SDL_FillRect(screen, &(l_rect[i]), SDL_MapRGB(screen->format, couleur_fond.r, couleur_fond.g, couleur_fond.b));
                    rect_blit.x = 0; rect_blit.y = 0;
                    nouveau_select = -1;
                }
                else continue;

                SDL_BlitSurface(l_img_bouton[i], &rect_blit, screen, &(l_rect[i]));
                SDL_UpdateRect(screen, l_rect[i].x, l_rect[i].y, l_rect[i].w, l_rect[i].h);


            }

            select = nouveau_select;

            break;

        case SDL_MOUSEBUTTONUP:
            if (ev.button.button != SDL_BUTTON_LEFT) break;

            for (i = 0; i < nb_boutons; i++)if (is_in_rect(ev.button.x, ev.button.y, &(l_rect[i]))) retour = boutons_actifs[i];
            break;

        case SDL_QUIT:
            retour = BOUTON_FERMETURE;
            break;

        default:
            break;

        }
    }


    SDL_BlitSurface(copie, NULL, screen, NULL);
    SDL_Flip(screen);
    TTF_CloseFont(police);
    SDL_FreeSurface(img_texte);
    for (i = 0; l_bouton[i]; i++) SDL_FreeSurface(l_img_bouton[i]);
    return retour;
}

char existe_fichier(const char* name)
{

    FILE* fichier;


    errno = 0;
    fichier = fopen(name, "r");

    if (fichier == NULL){
        if (errno == ENOENT) return 0;  //'no such file or directory'
    }
    else fclose(fichier);

    return 1;

}
void trouver_nouveau_nom(char* nom)
{
    /*
        trouve un nom de fichier commencant par nom tel qu'il n'existe aucun fichier ainsi nomme dans le repertoire courant
        l'extension du fichier est conservee, et doit etre presente !!

        par exemple, si "png#1.txt" existe, nom pointera sur "png#2.txt" apres appel
        l'appelant doit garantir que nom peut etre agrandi de quelques octets au plus
    */

    int pos_point, pos_num, i;
    char digit[] = "0123456789", ext[20];


    for(pos_point = 0; nom[pos_point] != '.'; pos_point++);
    strcpy(ext, nom + pos_point);
    for(pos_num = pos_point - 1; pos_num >= 0 && memchr(digit, nom[pos_num], 10) != NULL; pos_num--);
    pos_num++;

    for (i = 1; existe_fichier(nom); i++) {
            sprintf(nom + pos_num, "%d%s", i, ext);
    }


}


int taille_fichier (const char * file_name)
{
    /*
        renvoie la taille du fichier, -1 en cas d'erreur
    */

    FILE * f;
    int t;

    f = fopen(file_name, "rb");

    if (f != NULL)
    {
        fseek(f, 0, SEEK_END); /* aller à la fin du fichier */
        t = ftell(f); /* lire l'offset de la position courante par rapport au début du fichier */
        fclose(f);

        return t;
    }

    return -1;
}

char print_from_file(const char* filename, SDL_Color couleur, TTF_Font* police, int x, int y, int longueur_ligne)
{
    /*
        affiche le contenu d'un fichier en position (x, y)
        longueur_ligne est le nombre de pixels au bout duquel la fonction revient a la ligne automatiquement

        renvoie 1 si tout se passe bien, 0 sinon
    */


    int i, T = taille_fichier(filename);
    char* txt = malloc(sizeof(char) * (T + 1));
    FILE* fichier = fopen(filename, "r");
    SDL_Surface* img, *screen;
    SDL_Rect rect;

    if (fichier == NULL) return 0;

    //vice : T est plus grand que la longueur du fichier, a cause des retours a la ligne
    //en effet, le .txt contient un retour chariot '\r' et un saut de ligne '\n' soit 2 caracteres
    //mais une fois en chaine de caracteres il n'y a que un '\n'


    fread(txt, sizeof(char), T, fichier);       //on risque de lire plus de caractere qu'en contient le fichier
    //#ifdef _WIN32           //sous windows, autant 32bits que 64
        for (i = 0; i < T; i++) {
            if (txt[i] == '\n') T--;
        }
    //#endif

    txt[T] = '\0';

    img = render_auto_intelligent(police, txt, couleur, longueur_ligne); //cette fonction semble ne pas faire le boulot correctement...
    rect.x = x; rect.y = y;
    screen = SDL_GetVideoSurface();
    SDL_BlitSurface(img, NULL, screen, &rect);
    SDL_UpdateRect(screen, rect.x, rect.y, rect.w, rect.h);

    fclose(fichier);
    free(txt);
    SDL_FreeSurface(img);

    return 1;


}
