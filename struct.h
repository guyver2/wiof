#ifndef STRUCT_H
#define STRUCT_H



//---------------------------- CONSTANTES
#define TAILLE 15
#define TAILLEPERSO 40
#define TAILLEFEU 35
//double GRAVITE = 0.32;
#define POS_X 470
#define NB_NIVEAU_MAX  50
//#define POS_Y 110
enum {OK, BRULE, PERSO, PERSO_BRULE};

//---------------------------- STRUCTURES

// stocke la liste des niveau
typedef struct ListeNiveaux{
 int nbLv;
 char **path;
 char **pathApercut;
 int *bloque;
 SDL_Surface **apercut;
 int *limite_sauves;
 int *limite_morts;
} ListeNiveaux;


ListeNiveaux listeNiveaux;

// permet de stocker un couple d'entier
typedef struct couple {
 int x;
 int y;
}couple;



/* definie les données d'un niveau */
typedef struct Niveau{
 int limite_sauves;
 int limite_morts;
 int vent;
 double gravite;
 int POS_Y;
 int hp_feux;
 int hp_persos;
 couple posSauves;
 couple posMorts;
} Niveau;


/* defini une fenetre */
typedef struct fenetre{
 int x;
 int y;
 int etat;
} fenetre;

#define MAX_FEN 20
typedef struct tabFenetres{
 int nbFen;
 fenetre fenetres[MAX_FEN];
}tabFenetres;


/* defini un personnage */
typedef struct perso{
 int idFen;
 int hp;
 int x;
 int y;
 int etat; // sur une fenetre, sur l'echelle, ...
}perso;

#define MAX_PERSO 30
typedef struct tabPersonnages{
 int nbPersos;
 perso personnages[MAX_PERSO];
} tabPersonnages;



/* defini un feu */
typedef struct feu{
 int idFen;
 int hp;
 int compteur;
}feu;

#define MAX_FEU 20
typedef struct tabFeux{
 int nbFeux;
 feu feux[MAX_FEU];
} tabFeux;



/* defini ue goutte */
typedef struct goutte{
 int x;
 int y;
 int xInit;
 int yInit;
 float V0_sin;
 float V0_cos;
 int t;
 int hp;
}goutte;

#define MAX_GOUTTE 200
typedef struct tabGouttes{
 int nbGouttes;
 goutte gouttes[MAX_GOUTTE];
} tabGouttes;


/* defini un message */
typedef struct message{
 int x;
 int y;
 int t;
 int typ;
}message;

#define MAX_MESSAGE 25
typedef struct tabMessages{
 int nbMess;
 message messages[MAX_MESSAGE];
} tabMessages;


/* definie une eclaboussure */
typedef struct eclaboussure{
 int idFen;
 int t;
} eclaboussure;

#define MAX_ECLABOUSSURE 50
typedef struct tabEclaboussures{
 int nbEcla;
 eclaboussure eclaboussures[MAX_ECLABOUSSURE];
} tabEclaboussures;




/* definie une fumee */
typedef struct fumee{
 int t;
 int x;
 int y;
} fumee;

#define MAX_FUMEES 100
typedef struct tabFumees {
 int nbFumees;
 fumee fumees[MAX_FUMEES];
} tabFumees;

#endif
