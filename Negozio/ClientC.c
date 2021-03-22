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
} Trasporto;

// variabili Globali
int sock, i=0,j=0,size_ListaNeg, size_Carrello, flag;
struct sockaddr_in in;
char buff[4096];
int size = 0, num = 1,size_Carrello, indiceProd;
unsigned int len = sizeof(in);
int Risp;
char ProdIns[20];
Trasporto *ListaNeg, *Carrello, *App;
int id = 0,check = 0, control = 0;
char id_prima;
void prodotti_aggiornati(void);
// -----------------

int main(int argc, const char * argv[]) {
  if ( ( sock = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("Errore nella creazione del socket");
    exit(1);
  }
  in.sin_family      = AF_INET;
  in.sin_port        = htons(1030);
  if (inet_pton(AF_INET, argv[1], &in.sin_addr) < 0){
    fprintf(stderr,"inet_pton error for %s\n", argv[1]);
    exit (1);
  }
  if (connect(sock, (struct sockaddr *) &in, len) < 0) {
    perror("Errore nella connect");
    exit(1);
  }

  // Ciclo principale
  while(1){
    // Ricezione dei negozi e dei prodotti del cliente
    prodotti_aggiornati();

    // Stampa del menu
    Risp=-1;
    printf("\n======= MENU =======\n");
    printf("Cosa vuoi fare?\n[1] Stampa Carrello\n[2] Inserisci Prodotto nel Carrello\n[3] Elimina Prodotto dal Carrello\n[4] Acquista Prodotti nel Carrello\n[5] Esci\n");
    scanf("%d",&Risp);
    printf("====================\n");
    // ---------------

    switch(Risp){
      case 1: // Stampa Carrello
        printf("\n====== CARRELLO ======\n");
        for (i = 0; i < size_Carrello; i++) {
          printf("nome neg: %s\n", Carrello[i].nome_neg);
          printf("nome prod: %s\n", Carrello[i].nome_prod);
          printf("\n");
        }
        printf("======================\n");
        break;
      case 2: // Inserisci prodotto nel carrello
        memset((void*)&ProdIns,0,20*sizeof(char));
        printf("\nInserisci il nome del Prodotto che vuoi Inserire nel Carrello: ");
        fflush(stdin);
        scanf("%s",ProdIns);

        // Controllo dell'esistenza del prodotto inserito
        for (i = 0, check = 0; i < size_ListaNeg; i++){
          if(strcmp(ListaNeg[i].nome_prod, ProdIns) == 0){
            check = 1;
            indiceProd = i;
          }
        }
        // ----------------------------------------------


        if(size_Carrello < 1 && check == 1){ // Se è il primo elemento allochiamo il Carrello
            Carrello = (Trasporto *)calloc(1, sizeof(Trasporto));
        } else if (size_Carrello >= 1 && check == 1){ // Altrimenti reallochiamo il Carrello
            Carrello = (Trasporto *)realloc(Carrello, (size_Carrello+1)*sizeof(Trasporto));
        }

        // Se esiste il prodotto, lo inseriamo nel Carrello
        if(check == 1){
          Carrello[size_Carrello].flag = 0;
          Carrello[size_Carrello].id_neg = ListaNeg[indiceProd].id_neg;
          Carrello[size_Carrello].id_client = ListaNeg[indiceProd].id_client;
          strcpy(Carrello[size_Carrello].nome_neg,ListaNeg[indiceProd].nome_neg);
          strcpy(Carrello[size_Carrello].nome_prod,ListaNeg[indiceProd].nome_prod);
          size_Carrello++;
          printf("Prodotto aggiunto con successo!\n");
        } else if (check == 0) {
          printf("Il prodotto inserito non esiste!\n");
        }

        break;
      case 3: // Elimina dal Carrello
        memset((void*)&ProdIns,0,20*sizeof(char));
        printf("\nInserisci il nome del Prodotto che vuoi Eliminare dal Carrello: ");
        fflush(stdin);
        scanf("%s",ProdIns);

        // Controllo dell'esistenza del prodotto inserito
        for(i = 0, check = 0; i < size_Carrello; i++){
          if(strcmp(Carrello[i].nome_prod,ProdIns) == 0)
            check = 1;
        }
        if (check == 0) { // Se non esiste
          printf("Il prodotto che vuoi eliminare non esiste nel carrello!\n");
        } else { // Se esiste
          // Allochiamo in appoggio la stessa struttura con un elemento in meno
          size_Carrello--;
          App = (Trasporto *)calloc(size_Carrello, sizeof(Trasporto));
          // Cerchiamo l'elemento da eliminare
          for(i = 0; i < size_Carrello; i++){
            if(strcmp(Carrello[i].nome_prod,ProdIns) == 0)
              Carrello[i].flag = -1;
          }
          // Avendo sesstato la flag a -1 per l'elemento in questione
          // Inseriremo nella struttura di appoggio i restanti elementi
          for(i = 0; i < size_Carrello; i++){
            if(Carrello[i].flag > 0){
              App[size_Carrello].id_neg = Carrello[i].id_neg;
              App[size_Carrello].id_client = Carrello[i].id_client;
              strcpy(App[size_Carrello].nome_neg, Carrello[i].nome_neg);
              strcpy(App[size_Carrello].nome_prod, Carrello[i].nome_prod);
            }
          }
          // Infine deallochiamo il vecchio puntatore al carrello
          // e ne creiamo uno nuovo, deallocando la struttura di appoggio
          free(Carrello);
          Carrello = (Trasporto *)calloc(size_Carrello, sizeof(Trasporto));
          Carrello = App;
          free(App);
          printf("Prodotto eliminato correttamente!\n");
        }

      break;

      case 4: // Acquisto dei prodotti nel carrello
        // Variabile di controllo settata a 2 per far
        // riconoscere al server C l'intenzione di controlre un acquisto
        control = 2;
        write(sock, &control, sizeof(int));
        // Inviamo il carrello
        write(sock,&size_Carrello,sizeof(int));
        for(i = 0; i < size_Carrello; i++){
          write(sock,&Carrello[i],sizeof(Trasporto));
        }

        // Riceviamo il carrello con le flag cambiate
        // in 1 se il prodotto è ancora esistente nel database
        // in -1 se è stato prima dell'acquisto
        for(i = 0; i < size_Carrello; i++){
          read(sock,&Carrello[i],sizeof(Trasporto));
        }
        for(i = 0, check = 0; i < size_Carrello; i++){
          if(Carrello[i].flag == -1){
            printf("Il seguente prodotto non è più disponibile: %s\n", Carrello[i].nome_prod);
            check = 1;
          }
        }

        if(check == 1)
          printf("Acquisto fallito!\n");
        else
          printf("Acquisto andato a buon fine!\n");

        // Reset del carrello
        free(Carrello);
        size_Carrello = 0;
      break;
      case 5:
        exit(0);
      break;
    }
    // La Lista dei negozi viene deallocata ed allocata ad ogni passo
    free(ListaNeg);
  }
  return 0;
}

void prodotti_aggiornati()
{
  // Ricezione della lista dei negozi
  int control = 1;
  write(sock, &control, sizeof(int));
  read(sock, &size_ListaNeg, sizeof(int));
  ListaNeg = (Trasporto *)calloc(size_ListaNeg, sizeof(Trasporto));
  for(i = 0; i < size_ListaNeg; i++){
    read(sock, &ListaNeg[i], sizeof(Trasporto));
  }
  // Stampa
  printf("\n===== STAMPA DEI NEGOZI =====\n");
  for(i = 0; i < size_ListaNeg; i++){
    printf("id client: %d\n",ListaNeg[i].id_client);
    printf("id neg: %d\n", ListaNeg[i].id_neg);
    printf("nome neg: %s\n",ListaNeg[i].nome_neg);
    printf("nome prod: %s\n", ListaNeg[i].nome_prod);
    printf("\n");
  }
  printf("===============================\n");
}
