/****************************************************************************/
/*  PGCK : Psp Game Creator Kit base sur NGCK de Lapintade                  */
/*  Porte par Antoine Letouzey                                              */
/*  ameboure@yahoo.fr                                                       */
/****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "SDL.h"
#include "SDL_image.h"
#include <zlib.h>

#include "psp_main.h"
#include "Pgck.h"
#include "Pson.h"


//donne a l'avance le nombre de sprite qui sont charges
#define nombreSprite 72


//----------------------------------------------------------------
// declaration prealable de certaine fonctions
//----------------------------------------------------------------
//void SetGlobalColor(SDL_Surface*, SDL_Color);
extern void InitialiseGame(void);
extern void GameLoop(void);

//----------------------------------------------------------------
// VARIABLES GLOBALES
//----------------------------------------------------------------


//declaration du fond et de l'ecran
SDL_Surface* SpriteFond    = NULL;
SDL_Surface* screen = NULL;

//declaration du tableau contenant les sprites
SDL_Surface* tabSprite[nombreSprite];

short int continuer = 1;
int shot = 0;
// vitesse de rafraichissement 1 => 60Hz
float vitesse=1.5;
int pretAquitter = 0;


// utile pour savoir si on doit relancer l'affichage ou attendre un peu
long time1, time2; 

// liste de booleens qui servent a connaitre l'etat d'une touche
touche tabTouche[12];

// couleur du texte
SDL_Color CoulTexte;

//position du joystic
//couple posJoy;



//------------------------------------------
//                FONCTIONS
//------------------------------------------


//----------------------------
// charge les path et les droit des niveaux
//----------------------------
ListeNiveaux chargeListeNiveau()
 {
  //FILE *out = fopen("./std.out", "a");
  //fprintf(out, "entree dans chargeListeNiveau\n");
  //fclose(out);
  //out = fopen("./std.out", "a");
  
  ListeNiveaux res;
  FILE *fic = fopen("./datas/niveaux.dat", "r");
  fscanf(fic, "%d\n", &res.nbLv);
  //fprintf(out, "nb de niveaux : %d\n", res.nbLv);
  //fclose(out);
  res.path = (char**) malloc(res.nbLv * sizeof(char*));
  res.bloque = (int*) malloc(res.nbLv * sizeof(int));
  res.apercut = (SDL_Surface**) malloc(res.nbLv * sizeof(SDL_Surface*));
  res.limite_sauves = (int*) malloc(res.nbLv * sizeof(int));
  res.limite_morts = (int*) malloc(res.nbLv * sizeof(int));
  res.pathApercut = (char**) malloc(res.nbLv * sizeof(char*));
  
  int i;
  for (i=0; i<res.nbLv; i++)
   {
     //out = fopen("./std.out", "a");
     // allocation de memoire
     char* lu = (char*) malloc(200*sizeof(char));
     // lecture du nom
     fgets(lu, 200, fic);
     lu[strlen(lu)-1] = '\0';
     res.path[i] = lu;
     //fprintf(out, "%s\n", res.path[i]);
     //fclose(out); out = fopen("./std.out", "a");

     // chargement de la miniature
     char* lu2 = (char*) malloc(200*sizeof(char));
     fgets(lu2, 200, fic);
     //fprintf(out, "chemin vers la miniature : >%s<\n", lu2);
     //fclose(out); out = fopen("./std.out", "a");
     lu2[strlen(lu2)-1] = '\0';
     res.pathApercut[i] = lu2;
     res.apercut[i] = IMG_Load(lu2);
     //fprintf(out, "%s -> %d\n", res.pathApercut[i], res.apercut[i] == NULL);
     //fclose(out); out = fopen("./std.out", "a");

     // est-il bloque ?
     fscanf(fic,"%d\n", &res.bloque[i]);
     //fprintf(out, "bloque : %d\n", res.bloque[i]);
     //fclose(out); out = fopen("./std.out", "a");

     // chargement des nb de morts et de sauves
     FILE *fic2 = fopen(lu, "r");
     if (fic2) fscanf(fic2, "%d\n%d\n", &res.limite_sauves[i], &res.limite_morts[i]);
     fclose(fic2);
     //fprintf(out, "a sauver : %d  a pas faire mourir : %d\n\n", res.limite_sauves[i], res.limite_morts[i]);
     //fclose(out);
   }
  fclose(fic);
  return res;
 }






//---------------------------------------------------------------------------------------------
// chargement des images des sprites 
//--------------------------------------------------------------------------------------------- 

SDL_Surface *Load_and_Display(const char *filename)
 {
   SDL_Surface *Surftemp1, *Surftemp2;     /*  Surfaces temporaires  */

   Surftemp1 = IMG_Load(filename); /* Charge l'image */
   Surftemp2 = SDL_DisplayFormat(Surftemp1);
   SDL_FreeSurface(Surftemp1);
     /* Renvoie l'image chargee*/
   return Surftemp2;
 }



