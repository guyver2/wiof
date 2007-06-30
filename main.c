#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Pgck/psp_main.h"
#include "Pgck/Pgck.h"
#include "Pgck/Pson.h"

#include "struct.h"
#include "codage.h"


#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>


//--------------------------   TODO
// les son -> chercher [son]
// enlever les screen shots -> Pgck.c tt en bas
//---------------------------------




//############################### VARIABLES GLOBALES  ######################

enum {ETAT_MENU, ETAT_CREDIT, ETAT_CHOIXLV, ETAT_JEU, ETAT_PAUSE, ETAT_GAGNE,
      ETAT_PERDU, ETAT_SCORE, ETAT_SURVIVAL, ETAT_FINI};

tabFenetres fenetres;
tabFeux feux;
tabPersonnages personnages;
tabGouttes gouttes;
tabMessages messages;
tabEclaboussures eclaboussures;
tabFumees fumees;
couple echelle;
Niveau niveau;

int nbSauves = 0;
int nbMorts = 0;
int cpt = 1;
int FREQUENCE = 310;
int etat = ETAT_MENU;
unsigned long bestScore = 0;
unsigned long points = 0;

char chaineCode[50];

void new_fumee(int x, int y);
void InitialiseGame(void);
void GameLoop(void);
//################################  FONCTIONS  #############################


//----------------------------
// sauve les path et les droits des niveaux
//----------------------------
void sauveListeNiveau()
 {
  FILE *fic = fopen("./datas/niveaux.dat", "w");
  fprintf(fic, "%d\n", listeNiveaux.nbLv);
  int i;
  for (i=0; i<listeNiveaux.nbLv; i++)
   {
    fprintf(fic, "%s\n%s\n%d\n", listeNiveaux.path[i], listeNiveaux.pathApercut[i], listeNiveaux.bloque[i]);
   }
  fclose(fic);
 }


//---------------------------
// charge le meilleur score
//---------------------------
void chargeScore()
{
 FILE *fic;
 if ((fic = fopen("./datas/score.dat", "r")) == NULL) { bestScore = 0; return; }
 char score[50] = "";
 strcpy(chaineCode, score);
 fscanf(fic, "%s", score);
 fclose(fic);
 bestScore = decodage(score, 0,0);
}

//--------------------------
// sauve le score
//--------------------------
void sauveScore()
{
 char code[50] = "";
 FILE *fic = fopen("./datas/score.dat", "w");
 codage(code,bestScore,0);
 strcpy(chaineCode, code);
 fprintf(fic,"%s\n", code);
 fclose(fic);
}


//--------------------
// random entre a et b-1
//--------------------
int monRand(int a, int b)
 {
  if (a > b) return 0;
  return a + (rand()%(b-a));
 }

//-----------------------
// donne l'ange du Joystick
//---------------------
float get_angle()
 {
  couple c = PGetJoy();
  int x = c.x;
  int y = c.y;
  //---- calcul
  if (x < 0)
   {
    float a = asin(y/128.0);
    return -(M_PI - a);
   }
  else return 4.0;
 }


//------------------------  AFFICHAGE

//--------------------
// affiche un feu
//--------------------
void print_feu(feu *f)
 {
  int sprite;
  // definie le numero du sprite a afficher
  sprite = 30+f->compteur/4;
  f->compteur++;
  int decalage = 20 - ((double)f->hp / (double)niveau.hp_feux) * 20.0;
  if (!(f->compteur % 5))
   {
    f->hp++;
    if (f->compteur%3) new_fumee(fenetres.fenetres[f->idFen].x-TAILLE, fenetres.fenetres[f->idFen].y-TAILLE*3 + decalage);
    if (f->hp > niveau.hp_feux) f->hp = niveau.hp_feux;
   }
  f->compteur %= 20;
  PPrintImage(fenetres.fenetres[f->idFen].x - TAILLEFEU / 2, fenetres.fenetres[f->idFen].y - TAILLEFEU + 10 + decalage, sprite);
 }


//--------------------
// affiche une fumee
//--------------------
void print_fumee(fumee *f, int id)
 {
  int sprite = 59 + f->t/3;
  f->t++;
  f->y -= 3;
  if (!(f->t % 2)) f->x += niveau.vent;
  PPrintImage(f->x-(f->t/3) * 2, f->y, sprite);

  if (f->t > 26 || f->y < -20 || f->x > 480 || f->x < 0) {
   fumees.nbFumees--;
   fumees.fumees[id] = fumees.fumees[fumees.nbFumees];
  }
 }



//--------------------
// affiche un perso
//--------------------
// sur l'echelle
void print_perso(perso p)
 {
  if (p.etat == 1)
   {
    int sprite = 51;
    PPrintImage(p.x - 8, p.y - 12, sprite);
   }
 }

// dans une fenetre
void print_perso2(perso p)
 {
  if (p.etat == 0 && p.hp > 0)
   {
    int sprite = 50;
    PPrintImage(fenetres.fenetres[p.idFen].x - TAILLEPERSO / 2, fenetres.fenetres[p.idFen].y - TAILLEPERSO + 17, sprite);
   }
 }

// ange
void print_perso3(perso p)
 {
  if (p.etat == 2)
   {
    int sprite = 17;
    PPrintImage(fenetres.fenetres[p.idFen].x - TAILLEPERSO / 2, p.y, sprite);
   }
 }


