#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct prodotti {
  int id_neg;
  char nome_prod[20];
}Prodotto;
typedef struct negozio {
  int id_client;
  int id_neg;
  char nome_neg[20];
}Negozio;

int main(){
  FILE *Pf1;
  FILE *Pf2;
  Negozio A[4];
  Prodotto B[4];
  int i = 0;
  int size = 3;

  strcpy(A[0].nome_neg, "Nike");
  A[0].id_neg = 1;
  A[0].id_client = 221;

  strcpy(A[1].nome_neg, "Adidas");
  A[1].id_neg = 2;
  A[1].id_client = 150;

  strcpy(A[2].nome_neg, "Lacoste");
  A[2].id_neg = 3;
  A[2].id_client = 600;
  // ==================================
  strcpy(B[0].nome_prod, "Scarpa 720");
  B[0].id_neg = 1;

  strcpy(B[1].nome_prod, "Polo");
  B[1].id_neg = 3;

  strcpy(B[2].nome_prod, "Felpa");
  B[2].id_neg = 2;

  Pf1=fopen("Negozi.rfm", "wb");

  if(Pf1){
    fseek(Pf1, 0, SEEK_SET);
    fwrite(&size, sizeof(int), 1, Pf1);
    /*fseek(Pf1, sizeof(int)*sizeof(Negozio), SEEK_SET);*/
    for(i = 0; i < 3; i++){
      fwrite(&A[i], sizeof(Negozio), 1, Pf1);
    }
    fclose(Pf1);
  }
  else
      printf("Errore nella scrittura su file.\n");

  Pf2=fopen("Prodotti.rfm", "wb");

  if(Pf2){
    fseek(Pf2, 0, SEEK_SET);
    fwrite(&size, sizeof(int), 1, Pf2);
    /*fseek(Pf2, sizeof(int)*sizeof(Prodotto), SEEK_SET);*/
    for(i = 0; i < 3; i++){
      fwrite(&B[i], sizeof(Prodotto), 1, Pf2);
    }
    fclose(Pf2);
  }
  else
      printf("Errore nella scrittura su file.\n");

  return 0;
}