//------------------------------------------
// donne le temps en milisecondes
//------------------------------------------
long PTime()
{
 return SDL_GetTicks();
}

//------------------------------------------
// fixe la vitesse du jeu
//------------------------------------------
void PVitesse(float v)
{
 vitesse = v;
}

//------------------------------------------
// change l'image de fond
//------------------------------------------
void PChangeFond(char* fic)
{
 if (SpriteFond != NULL) SDL_FreeSurface(SpriteFond);
 SpriteFond = IMG_Load(fic);
}

void PBloqueFond()
{
 SpriteFond = screen;
}




//---------------------------------
// fait avancer l'ecran de chargement
//---------------------------------
void progression(int pourcentage)
{
  SDL_Rect rect;
  rect.x = 235;
  rect.y = 110 - (pourcentage/2);
  SDL_BlitSurface(tabSprite[41], NULL, screen, &rect);
  rect.x = 0; rect.y = 0;
  SDL_BlitSurface(tabSprite[40], NULL, screen, &rect);
  SDL_Flip(screen);
}


//------------------------------------------------------
// chargement des elements du niveau a partir du fichier
//------------------------------------------------------
Niveau PChargeNiveau(char *fic, tabFenetres *fen)
{
 progression(0); //-------------------> 0%
 
 FILE *out = fopen("std.out", "a");
 FILE *fichier = fopen(fic,"r");
 char lu[200];
 Niveau n;
 // lecture du nb de perso a sauver
 fscanf(fichier, "%d\n", &n.limite_sauves);
 // lecture du nb de perso mort max
 fscanf(fichier, "%d\n", &n.limite_morts);
 
 progression(5); //-------------------> 5%

 // lecture des hp des perso
 fscanf(fichier, "%d\n", &n.hp_persos);
 // lecture des hp des feux
 fscanf(fichier, "%d\n", &n.hp_feux);
 // lecture de la position du pompier
 fscanf(fichier, "%d\n", &n.POS_Y);

 progression(10); //-------------------> 10%

 // lecture du vent
 fscanf(fichier, "%d\n", &n.vent);
 // lecture de la gravite
 float f;
 fscanf(fichier, "%f\n", &f);
 n.gravite = (double)f;

 progression(15); //-------------------> 15%

 // lecture de la positions des scores
 fscanf(fichier, "%d %d\n", &n.posSauves.x, &n.posSauves.y);
 fscanf(fichier, "%d %d\n", &n.posMorts.x, &n.posMorts.y);
 
 
 //---------------- fenetres
 fen->nbFen = -1;
 // lecture du nb de fenetres
 int tmp;
 fscanf(fichier,"%d\n",&tmp);
 fen->nbFen = tmp;
 fprintf(out,"trouve %d fenetres\n",fen->nbFen);

 progression(20); //-------------------> 20%

 // lecture des coords des fenetres
 int i;
 for(i=0; i<fen->nbFen; i++)
  {
   int x,y;
   fscanf(fichier,"%d %d\n",&x, &y);
   fen->fenetres[i].x = x;
   fen->fenetres[i].y = y;
   fen->fenetres[i].etat = OK;
   fprintf(out,"en (%3d,%3d)\n", fen->fenetres[i].x, fen->fenetres[i].y);

   progression(20+2*i); //-------------------> %

   //printf("nouvelle fenetre en %d %d\n",(*couple)[i].y, (*couple)[i].x);
  }

 progression(60); //-------------------> 60%

 // sprite perso
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[50] = IMG_Load(lu);
 fprintf(out,"perso -> '%s' (%d)\n",lu, tabSprite[50] == NULL);

 progression(65); //-------------------> 65%
 
 // sprite ange
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[17] = IMG_Load(lu);
 fprintf(out,"ange -> '%s' (%d)\n", lu ,tabSprite[17] == NULL);
 
 progression(70); //-------------------> 65%
 
 // perso echelle
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[51] = IMG_Load(lu);
 fprintf(out,"perso_echelle -> '%s' (%d)\n",lu ,tabSprite[51] == NULL);
 
 progression(75); //-------------------> 65%
 
 // fond des fenetres
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[16] = IMG_Load(lu);
 fprintf(out,"fond -> '%s' (%d)\n",lu ,SpriteFond == NULL);
 
 progression(80); //-------------------> 65%
 
 // mur
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 if (tabSprite[36] != NULL) SDL_FreeSurface(tabSprite[36]);
 tabSprite[36] = IMG_Load(lu);
 fprintf(out,"mur -> '%s' (%d)\n",lu ,tabSprite[36] == NULL);
 
 progression(85); //-------------------> 65%
 
 // help
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[37] = IMG_Load(lu);
 fprintf(out,"help -> '%s' (%d)\n",lu ,tabSprite[37] == NULL);
 
 progression(90); //-------------------> 65%
 
 // thanx
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[38] = IMG_Load(lu);
 fprintf(out,"thx -> '%s' (%d)\n",lu ,tabSprite[38] == NULL);
 
 progression(95); //-------------------> 65%
 
 // safe
 fgets(lu, 200, fichier);
 lu[strlen(lu)-1] = '\0';
 tabSprite[39] = IMG_Load(lu);
 fprintf(out,"safe -> '%s' (%d)\n",lu ,tabSprite[39] == NULL);
 fclose(out);
 fclose(fichier);
 
 SpriteFond = IMG_Load("datas/static/vide.png");
 progression(100); //-------------------> 65%
 
 return n;
}