//--------------------
// affiche une goutte
//--------------------
void print_goutte(goutte g)
 {
  int majoration = g.t/4;
  if (majoration>9) majoration = 9; // on reste sur la plus grosse goutte
  int sprite = 20+majoration;
  PPrintImage(g.x, g.y, sprite);
 }

//--------------------
// affiche un message
//--------------------
void print_message(int id, message *m)
 {
  PPrintImage(m->x-60, m->y-12, 37+m->typ);
  m->t--;
  // supprime le message
  if (m->t <= 0)
   {
    messages.nbMess--;
    messages.messages[id] = messages.messages[messages.nbMess];
   }
 }

//-------------------------
// affiche une eclaboussure
//-------------------------
void print_eclaboussure(int id, eclaboussure *e)
 {
  int sprite = 53+(e->t);
  fenetre f = fenetres.fenetres[e->idFen];
  PPrintImage(f.x - 50, f.y - 55, sprite);
  e->t++;
  // supprime l'eclaboussure
  if (e->t > 5)
   {
    eclaboussures.nbEcla--;
    eclaboussures.eclaboussures[id] = eclaboussures.eclaboussures[eclaboussures.nbEcla];
   }
 }


//--------------------- AJOUT

//---------------------
// ajoute une eclaboussure
//---------------------
void new_eclaboussure(int f)
 {
  int i;
  for (i=0; i< eclaboussures.nbEcla; i++)
   {
    if (eclaboussures.eclaboussures[i].idFen == f && eclaboussures.eclaboussures[i].t < 3) return;
   }
  eclaboussure e;
  e.idFen = f;
  e.t = 0;
  eclaboussures.eclaboussures[eclaboussures.nbEcla] = e;
  eclaboussures.nbEcla++;
 }


//--------------------
// ajoute un message
//--------------------
void new_message(int x, int y, int t)
 {
  message m;
  m.x = x;
  m.y = y;
  m.typ = t;
  m.t = 30;
  messages.messages[messages.nbMess] = m;
  messages.nbMess++;
 }

//--------------------
// ajoute une fumee
//--------------------
void new_fumee(int x, int y)
 {
  fumee f;
  f.x = x;
  f.y = y;
  f.t = 0;
  fumees.fumees[fumees.nbFumees] = f;
  fumees.nbFumees++;
 }


//--------------------
// ajoute un feu
//--------------------
void new_feu(int f ,int hp)
 {
  if (fenetres.fenetres[f].etat == BRULE || fenetres.fenetres[f].etat == PERSO_BRULE) return;
  if (fenetres.fenetres[f].etat == PERSO)
   {
    PPlaySound(0, 4);
    fenetres.fenetres[f].etat = PERSO_BRULE;
    new_message(fenetres.fenetres[f].x + TAILLE/2, fenetres.fenetres[f].y -(TAILLE +20), 0);
   }
  else fenetres.fenetres[f].etat = BRULE;
  // construit le nouveau feu
  feu nouvFeu;
  nouvFeu.idFen = f;
  nouvFeu.hp = hp;
  nouvFeu.compteur = 0;
  feux.feux[feux.nbFeux] = nouvFeu;
  feux.nbFeux++;
 }


//-----------------------
// ajoute un perso (on sppose que la fenetre ne brule pas)
//-----------------------
void new_perso(int f, int hp)
 {
  fenetres.fenetres[f].etat += PERSO;
  perso p;
  p.idFen = f;
  p.hp = hp;
  p.etat = 0;
  p.x = 0;
  p.y = 0;
  personnages.personnages[personnages.nbPersos] = p;
  personnages.nbPersos++;
 }

//-----------------------
// ajoute une goutte
//-----------------------
void new_goutte(float v, int hp)
 {
  float alpha = get_angle();
  if (alpha != 4.0)
   {
    alpha += 0.3 - (float) monRand(0,6) / 35.0;
    goutte g;
    g.x = g.xInit = POS_X;
    g.y = g.yInit = niveau.POS_Y;
    g.V0_sin = v*(sin(alpha));
    g.V0_cos = v*(cos(alpha));
    g.t = 0;
    g.hp = hp;
    gouttes.gouttes[gouttes.nbGouttes] = g;
    gouttes.nbGouttes++;
   }
 }

//----------------
// affichage complet
//----------------
void affichage()
 {
  int i;
  for (i=0; i< fenetres.nbFen; i++) PPrintImage(fenetres.fenetres[i].x-TAILLE, fenetres.fenetres[i].y-(38), 16);;
  for (i=0; i< feux.nbFeux; i++) print_feu(&feux.feux[i]);
  PPrintImage(0, 0, 36); // affiche le mur
  for (i=0; i< personnages.nbPersos; i++) print_perso2(personnages.personnages[i]);
  for (i=0; i< fumees.nbFumees; i++) print_fumee(&fumees.fumees[i], i);
  for (i=0; i< eclaboussures.nbEcla; i++) print_eclaboussure(i, &eclaboussures.eclaboussures[i]);
  PPrintImage(echelle.x, echelle.y, 19); // affichage de l'echelle
  for (i=0; i< personnages.nbPersos; i++) print_perso(personnages.personnages[i]);
  for (i=0; i< personnages.nbPersos; i++) print_perso3(personnages.personnages[i]);
  for (i=0; i< messages.nbMess; i++) print_message(i, &messages.messages[i]);
  PPrintNb(niveau.posSauves.x, niveau.posSauves.y, niveau.limite_sauves - nbSauves);
  PPrintNb(niveau.posMorts.x, niveau.posMorts.y, niveau.limite_morts - nbMorts);
  for (i=0; i< gouttes.nbGouttes; i++) print_goutte(gouttes.gouttes[i]);
 }





