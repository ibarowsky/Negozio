/*
  Client N identifica i proprietari dei negozi che possono
  amministrare i propri negozi o i prodotti, aggiungendone
  e rimuovendone una quantità indefinita.

  Ogni caso dello switch sarà inviato prima al Server N,
  poi successivamente che provvederà all'inserimento o
  eliminazione di Profotti o Negozi.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct trasporto {
  int flag;
  int id_client;
  int id_neg;
  char nome_neg[20];
  char nome_prod[20];
}Trasporto;

int sock, i=0,j=0,size_neg_client, size_prod,flag;
struct sockaddr_in in;
char buff[4096];
int size = 0, num = 1;
unsigned int len = sizeof(in);
int Risp;
char Rigo[20], Lista[1000];
Trasporto *A, *B;
int id = 0,check = 0;
char id_prima;
void prodotti_aggiornati();

int main(int argc, const char * argv[]) {


  if ( ( sock = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("Errore nella creazione del socket");
    exit(1);
  }
  in.sin_family      = AF_INET;
  in.sin_port        = htons(1031);
  if (inet_pton(AF_INET, argv[1], &in.sin_addr) < 0){
    fprintf(stderr,"inet_pton error for %s\n", argv[1]);
    exit (1);
  }
  if (connect(sock, (struct sockaddr *) &in, len) < 0) {
    perror("Errore nella connect");
    exit(1);
  }
  // Creazione pacchetto
  A = (Trasporto *)calloc(1, sizeof(Trasporto));

  // id indica un pin (è personale e univoco per ogni negoziante)
  // può essere creato se non lo si ha o utilizzare quello che si ha
  printf("Inserisci un nuovo id o usa il tuo: ");
  gets(Rigo);
  Risp = atoi(Rigo);
  A->id_client = Risp;
  A->id_neg = -1;
  strcpy(A->nome_neg,"");
  strcpy(A->nome_prod,"");
  // Qui inseriamo solo l'id del cliente nel pacchetto
  // perché servirà al Server N per inviare al client N
  // solo i negozi associati a questo id client
  while(1){
    sleep(1);
    prodotti_aggiornati();
    // Ricezione dei negozi e dei prodotti del cliente

    // Menu
    Risp=-1;
    printf("\n======= MENU =======\n");
    printf("Cosa vuoi fare?\n[1] Aggiungi un Negozio\n[2] Aggiungi un Prodotto\n[3] Elimina un Negozio\n[4] Elimina un Prodotto\n[5] Esci\n");
    scanf("%d",&Risp);
    A->flag=Risp;
    fflush(stdin);
    printf("====================\n");

    switch(A->flag){
      case 1: // Aggiungi negozio

        // Inviamo al ServerN sia il nome del Negozio
        // che il nome del primo Prodotto
        // (Non possono esistere negozi che non hanno prodotti)
        memset((void*)&Rigo,0,20*sizeof(char));
        printf("\nInserisci nome negozio: ");
        fflush(stdin);
        scanf("%s",Rigo);
        fflush(stdin);
        strcpy(A->nome_neg,Rigo);
        memset((void*)&Rigo,0,20*sizeof(char));
        printf("\nInserisci nome prodotto: ");
        fflush(stdin);
        scanf("%s",Rigo);
        fflush(stdin);
        strcpy(A->nome_prod,Rigo);
        write(sock,A,sizeof(Trasporto));
      break;
      case 2: // Inserimento di un prodotto in un negozio esistente
        // Controlliamo che esista il negozio e inviamo il nome
        // del prodotto
        check = 0;
        while(check == 0){
          memset((void*)&Rigo,0,20*sizeof(char));
          printf("\nInserisci nome negozio per quel prodotto: ");
          fflush(stdin);
          scanf("%s",Rigo);

          for(i = 0; i < size_neg_client; i ++){
            if(strcmp(B[i].nome_neg,Rigo) == 0){
              check = 1;
              A->id_neg = B[i].id_neg;
            }
          }
        }
        strcpy(A->nome_neg,Rigo);
        fflush(stdin);
        memset((void*)&Rigo,0,20*sizeof(char));
        printf("\nInserisci nome prodotto: ");
        fflush(stdin);
        scanf("%s",Rigo);
        strcpy(A->nome_prod,Rigo);
        fflush(stdin);
        write(sock,A,sizeof(Trasporto));
      break;
      case 3: // Elimina negozio
        // Controlliamo che esista il negozio e lo Inviamo
        // al ServerN
        check = 0;
        while(check == 0){
          memset((void*)&Rigo,0,20*sizeof(char));
          printf("\nInserisci nome negozio: ");
          fflush(stdin);
          scanf("%s",Rigo);
          for(i = 0; i < size_neg_client; i ++){
            if(strcmp(B[i].nome_neg,Rigo) == 0){
              check = 1;
              A->id_neg = B[i].id_neg;
              strcpy(A->nome_neg,Rigo);
            }
          }

        }
        write(sock, A, sizeof(Trasporto));
       break;
      case 4: // Elimina prodotto
        // Controlliamo l'esistenza del prodotto
        // e lo inviamo
        check = 0;
        while(check == 0){
          memset((void*)&Rigo,0,20*sizeof(char));
          printf("\nInserisci nome prodotto: ");
          gets(Rigo);
          for(i = 0; i < size_neg_client; i ++){
            if(strcmp(B[i].nome_prod,Rigo) == 0){
              check = 1;
              A->id_neg = B[i].id_neg;
              strcpy(A->nome_prod, Rigo);
              strcpy(A->nome_neg, B[i].nome_prod);
            }
          }
        }

        write(sock, A, sizeof(Trasporto));
       break;
      case 5:
            exit(0);
      break;
    }
  }

  return 0;

}


void prodotti_aggiornati()
{
  // Ricezione dei prodotti e dei negozi dal Server N
  A->flag = 0;
  write(sock, A, sizeof(Trasporto));
  read(sock, &size_neg_client, sizeof(int));
  B = (Trasporto *)calloc(size_neg_client, sizeof(Trasporto));
  for(i = 0; i < size_neg_client; i++){
    read(sock, &B[i], sizeof(Trasporto));
  }
  // Stampa
  printf("\n===== STAMPA DEI NEGOZI =====\n");
  for(i = 0; i < size_neg_client; i++){
    printf("id client: %d\n",B[i].id_client);
    printf("id neg: %d\n", B[i].id_neg);
    printf("nome neg: %s\n",B[i].nome_neg);
    printf("nome prod: %s\n", B[i].nome_prod);
  }
  printf("===============================\n");
}