//------------------------------------
// quitte le jeu
//------------------------------------
void Quit()
 {
  continuer = 0;
 }


// -------------------------------------------------------------------
// affiche une image  en (x,y)
// -------------------------------------------------------------------

void PPrintImage(int x, int y, int numSprite)
{
  SDL_Rect rect;
  rect.x = x; rect.y = y;
  SDL_BlitSurface(tabSprite[numSprite], NULL, screen, &rect);
}


//--------------------------------------------------------------------
// affiche la miniature d'un niveau
//--------------------------------------------------------------------
void PPrintMiniNiveau(int x, int y, SDL_Surface *mini, int bloque, int infos, int nbS, int nbM)
{
 SDL_Rect rect;
 rect.x = x; rect.y = y;
 if (bloque) SDL_BlitSurface(tabSprite[71], NULL, screen, &rect);
 else
  {
   SDL_BlitSurface(mini, NULL, screen, &rect);
   if (infos)
    {
     //rect.x = x-3; rect.y = y-3;
     SDL_BlitSurface(tabSprite[10], NULL, screen, &rect);
     PPrintNb(x+33, y+80, nbS);
     PPrintNb(x+100, y+80, nbM);
    }
  }
}

//--------------------------------------------------------------------
// affiche un nombre (recursif)
//--------------------------------------------------------------------
void PPrintNb(int x, int y, int v)
 {
  if (v<0) v = 0;
  int unite = v%10;
  SDL_Rect rect;
  rect.x = x; rect.y = y;
  SDL_BlitSurface(tabSprite[unite], NULL, screen, &rect);
  if(v/10) PPrintNb(x-15, y, v /10);
 }

void PPrintScore(int x, int y, unsigned long v)
 {
  if (v<0) v = 0;
  int unite = v%10;
  SDL_Rect rect;
  rect.x = x; rect.y = y;
  SDL_BlitSurface(tabSprite[unite], NULL, screen, &rect);
  if(v/10) PPrintNb(x-15, y, v /10);
 }