//-----------------------------  JEU

//------------------
// fait bouger les gouttes
//------------------
void bougeGouttes()
 {
  int i;
  for (i=0; i< gouttes.nbGouttes; i++)
   {
    goutte *g = &gouttes.gouttes[i];
    g->t++;
    g->V0_cos += (float)niveau.vent/30.; // effet du vent
    g->x = g->V0_cos * g->t + g->xInit;
    g->y = -(0.5 * - niveau.gravite * g->t * g->t + g->V0_sin * g->t) + g->yInit;
    // verif que la goutte sort pas
    if (g->x < -20 || g->x > 470 || g->y < -40 || g->y >272)
     { // dehors
      gouttes.nbGouttes--;
      gouttes.gouttes[i] = gouttes.gouttes[gouttes.nbGouttes];
      i--;
     }
   }
 }

//--------------------------------
// gere les interraction gouttes / feux
//--------------------------------
void eteint()
 {
  int i,j;
  for (i=0; i< gouttes.nbGouttes; i++)
   {
    goutte *g = &gouttes.gouttes[i];
    for (j=0; j< feux.nbFeux; j++)
     {
      feu *f = &feux.feux[j];
      int xf = fenetres.fenetres[f->idFen].x;
      int yf = fenetres.fenetres[f->idFen].y;
      if ((g->x+3 > xf - TAILLEFEU/2) && (g->x-3 < xf + TAILLEFEU/2) && (g->y+3 > yf - TAILLEFEU) && (g->y-3 < yf))
       {// sur le feu
        new_eclaboussure(f->idFen); // ajoute une eclaboussure
        f->hp -= g->hp;
        if (f->hp <= 0)
         {
          if(fenetres.fenetres[f->idFen].etat == BRULE) fenetres.fenetres[f->idFen].etat = OK;
          else
           { // il y avait qq'un
            fenetres.fenetres[f->idFen].etat = PERSO;
            new_message(fenetres.fenetres[f->idFen].x + TAILLE/2, fenetres.fenetres[f->idFen].y - (TAILLE +20) , 1);
           }//fin du else
          // suppression du feu
          feux.nbFeux--;
          feux.feux[j] = feux.feux[feux.nbFeux];
          j--;
         }
        // suppression de la goutte
        gouttes.nbGouttes--;
        gouttes.gouttes[i] = gouttes.gouttes[gouttes.nbGouttes];
        i--;
        break; // la goutte a disparu, pas la peine de verifier les autres feux
       } // fin de la goutte sur le feu
     } // fin du for gouttes
   } // fin du for feux
 }

//------------------------
// fait bruller les persos
//------------------------
void persoBrule()
 {
  int i;
  for (i=0; i< personnages.nbPersos; i++)
   {
    perso *p = &personnages.personnages[i];
    if (fenetres.fenetres[p->idFen].etat == PERSO_BRULE)
     { // sa fenetre brule
      p->hp--;
      if (p->hp <=0)
       {//perso mort
        PPlaySound(2, 3);
        p->etat = 2;
        nbMorts++;
        p->y = fenetres.fenetres[p->idFen].y - TAILLEPERSO;
        fenetres.fenetres[p->idFen].etat = BRULE;
       }
     }
   }
 }


//------------------
// bouge les anges
//------------------
void bougeAnge()
 {
  int i;
  for (i=0; i< personnages.nbPersos; i++)
   {
    if (personnages.personnages[i].etat != 2) continue; // passe direct au siuvant
    perso *p = &personnages.personnages[i];
    p->y -=2;
    if(p->y < -30)
     { //hors ecran
      personnages.nbPersos--;
      personnages.personnages[i] = personnages.personnages[personnages.nbPersos];
      i--;
     }
   }
 }

//-----------------------------
// donne le numero de la fenetre sous laquelle est l'echelle (-1 si aucune)
//-----------------------------
int positionEchelle()
 {
  int res = -1;
  int i;
  for(i=0; i<fenetres.nbFen; i++)
   {
    fenetre f = fenetres.fenetres[i];
    if (echelle.x > f.x -20 && echelle.x < f.x-2 && echelle.y > f.y -12 && echelle.y < f.y) return i;
   }
  return res;
 }


//-----------------------------
//fait apparaitre des feux dans les fenetres
//-----------------------------
void ajout_feu()
 {
  int i;
  for(i=0; i<fenetres.nbFen; i++)
   {
    if (fenetres.fenetres[i].etat == OK || fenetres.fenetres[i].etat == PERSO)
     { //fenetre sans feu
      int prob = monRand(0,FREQUENCE);
      if (prob == 1) new_feu(i,niveau.hp_feux);
     }
   }
 }

