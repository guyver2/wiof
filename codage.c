#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "codage.h"

char correspondance[9][10] = {
{'3', '5', '0', '6', '7', '1', '9', '2', '4', '8' }, //correspondance[0]
{'8', '7', '2', '0', '6', '3', '4', '1', '9', '5' }, //correspondance[1]
{'9', '1', '4', '5', '0', '8', '2', '6', '3', '7' }, //correspondance[2]
{'4', '8', '7', '3', '5', '0', '1', '9', '6', '2' }, //correspondance[3]
{'1', '2', '9', '8', '4', '6', '0', '7', '5', '3' }, //correspondance[4]
{'5', '6', '3', '2', '9', '7', '4', '0', '8', '1' }, //correspondance[5]
{'6', '3', '1', '9', '2', '5', '8', '7', '0', '4' }, //correspondance[6]
{'2', '9', '5', '4', '7', '8', '3', '1', '6', '0' }, //correspondance[7]
{'7', '4', '8', '1', '6', '3', '5', '2', '9', '0' }  //correspondance[8]
};

//char chaineCode[50] = "\0";

int somme(char* chaine, int decalage)
{
 if (decalage == strlen(chaine)) return 0;
 char tmp[2];
 sprintf(tmp, "%c", (chaine[decalage]));
 int v = atoi(tmp);
 return v + somme(chaine, decalage+1);
}



void codage(char* code, unsigned long score, int decalage)
{
 if (decalage> 8) decalage = 0;
 int v = score%10;
 char c = correspondance[decalage][v];
 code[strlen(code)] = c;
 code[strlen(code)+1] = '\0';
 printf("lu %d, ajout de %c (rang %d)\n>>%s\n", v, c, decalage, code);
 if (score / 10) codage(code, score/10, decalage+1);
 else
  {
   int tmp = somme(code,0);
   int reste = tmp%10;
   sprintf(code, "%s%d", code, reste);
  }
}




//----------------------------------------------------------

int verifCRC(char* code, int val, int decalage)
{
 if (decalage == strlen(code)-1) // on a presque fini;
  {
   char tmp[2];
   sprintf(tmp, "%c", (code[decalage]));
   int v = atoi(tmp);
   if (v == val%10) return 1;
   else return 0;
  }
  char tmp[2];
  sprintf(tmp, "%c", (code[decalage]));
  int v = atoi(tmp);
  return verifCRC(code, val+v, decalage+1);
}


int indice(char v, char seed[10])
{
 int i = 0;
 while(seed[i] != v && i<11) { printf("seed[%d] = %c\n", i, seed[i]); i++; }
 printf("trouve : seed[%d] (%c) = %c\n", i, seed[i], v);
 return i;
}

unsigned long decodage(char* code, unsigned long score, int decalage)
{
 // verif initiale
 if (decalage == 0)
  {
   if (!verifCRC(code,0, 0)) { printf("tricheur !!!\n"); return 0;}
   else printf("code correcte apparement\n");
  }

 //- decodage
 int rang = decalage % 9;
 char tmp[2]; 
 sprintf(tmp, "%d", indice(code[decalage], correspondance[rang]));
 int val = atoi(tmp);
 printf("lu %c, decode %d, (rang %d)\n------------\n", code[decalage], val, rang);
 if (decalage == strlen(code)-2) return pow(10,decalage)*val + score;
 return decodage(code, pow(10,decalage)*val + score, decalage+1);
}