void PPrintCode(int x, int y, char* code)
{
 if ((*code) == 0) return;
 char c = code[strlen(code)-1];
 int unite = c - '0';
 code[strlen(code)-1] = '\0';
 SDL_Rect rect;
 rect.x = x; rect.y = y;
 SDL_BlitSurface(tabSprite[unite], NULL, screen, &rect);
 PPrintCode(x-15, y, code);
 }

void PPrintFond()
 {
  SDL_Rect rect;
  rect.x = 0; rect.y = 0;
  SDL_BlitSurface(SpriteFond, NULL, screen, &rect);
 }


/*

// -------------------------------------------------------------------
// pour ecrire une chaine de caractere
//le couple (x,y) donne les coordonees en pixel du coin superieur gauche de la 1ere lettre de la chaine
//ch est un tableau de char contenant la chaine
//taille est la taille de la police
// -------------------------------------------------------------------

void PPrintMot(int x, int y, char *ch)
{
    // Ecriture du texte dans la SDL_Surface "texte" en mode Blended (optimal)
    SDL_Surface *texte = TTF_RenderUTF8_Blended(police, ch, CoulTexte);
    SDL_Rect rect;
    rect.x = x; rect.y = y;
    SDL_BlitSurface(texte, NULL, screen, &rect);
    SDL_FreeSurface(texte);
}
*/

//------------------------------------------------------------------------
// modifie la couleur du texte a partir de 3 composantes (RVB)
//------------------------------------------------------------------------
void PSetCouleurTexte(int R, int V, int B)
{
 CoulTexte.r=R; 
 CoulTexte.g=V;
 CoulTexte.b=B;
}


// -------------------------------------------------------------------
// vide la fenetre en remplissant le fond
// -------------------------------------------------------------------
void EffaceEcran2()
{
 SDL_Rect rect;
 rect.x = rect.y = 0;
 SDL_BlitSurface(SpriteFond, NULL, screen, &rect);
}


// -------------------------------------------------------------------------------------------
// Permet de bloquer une touche ou bouton de la souris  -> desactive le fait de garder appuyee
// -------------------------------------------------------------------------------------------
void PBloqueTouche(int t)
{
 if (tabTouche[t].etat) tabTouche[t].libre = 0;
}
// -------------------------------------------------------------------
// Bloque toutes les commandes d'un coup
// -------------------------------------------------------------------
void PEffaceTouches()
{
 int i;
 for(i=Circle; i<R; i++) if (tabTouche[i].etat) tabTouche[i].libre = 0;
}

// -------------------------------------------------------------------
// Mise a jour des "valeurs" des touches.
// -------------------------------------------------------------------

void demanderQuitter(void);

void UpdateCommandes()
{
 SceCtrlData pad;
 sceCtrlReadBufferPositive(&pad, 1);
 if (pad.Buttons & PSP_CTRL_CIRCLE) tabTouche[Circle].etat = 1;
 else { tabTouche[Circle].etat = 0; tabTouche[Circle].libre = 1; }
 
 if (pad.Buttons & PSP_CTRL_TRIANGLE) tabTouche[Triangle].etat = 1;
 else { tabTouche[Triangle].etat = 0; tabTouche[Triangle].libre = 1; }

 if (pad.Buttons & PSP_CTRL_SQUARE) tabTouche[Square].etat = 1;
 else { tabTouche[Square].etat = 0; tabTouche[Square].libre = 1; }

 if (pad.Buttons & PSP_CTRL_CROSS) tabTouche[Cross].etat = 1;
 else { tabTouche[Cross].etat = 0; tabTouche[Cross].libre = 1; }

 if (pad.Buttons & PSP_CTRL_START) tabTouche[Start].etat = 1;
 else { tabTouche[Start].etat = 0; tabTouche[Start].libre = 1; }

 if (pad.Buttons & PSP_CTRL_SELECT) tabTouche[Select].etat = 1;
 else { tabTouche[Select].etat = 0; tabTouche[Select].libre = 1; }

 if (pad.Buttons & PSP_CTRL_RIGHT) tabTouche[Right].etat = 1;
 else { tabTouche[Right].etat = 0; tabTouche[Right].libre = 1; }

 if (pad.Buttons & PSP_CTRL_UP) tabTouche[Up].etat = 1;
 else { tabTouche[Up].etat = 0; tabTouche[Up].libre = 1; }

 if (pad.Buttons & PSP_CTRL_LEFT) tabTouche[Left].etat  = 1;
 else { tabTouche[Left].etat = 0; tabTouche[Left].libre = 1; }

 if (pad.Buttons & PSP_CTRL_DOWN) tabTouche[Down].etat  = 1;
 else { tabTouche[Down].etat = 0; tabTouche[Down].libre = 1; }

 if (pad.Buttons & PSP_CTRL_LTRIGGER) tabTouche[L].etat = 1;
 else { tabTouche[L].etat = 0; tabTouche[L].libre = 1; }

 if (pad.Buttons & PSP_CTRL_RTRIGGER) tabTouche[R].etat = 1;
 else { tabTouche[R].etat = 0; tabTouche[R].libre = 1; }
 
 if ((pad.Buttons & PSP_CTRL_HOME) && !pretAquitter) demanderQuitter();
}