//-----------------------------
// fait apparaitre des persos
//-----------------------------
void ajout_perso()
 {
  int i;
  for(i=0; i<fenetres.nbFen; i++)
   {
    fenetre f = fenetres.fenetres[i];
    int ex = echelle.x;
    int ey = echelle.y;
    if (f.etat == OK && (ex < f.x-24 || ex > f.x+2) && (ey < f.y-22 || ey > f.y+4))
     { //fenetre sans feu et pas sur l'echelle
      int prob = monRand(0,FREQUENCE);
      if (prob == 1) new_perso(i,niveau.hp_persos);
     }
   }
 }


//------------------------------
// sauve les personnages
//------------------------------
void sauvePerso()
 {
  int i;
  int e = positionEchelle();
  for(i=0; i<personnages.nbPersos; i++)
   {
    if (e == personnages.personnages[i].idFen && fenetres.fenetres[e].etat == PERSO)
     { //perso sauvable
      fenetres.fenetres[e].etat = OK;
      personnages.personnages[i].etat = 1;
      personnages.personnages[i].x = echelle.x;
      personnages.personnages[i].y = echelle.y-4;
      nbSauves++;
      return; // un seul a la fois maxi
     }
   }
 }


//------------------------------------
// fait bouger l'echelle
//------------------------------------
void bougeEchelle()
 {
  int i;
  int decx = 0;
  int decy = 0;
  int vitesse = 3;
  if (PGet(Square) && echelle.x > 190)
   {
    echelle.x -= vitesse;
    decx -= vitesse;
   }
  if (PGet(Circle) && echelle.x < 470)
   {
    echelle.x += vitesse;
    decx += vitesse;
   }
  if (PGet(Triangle) && echelle.y > 0)
   {
    echelle.y -= vitesse;
    decy -= vitesse;
   }
  if (PGet(Cross) && echelle.y < 272)
   {
    echelle.y += vitesse;
    decy += vitesse;
   }
 //repercute le mvt sur les perso sur l'echelle
 for(i=0; i<personnages.nbPersos; i++)
 {
  if (personnages.personnages[i].etat == 1)
   {
    personnages.personnages[i].x +=decx;
    personnages.personnages[i].y += decy + 4;
    if (personnages.personnages[i].y > 272)
     { //perso a virer
      new_message(personnages.personnages[i].x, personnages.personnages[i].y-30,2);
      personnages.nbPersos--;
      personnages.personnages[i] = personnages.personnages[personnages.nbPersos];
      i--;
     }
   }
 }
}



//######################## DIFFERENTS ETATS DU JEU ######################




//--- --------------------------------menu du jeu



//---------- des gouttes dans les menus
void new_goutte2(float v, float alpha, int xDep, int yDep)
{
 goutte g;
 alpha += 0.2 - (float) monRand(0,4) / 35.0;
 g.x = g.xInit = xDep;
 g.y = g.yInit = yDep;
 g.V0_sin = v*(sin(alpha));
 g.V0_cos = v*(cos(alpha));
 if (g.V0_cos < 0.1 && g.V0_cos > -0.1) g.V0_cos = 0.0;
 g.t = 0;
 g.hp = 14;
 gouttes.gouttes[gouttes.nbGouttes] = g;
 gouttes.nbGouttes++;
}


int idMenu = 0;
int cptMenu = 0;
int bonusMenu = 0;
void initMenus()
 {
  PVitesse(0.0);
  chargeScore();
  cptMenu = 0;
  idMenu = 0;
  gouttes.nbGouttes = 0;
  niveau.vent = 0;
  niveau.gravite = -0.32;
  PChangeFond("datas/static/menus/menu.png");
 }


void menus()
 {
  if (PGet(R)) bonusMenu = 1;
  if (PGet(L)) bonusMenu = 0;
  float angle;
  cptMenu++;
  cptMenu %= 2;
  if (PGet(Cross))
   {
    PPlaySound(7, 1);
    PBloqueTouche(Cross);
    if (idMenu == 0)
     {
      etat = ETAT_CHOIXLV;
      InitialiseGame();
      return;
     }
    if (idMenu == 1)
     {
      PBloqueTouche(Cross);
      etat = ETAT_SURVIVAL;
      InitialiseGame();
      return;
     }
    if (idMenu == 2)
     {
      PBloqueTouche(Cross);
      etat = ETAT_CREDIT;
      InitialiseGame();
      return;
     }
    if (idMenu == 3)
     {
      Quit();
      return;
     }
   } // fin du if CROSS
  if (PGet(Up)) { PBloqueTouche(Up); idMenu--; PPlaySound(4, 1); }
  if (PGet(Down)) { PBloqueTouche(Down); idMenu++; PPlaySound(4, 1); }
  if (idMenu > 3) idMenu = 0;
  if (idMenu < 0) idMenu = 3;

  // les gouttes
  int i;
  switch (idMenu) {
   case 0 : angle = 3.4; break; //2
   case 1 : angle = 3.6; break; //2.2
   case 2 : angle = 3.8; break; //2.4
   case 3 : angle = 3.95; break; //2.6
   default : angle = 3.14;
   }
  if (!cptMenu && bonusMenu)
   {
    //new_goutte2(-12.0, 6.28-angle, 10, 200);
    new_goutte2(12.0, angle, 470, 30);
   }
  //new_goutte(14,5);
  bougeGouttes();
  for (i=0; i< gouttes.nbGouttes; i++) print_goutte(gouttes.gouttes[i]);

  // affichage du menu choisi
  int sprite = 42 + idMenu;
  PPrintImage(157, 20, sprite);
  
 }



