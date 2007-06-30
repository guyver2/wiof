/****************************************************************************/
/*  PGCK : Psp Game Creator Kit base sur NGCK de Lapintade                  */
/*  Porte par Antoine Letouzey                                              */
/*  ameboure@yahoo.fr                                                       */
/****************************************************************************/

#ifndef PGCK_H
#define PGCK_H

#define printf pspDebugScreenPrintf

#include "../struct.h"


// -----------------------------------------------------
// Structures de données.
// -----------------------------------------------------


typedef struct touche
{
      char etat;
      char libre;
}touche;

//------------------------------------------------------
// Constantes
//------------------------------------------------------
enum Touche {Circle, Triangle, Square, Cross, Start, Select, Right, Up, Left, Down, L, R};





// -----------------------------------------------------
// Fonctions utilisables hors de Agck.c
// -----------------------------------------------------

// chargement d'un niveau : le fichier doit etre structure comme dans la specification
Niveau PChargeNiveau(char *fic, tabFenetres *fen);


// affichage
void PPrintFond();
void PPrintImage(int x, int y, int numSprite);
void PPrintNb(int x, int y, int v);
void PPrintScore(int x, int y, unsigned long v);
void PPrintCode(int x, int y, char* code);
void PPrintMiniNiveau(int x, int y, SDL_Surface *mini, int bloque, int infos, int nbS, int nbM);

// change l'image de fond
void PChangeFond(char* fic);
void PBloqueFond();

// affichage d'une chaine
//void APrintMot(int x, int y, char* ch, int taille); 

// renvois l'état d'une touche (le résultat est à traiter comme un booléen)
int PGet(int);
couple PGetJoy(void);

// bloque une touche en particulier (voir la doc)
void PBloqueTouche(int x);
// bloque toute les touches
void PEffaceTouches(void);

// donne le temps
long PTime();
//fixe la vitesse du jeu
void PVitesse(float v);

// définie la couleur du texte
//void ASetCouleurTexte(int R, int V, int B);

// quite le jeu
void Quit();
// -----------------------------------------------------

#endif