//--------------------------------------------------------------------
// Les fonctions suivantes permettent de raporter l'etat d'une touche
//--------------------------------------------------------------------

int PGet(int t)
 {
  return (tabTouche[t].etat && tabTouche[t].libre);
 }

// -------------------------------------------------------------------
couple PGetJoy()
{
 couple posJoy;
 SceCtrlData pad;
 sceCtrlReadBufferPositive(&pad, 1);
 posJoy.x = pad.Lx-128;
 posJoy.y = pad.Ly-128;
 return posJoy;
}



void demanderQuitter()
{
 //-- coupe le son
 int oldVolume = PGetVolume();
 PSetVolume(1);
 SDL_Surface tmpQuitter;
 pretAquitter = 1;
 tmpQuitter = (*screen);
 SDL_Rect r;
 r.x = 0; r.y = 0;
 SDL_BlitSurface(tabSprite[68], NULL, screen, &r);
 SDL_Flip(screen);
 while(1)
  {
   UpdateCommandes();
   if (PGet(Cross)) { continuer = 0; PBloqueTouche(Cross); break; }
   if (PGet(Triangle))
    {
     PBloqueTouche(Triangle);
     (*screen) = tmpQuitter;
     pretAquitter = 0;
     PSetVolume(oldVolume);
     break;
    }
  }
}