//------------------------------------ credits du jeu
int decalage = 0;
int sens = -1;
int bonusCredit = 0;
#define tailleTexte 400

void initCredits()
 {
  PVitesse(0.0);
  gouttes.nbGouttes = 0;
  niveau.gravite = 0.5;
  niveau.vent = 0;
  decalage = 100;
  sens = -1;
  PChangeFond("datas/static/credits/fond.png");
 }


void credits()
 {
  if (PGet(R)) bonusCredit = 1;
  if (PGet(L)) bonusCredit = 0;
  if (PGet(Cross))
   {
    PPlaySound(7, 1);
    PBloqueTouche(Cross);
    etat = ETAT_MENU;
    InitialiseGame();
    return;
   } // fin du if CROSS
  if (PGet(Up)) {
   sens = -1;
   decalage -=1;
  }
  if (PGet(Down)) {
   sens = 1;
   decalage +=1;
   }
  decalage += sens;
  if (decalage > tailleTexte) decalage = 0;
  else if (decalage < 0) decalage = tailleTexte;
  // affichage des deux images a la suite
  PPrintImage(204, 75+decalage, 69);
  PPrintImage(204, 75+decalage-tailleTexte, 69);
  //puis de la facade de l'extincteur
  PPrintImage(0, 0, 18);

  //puis les gouttes
  int i;
  if (bonusCredit)
   {
    if (PGet(Left))
     {
      niveau.vent -= 1;
      if (niveau.vent < -6) niveau.vent = -6;
      PBloqueTouche(Left);
     }
    if (PGet(Right))
     {
      niveau.vent += 1;
      if (niveau.vent > 6) niveau.vent = 6;
      PBloqueTouche(Right);
     }
    new_goutte2(5, 4.71, monRand(0,480), 0-monRand(0,4) );
   }
  bougeGouttes();
  for (i=0; i< gouttes.nbGouttes; i++) print_goutte(gouttes.gouttes[i]);
  
 }


//------------------------- jeu

int numNiveauCourant = 0;
int niveauMax = 0;

void initJeu()
{
 //PChangeMusiqueFond("datas/audio/test.ogg");
 //PPlayStopFond(1);
 nbSauves = 0;
 nbMorts = 0;
 echelle.x = 200;
 echelle.y = 170;
 gouttes.nbGouttes = 0;
 feux.nbFeux = 0;
 personnages.nbPersos = 0;
 messages.nbMess = 0;
 eclaboussures.nbEcla = 0;
 fumees.nbFumees = 0;
 pspfrequence(333);
 PVitesse(1.3);

 //listeNiveaux = chargeListeNiveau();
 niveau = PChargeNiveau(listeNiveaux.path[numNiveauCourant], &fenetres);
 ajout_feu();
 ajout_perso();
}

void jeu()
{
 // la pluie c'est pour ceux qui ont fini le jeu
 if (PGet(L) && PGet(R) && (niveauMax == (listeNiveaux.nbLv-1))) { new_goutte2(5, 4.71, monRand(0,480), 0-monRand(0,4) ); }
 else if (PGet(L) || PGet(R)) { PPlaySound(3, 2); new_goutte(14,5); }
 ajout_feu();
 ajout_perso();
 bougeGouttes();
 eteint();
 persoBrule();
 sauvePerso();
 bougeAnge();
 bougeEchelle();
 affichage();
 //PPrintImage(0,0, 68); // l'image du titre en sur-impression


 // gagne
 if (nbSauves >= niveau.limite_sauves)
  {
   etat = ETAT_GAGNE;
   InitialiseGame();
  }
 // perdu
 if (nbMorts > niveau.limite_morts)
  {
   etat = ETAT_PERDU;
   InitialiseGame();
  }

 if (PGet(Start))
  {
   etat = ETAT_PAUSE;
   InitialiseGame();
  }
}


//------------------ jeu en pause

void initPause()
{
 //PPlaySound(4, 1);
 int volume = PGetVolume();
 PSetVolume(volume/2);
 PBloqueTouche(Cross);
 PBloqueTouche(Triangle);
}


void Pause()
{
 PPrintImage(0, 0, 48);
 if (PGet(Cross))
  {
   int volume = PGetVolume();
   PSetVolume(2*volume);
   etat = ETAT_JEU;
   PBloqueTouche(Cross);
  }
 if (PGet(Triangle))
  {
   int volume = PGetVolume();
   PSetVolume(2*volume);
   PPlaySound(7, 1);
   etat = ETAT_CHOIXLV;
   PBloqueTouche(Triangle);
   InitialiseGame();
  }
}


//---------------------  gagne

void initGagne()
{
 PVitesse(1.0);
 PPlaySound(5, 1);
 PBloqueFond();
 // verifie qi on a fini un niveau encore non fini
 if (numNiveauCourant < listeNiveaux.nbLv-1)
  {
   numNiveauCourant++;
   if (numNiveauCourant > niveauMax)
    {
     listeNiveaux.bloque[niveauMax+1] = 0;
     niveauMax = numNiveauCourant;
     sauveListeNiveau();
    }
  }
}


void gagne()
{
 PPrintImage(90, 12, 35);
 if (PGet(Start))
  {
   PPlaySound(7, 1);
   if (numNiveauCourant == listeNiveaux.nbLv-1) etat = ETAT_FINI;
   else etat = ETAT_CHOIXLV;
   InitialiseGame();
  }
}


//---------------------  perdu

void initPerdu()
{
 PVitesse(1.0);
 PPlaySound(6,1);
 PBloqueFond();
}


void perdu()
{
 PPrintImage(82, 0, 49);
 if (PGet(Start))
  {
   PPlaySound(7, 1);
   etat = ETAT_CHOIXLV;
   InitialiseGame();
  }
}



//------------------------------------ choix du lv

int selectionLv = 0;
int oldSelection = 0;
int transition = 0;
int sensMvt = 0;
int fastScroll = 0;
int appuiProlonge = 0;

void initChoixLv()
{
  PVitesse(0.0);
  selectionLv = numNiveauCourant;
  PChangeFond("datas/static/menus/selectionLv.png");
  int i = 0;
  while(listeNiveaux.bloque[i+1] == 0 && i < listeNiveaux.nbLv-1) i++;
  niveauMax = i;
  fastScroll = 0;
  appuiProlonge = 0;
}

void choixLv()
{
 //PPrintNb(20,20, niveauMax);
 //PPrintNb(40, 20, selectionLv);
 //PPrintNb(20, 40, listeNiveaux.nbLv);
 if (transition <= 0) // si on est pas en train de bouger
  {
   if (selectionLv > 0)
    { // niveau - 1
     PPrintMiniNiveau(20, 97, listeNiveaux.apercut[selectionLv-1], 0, 0, 0, 0);
    }
   PPrintMiniNiveau(180, 97, listeNiveaux.apercut[selectionLv], 0, 1, listeNiveaux.limite_sauves[selectionLv], listeNiveaux.limite_morts[selectionLv]);
   if (selectionLv < listeNiveaux.nbLv-1)
    {// niveau - 1
     PPrintMiniNiveau(340, 97, listeNiveaux.apercut[selectionLv+1], listeNiveaux.bloque[selectionLv+1], 0, 0, 0);
    }
   if (PGet(Right) && selectionLv < niveauMax)
    {
     PPlaySound(4,1);
     if (sensMvt == 1 && appuiProlonge == 1) fastScroll = 1;
     else fastScroll = 0;
     appuiProlonge = 1;
     oldSelection = selectionLv;
     selectionLv++;
     transition = 160;
     sensMvt = 1;
    }
   if (PGet(Left) && selectionLv > 0)
    {
    PPlaySound(4,1);
     if (sensMvt == -1 && appuiProlonge == 1) fastScroll = 1;
     else fastScroll = 0;
     appuiProlonge = 1;
     oldSelection = selectionLv;
     selectionLv--;
     transition = 160;
     sensMvt = -1;
    }
   if (PGet(Triangle))
    {
     PPlaySound(7,1);
     PBloqueTouche(Triangle);
     etat = ETAT_MENU;
     InitialiseGame();
    }
   if (PGet(Cross))
    {
     PPlaySound(7,1);
     PBloqueTouche(Cross);
     etat = ETAT_JEU;
     numNiveauCourant = selectionLv;
     InitialiseGame();
    }
  } // fin de l'etat non transition
 else // si on est en train de changer de selection
  {
   if ( (sensMvt == 1 && !PGet(Right)) || (sensMvt == -1 && !PGet(Left))) appuiProlonge = 0;
   transition -= 10+(8*fastScroll);
   // on re-affiche les 3 niveau d'avant
   if (oldSelection > 0)
       PPrintMiniNiveau(20 + (160-transition)*(-sensMvt), 97, listeNiveaux.apercut[oldSelection-1], 0, 0, 0, 0);
   PPrintMiniNiveau(180 + (160-transition)*(-sensMvt), 97, listeNiveaux.apercut[oldSelection], 0, 0, 0, 0);
   if (oldSelection < listeNiveaux.nbLv-1)
       PPrintMiniNiveau(340 + (160-transition)*(-sensMvt), 97, listeNiveaux.apercut[oldSelection+1], listeNiveaux.bloque[oldSelection+1], 0, 0, 0);
   // on affiche le nouveau niveau
   if (sensMvt == 1 && selectionLv < listeNiveaux.nbLv-1)
      PPrintMiniNiveau(500 + (160-transition)*(-sensMvt), 97, listeNiveaux.apercut[selectionLv+1], listeNiveaux.bloque[selectionLv+1], 0, 0, 0);
   if (sensMvt == -1 && selectionLv > 0)
      PPrintMiniNiveau(-140 + (160-transition)*(-sensMvt), 97, listeNiveaux.apercut[selectionLv-1], listeNiveaux.bloque[selectionLv+1], 0, 0, 0);
  }
}


//--------------------------------------------------/
//-------------------- SURVIVAL ------------------
//--------------------------------------------------/

#define LIMITE 12000
//------------- special survival
int combo = 0;
int vitesseJeu = 1.5;
unsigned int nbFeuxEteints = 0;
unsigned long nbGoutteTirees = 0;
long timeDebut = 0;
long timeCourant = 0;
long timeLimiteProchain = 0;