//-------------------------------------------------------------------------------------------- 
// Initialisation du moteur
//--------------------------------------------------------------------------------------------
void PGCKInit()
{
  time1 = time2 = SDL_GetTicks();
  // par defaut on active l'affichage de la souris
  // initialisation des sprites
  SpriteFond   = IMG_Load("datas/static/menu.png");
  int i = 0;
  tabSprite[i++] = IMG_Load("datas/static/chiffres/00.png");//--0
  tabSprite[i++] = IMG_Load("datas/static/chiffres/01.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/02.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/03.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/04.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/05.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/06.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/07.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/08.png");
  tabSprite[i++] = IMG_Load("datas/static/chiffres/09.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/miniselect.png");//----10
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/allumette1.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/allumette2.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/allumette3.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/allumette4.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/allumette5.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/arriere.png");//NULL;// IMG_Load("datas/static/menus/allumette6.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/angel.png");
  tabSprite[i++] = IMG_Load("datas/static/credits/facade.png");
  tabSprite[i++] = IMG_Load("datas/static/echelle.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte1.png");//----20
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte2.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte3.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte4.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte5.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte6.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte7.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte8.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte9.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/goutte10.png");
  tabSprite[i++] = IMG_Load("datas/static/feux/feu1.png");// ----30
  tabSprite[i++] = IMG_Load("datas/static/feux/feu2.png");
  tabSprite[i++] = IMG_Load("datas/static/feux/feu3.png");
  tabSprite[i++] = IMG_Load("datas/static/feux/feu4.png");
  tabSprite[i++] = IMG_Load("datas/static/feux/feu5.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/gagne.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/immeuble.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/help.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/thanx.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/safe.png");
  tabSprite[i++] = IMG_Load("datas/static/loading/load.png");//----40
  tabSprite[i++] = IMG_Load("datas/static/loading/progression.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/menu1.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/menu2.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/menu3.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/menu4.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/niveaumenu.png");
  tabSprite[i++] = NULL;// IMG_Load("datas/static/menus/objectif.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/pause2.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/perdu.png");
  tabSprite[i++] = IMG_Load("datas/immeuble/perso.png");//------50
  tabSprite[i++] = IMG_Load("datas/immeuble/perso_echelle.png");
  tabSprite[i++] = NULL;//IMG_Load("datas/static/menus/transition.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes1.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes2.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes3.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes4.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes5.png");
  tabSprite[i++] = IMG_Load("datas/static/gouttes/sgouttes6.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee01.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee02.png"); //--------60
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee03.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee04.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee05.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee06.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee07.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee08.png");
  tabSprite[i++] = IMG_Load("datas/static/fumees/fumee09.png");
  tabSprite[i++] = IMG_Load("datas/static/menus/quitter.png");
  tabSprite[i++] = IMG_Load("datas/static/credits/texte.png");
  tabSprite[i++] = NULL; //IMG_Load("datas/static/menus/noir.png"); //---- 70
  tabSprite[i++] = IMG_Load("datas/static/lock.png"); //---- 71
  
  //-- verif de l'etat des sprites charges
  FILE *fic=fopen("std.out","w");
  int j;
  for (j=0; j<i; j++) fprintf(fic, "etat du sprite n°%d : %d\n", j, tabSprite[j] == NULL);
  fclose(fic);

  // charge la liste des niveaux
  listeNiveaux = chargeListeNiveau();
 
  
  // initialisation de l'etat des touches
  for (i=Circle; i<R; i++) {tabTouche[i].etat = 0; tabTouche[i].libre = 1; }
  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
  // initialise la couleur du texte en rouge.
  PSetCouleurTexte(220,10,10);
  // initialisation de la partie son.
  PInitSon();
}


// ---------------------------------------------------------------------------------------------
void PGCKGameLoop()
{
  time1 = SDL_GetTicks();
  
  // vitesse=1 => le jeu tourne a 60 fps 
  if ((float)(time1-time2)<16.0*vitesse)
  {
    UpdateCommandes();
    return;
   }

  time2=time1;
  EffaceEcran2();
  UpdateCommandes();
  GameLoop();
  return;
}


// ---------------------------------------------------------------------------------------------
void PGCKUnInit()
{
  int i;
  // liberation de l'espace alloue pour les images
  //if (SpriteFond != NULL) SDL_FreeSurface(SpriteFond);
  for (i=0;i<nombreSprite;i++) if (tabSprite[i] != NULL) SDL_FreeSurface(tabSprite[i]);
}


//------- prog principal


int PGCKmain(int argc, char *argv[])
{
 //-- initialise le Random
 srand(time(0));
 //TTF_Init();
 /* Initialise SDL */
 if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0 ) {
	printErr("Couldn't initialize SDL: %s\n",SDL_GetError());
	exit(1);
 }
 atexit(SDL_Quit);
 screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

 //---------affichage du logo pendant le chargement des images (ou au moins 4 secondes)
 long t1 = SDL_GetTicks();
 SDL_Surface *logo = IMG_Load("datas/logo.png");
 SDL_Rect rect;
 rect.x = 0; rect.y = 0;
 SDL_BlitSurface(logo, NULL, screen, &rect);
 SDL_Flip(screen);
 
 PGCKInit();
 while(SDL_GetTicks() - t1 < 4000) ;// secondes au moins
 SDL_FreeSurface(logo);
 // chargemnt fini

 InitialiseGame();
 while (continuer) {
          PGCKGameLoop();
          SDL_Flip(screen);
          if (PGet(L) && PGet(R) && PGet(Select)) // on prend un screen
           {
            char path[50];
            sprintf(path,"datas/shots/shot--%d.bmp", shot);
            shot++;
            SDL_SaveBMP(screen, path);
           }
 }
 // fin du jeu liberation de la memoire allouee
 PGCKUnInit();
 //FIN du programme
 return 0;
}