//--------------------------------
// gere les interraction gouttes / feux SURVIVAL
//--------------------------------
void eteintSurvival()
 {
  int i,j;
  for (i=0; i< gouttes.nbGouttes; i++)
   {
    goutte *g = &gouttes.gouttes[i];
    for (j=0; j< feux.nbFeux; j++)
     {
      feu *f = &feux.feux[j];
      int xf = fenetres.fenetres[f->idFen].x;
      int yf = fenetres.fenetres[f->idFen].y;
      if ((g->x+3 > xf - TAILLEFEU/2) && (g->x-3 < xf + TAILLEFEU/2) && (g->y+3 > yf - TAILLEFEU) && (g->y-3 < yf))
       {// sur le feu
        points += combo*15;
        new_eclaboussure(f->idFen); // ajoute une eclaboussure
        f->hp -= g->hp;
        if (f->hp <= 0)
         {
          if(fenetres.fenetres[f->idFen].etat == BRULE) fenetres.fenetres[f->idFen].etat = OK;
          else
           { // il y avait qq'un
            fenetres.fenetres[f->idFen].etat = PERSO;
            new_message(fenetres.fenetres[f->idFen].x + TAILLE/2, fenetres.fenetres[f->idFen].y - (TAILLE +20) , 1);
            points += combo*5;
           }//fin du else
          // suppression du feu
          feux.nbFeux--;
          feux.feux[j] = feux.feux[feux.nbFeux];
          j--;
          // score
          points += 150*combo;
          nbFeuxEteints++;
         }
        // suppression de la goutte
        gouttes.nbGouttes--;
        gouttes.gouttes[i] = gouttes.gouttes[gouttes.nbGouttes];
        i--;
        break; // la goutte a disparu, pas la peine de verifier les autres feux
       } // fin de la goutte sur le feu
     } // fin du for gouttes
   } // fin du for feux
 }

//------------------------
// fait bruller les persos SURVIVAL
//------------------------
void persoBruleSurvival()
 {
  int i;
  for (i=0; i< personnages.nbPersos; i++)
   {
    perso *p = &personnages.personnages[i];
    if (fenetres.fenetres[p->idFen].etat == PERSO_BRULE)
     { // sa fenetre brule
      p->hp--;
      if (p->hp <=0)
       {//perso mort
        PPlaySound(2, 3);
        p->etat = 2;
        nbMorts++;
        p->y = fenetres.fenetres[p->idFen].y - TAILLEPERSO;
        fenetres.fenetres[p->idFen].etat = BRULE;
        //score
        combo = 0;
       }
     }
   }
 }



//----------------
// affichage complet  SURVIVAL
//----------------
void affichageSurvival()
 {
  int i;
  for (i=0; i< fenetres.nbFen; i++) PPrintImage(fenetres.fenetres[i].x-TAILLE, fenetres.fenetres[i].y-(38), 16);;
  for (i=0; i< feux.nbFeux; i++) print_feu(&feux.feux[i]);
  PPrintImage(0, 0, 36); // affiche le mur
  for (i=0; i< personnages.nbPersos; i++) print_perso2(personnages.personnages[i]);
  for (i=0; i< fumees.nbFumees; i++) print_fumee(&fumees.fumees[i], i);
  for (i=0; i< eclaboussures.nbEcla; i++) print_eclaboussure(i, &eclaboussures.eclaboussures[i]);
  PPrintImage(echelle.x, echelle.y, 19); // affichage de l'echelle
  for (i=0; i< personnages.nbPersos; i++) print_perso(personnages.personnages[i]);
  for (i=0; i< personnages.nbPersos; i++) print_perso3(personnages.personnages[i]);
  for (i=0; i< messages.nbMess; i++) print_message(i, &messages.messages[i]);
  // score
  PPrintScore(niveau.posSauves.x, niveau.posMorts.y + 20, points);
  PPrintNb(niveau.posSauves.x, niveau.posMorts.y + 40, (timeLimiteProchain - timeCourant) / 1000);
  PPrintNb(niveau.posSauves.x, niveau.posSauves.y, nbSauves);
  PPrintNb(niveau.posMorts.x, niveau.posMorts.y, niveau.limite_morts - nbMorts);
  //PPrintNb(niveau.posMorts.x+20, niveau.posMorts.y, niveau.limite_morts);
  //PPrintNb(niveau.posMorts.x+50, niveau.posMorts.y, niveau.hp_persos);
  for (i=0; i< gouttes.nbGouttes; i++) print_goutte(gouttes.gouttes[i]);
 }


//------------------------------
// sauve les personnages SURVIVAL
//------------------------------
void sauvePersoSurvival()
 {
  int i;
  int e = positionEchelle();
  for(i=0; i<personnages.nbPersos; i++)
   {
    if (e == personnages.personnages[i].idFen && fenetres.fenetres[e].etat == PERSO)
     { //perso sauvable
      fenetres.fenetres[e].etat = OK;
      personnages.personnages[i].etat = 1;
      personnages.personnages[i].x = echelle.x;
      personnages.personnages[i].y = echelle.y-4;
      nbSauves++;
      // donne des points
      combo++;
      points+= 100*combo;
      timeLimiteProchain = timeCourant + LIMITE;
      // rajoute des personnages qui peuvent mourrir
      if (!(nbSauves % 10))
       {
        if (nbMorts > 0) nbMorts--;
       }
      if (!(nbSauves % 15))
       {
        niveau.vent = 3-monRand(0,6);
        if (niveau.hp_persos>20) niveau.hp_persos-=4;
        if (niveau.hp_feux<200) niveau.hp_feux+=4;
        if (vitesseJeu > 0.0) vitesseJeu -=0.3;
        PVitesse(vitesseJeu);
       }
      if (!(nbSauves % 75) && niveau.limite_morts) niveau.limite_morts--;
      return; // un seul a la fois maxi
     }
   }
 }

//------------- fin special survival

void initSurvival()
{
 points = 0;
 nbFeuxEteints = 0;
 nbGoutteTirees = 0;
 vitesseJeu = 1.5;
 PVitesse(vitesseJeu);


 nbSauves = 0;
 nbMorts = 0;
 echelle.x = 200;
 echelle.y = 170;
 gouttes.nbGouttes = 0;
 feux.nbFeux = 0;
 personnages.nbPersos = 0;
 messages.nbMess = 0;
 eclaboussures.nbEcla = 0;
 fumees.nbFumees = 0;
 pspfrequence(333);
 niveau = PChargeNiveau("datas/survival.lv", &fenetres);
 niveau.vent = 3-monRand(0,6);
 ajout_feu();
 ajout_perso();

 timeDebut = PTime();
 timeLimiteProchain = timeDebut + LIMITE;
}


void survival()
{
 timeCourant = PTime();

 if (PGet(L) || PGet(R))
  {
   PPlaySound(3, 2);
   new_goutte(14,5);
   if (points > 0) points--;
   nbGoutteTirees++;
  }
 ajout_feu();
 ajout_perso();
 bougeGouttes();
 eteintSurvival();
 persoBruleSurvival();
 sauvePersoSurvival();
 bougeAnge();
 bougeEchelle();
 affichageSurvival();

 // perdu
 if (timeLimiteProchain < timeCourant)
  {
   nbMorts++;
   timeLimiteProchain = timeCourant + LIMITE;
  }
  // vraiment perdu
 if (nbMorts > niveau.limite_morts)
  {
   etat = ETAT_SCORE;
   InitialiseGame();
  }
}




//---------------------  score
long timeFin = 0;

void initScore()
{
 PVitesse(1.0);
 PPlaySound(6,1);
 PChangeFond("./datas/static/menus/fondScore.png");
 timeFin = PTime();
 if (points > bestScore)
  {
   bestScore = points;
   int i;
   for(i=0; i<strlen(chaineCode); i++) chaineCode[i] = '\0';
   sauveScore();
  }
 else
  {
   int i;
   for(i=0; i<strlen(chaineCode); i++) chaineCode[i] = '\0';
   char tmp[50] = "";
   codage(tmp, bestScore, 0);
   strcpy(chaineCode, tmp);
  }
}


void score()
{
 int posDebut = 9;
 int decalage = 26;
 PPrintNb(445, posDebut, nbSauves);
 PPrintNb(445, posDebut+decalage, nbFeuxEteints);
 PPrintNb(445, posDebut+decalage*2, nbGoutteTirees);
 unsigned int duree = (timeFin - timeDebut)/ 1000;
 int nbMinutes = duree / 60;
 int nbSecondes = duree % 60;
 PPrintNb(405, posDebut+decalage*3, nbMinutes);
 PPrintNb(445, posDebut+decalage*3, nbSecondes);

 PPrintScore(445,posDebut+decalage*4, points);

 PPrintScore(445, posDebut+decalage*5, bestScore);
 char tmp[50];
 strcpy(tmp, chaineCode);
 PPrintCode(445, posDebut+decalage*6, tmp);
 
 if (PGet(Cross))
  {
   PPlaySound(7, 1);
   etat = ETAT_MENU;
   InitialiseGame();
  }
}


//------------------------------  FINI   -----------------------------------

void initFini()
{
 PChangeFond("datas/static/menus/fini.png");
}

void fini()
{
 if(PGet(Cross))
  {
   PPlaySound(7,1);
   PBloqueTouche(Cross);
   etat= ETAT_MENU;
   InitialiseGame();
  }
}


//###############################  BOUCLE  #################################
void InitialiseGame()
{
 switch (etat) {
  case ETAT_MENU: initMenus(); break;
  case ETAT_CREDIT : initCredits(); break;
  case ETAT_CHOIXLV : initChoixLv(); break;
  case ETAT_JEU : initJeu(); break;
  case ETAT_PAUSE : initPause(); break;
  case ETAT_GAGNE : initGagne(); break;
  case ETAT_PERDU : initPerdu(); break;
  case ETAT_SURVIVAL : initSurvival(); break;
  case ETAT_SCORE : initScore(); break;
  case ETAT_FINI : initFini(); break;
  default : break;
 }
}

void GameLoop()
{
 switch (etat) {
  case ETAT_MENU: menus(); break;
  case ETAT_CREDIT : credits(); break;
  case ETAT_CHOIXLV : choixLv(); break;
  case ETAT_JEU : jeu(); break;
  case ETAT_PAUSE : Pause(); break;
  case ETAT_GAGNE : gagne(); break;
  case ETAT_PERDU : perdu(); break;
  case ETAT_SURVIVAL : survival(); break;
  case ETAT_SCORE : score(); break;
  case ETAT_FINI : fini();
  default : break;
 }

}

