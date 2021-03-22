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
#define SERVER_PORT 1025
#define BACKLOG 10
#define MAXLINE 1024

void* cliente (void* arg);
void* negozio (void* arg);

typedef struct negozio{
  int id_client;
  int id_neg;
  char nome_neg[20];
}Negozio;

typedef struct prodotto{
  int id_neg;
  char nome_prod[20];
}Prodotto;

typedef struct trasporto {
  int flag;
  int id_client;
  int id_neg;
  char nome_neg[20];
  char nome_prod[20];
}Trasporto;

pthread_t thread_negozio;
pthread_t thread_cliente;

pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;


// Dichiarazioni variabili globali
int sock_neg, sock_cl;
struct sockaddr_in addr_c,addr_n, in_neg,in_cl, tempaddr;
unsigned int len=sizeof(tempaddr);
unsigned int len_c = sizeof(addr_c);
unsigned int len_n = sizeof(addr_n);



int main(int argc, const char * argv[]) {
  if ( ( sock_neg = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
  }
  in_neg.sin_family      = AF_INET;
  in_neg.sin_addr.s_addr = htonl(INADDR_ANY);
  in_neg.sin_port        = htons(1040);
  if ( bind(sock_neg, (struct sockaddr *) &in_neg, sizeof(in_neg)) < 0 ) {
    perror("bind");
    exit(1);
  }
  if ( ( sock_cl = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
  }
  in_cl.sin_family      = AF_INET;
  in_cl.sin_addr.s_addr = htonl(INADDR_ANY);
  in_cl.sin_port        = htons(1045);
  if ( bind(sock_cl, (struct sockaddr *) &in_cl, sizeof(in_cl)) < 0 ) {
    perror("bind");
    exit(1);
  }


  pthread_create (&thread_negozio,NULL,negozio,NULL);
  pthread_create (&thread_cliente,NULL,cliente,NULL);

  pthread_join(thread_negozio,NULL);
  pthread_join(thread_cliente,NULL);

}

void* negozio (void* arg)
{
  Negozio *Negozio_App;
  Prodotto *Prodotto_App;
  Trasporto *A, *B;
  int i, j, k;
  FILE  *Pf1,*Pf2;
  int size_neg=0,size_prod=0, size_finale=0;
  int flag;
  int app_size_prod, app_size_neg;

  if(recvfrom(sock_neg, &flag, sizeof(int), 0, (struct sockaddr *)&addr_n, &len_n)<0){
      perror("Errore ricezione int protocollo.\n");
      exit(-1);
  }
  while(1){
    // LETTURA DA FILE
    Pf1 = fopen("Negozi.rfm", "rb");
    if(Pf1){
      fseek(Pf1, 0, SEEK_SET);
      fread(&size_neg, sizeof(int), 1, Pf1); printf("Il size e: %d\n\n", size_neg);
      /*fseek(Pf1, sizeof(int)*sizeof(Negozio), SEEK_SET);*/
      Negozio_App = (Negozio *)calloc(size_neg, sizeof(Negozio));
      for(i = 0; i < size_neg; i++){
        fread(&Negozio_App[i], sizeof(Negozio),1,Pf1);
        printf("Dopo Apertura file, Neg: %s\n", Negozio_App[i].nome_neg);
        printf("Dopo Apertura file, Id Neg: %d\n", Negozio_App[i].id_neg);
        printf("Dopo Apertura file, Id client: %d\n", Negozio_App[i].id_client);
      }
      fclose(Pf1);
    }
    else
      printf("Il seguente id client non ha negozi.\n");

    Pf2 = fopen("Prodotti.rfm", "rb");
    if(Pf2){
      fseek(Pf2, 0, SEEK_SET);
      fread(&size_prod, sizeof(int), 1, Pf2); printf("Il size e: %d\n\n", size_prod);
      /*fseek(Pf2, sizeof(int)*sizeof(Prodotto), SEEK_SET);*/
      Prodotto_App = (Prodotto *)calloc(size_prod, sizeof(Prodotto));
      for(i = 0; i < size_prod; i++){
        fread(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
        printf("Dopo Apertura file, Prod: %s\n", Prodotto_App[i].nome_prod);
        printf("Dopo Apertura file, Id Neg: %d\n", Prodotto_App[i].id_neg);
      }
      fclose(Pf2);
    }
    else{
      printf("Errore durante l’apertura del file.\n");
    }
    // =================


    // ======================= COSTRUZIONE DELLA STRUCT FINALE =======================
    if(size_neg >= size_prod)
      size_finale = size_neg;
    else
      size_finale = size_prod;

    A = (Trasporto *)calloc(size_finale, sizeof(Trasporto));


    for(i = 0, k = 0; i < size_neg; i++){
      for(j = 0; j < size_prod; j++){
        if(Negozio_App[i].id_neg == Prodotto_App[j].id_neg){
          A[k].id_client = Negozio_App[i].id_client;
          A[k].id_neg = Negozio_App[i].id_neg;
          strcpy(A[k].nome_neg, Negozio_App[i].nome_neg);
          strcpy(A[k].nome_prod, Prodotto_App[j].nome_prod);
          A[k].flag = 0;
          k++;
        }
      }
    }

    // =========================== Invio Negozi e Prodotti ============================
    if(sendto(sock_neg, &size_finale, sizeof(int), 0, (struct sockaddr *)&addr_n, len_n)<0){
        perror("Errore invio size al server N");
        exit(-1);
    }//size negozio
    if (size_finale > 0) {
      if(sendto(sock_neg, A, size_finale*sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, len_n)<0){
          perror("Errore invio pacchetto al server N");
          exit(-1);
      }
    }
    for(i = 0; i < size_finale; i++){
      printf("id client: %d\n",A[i].id_client);
      printf("id neg: %d\n", A[i].id_neg);
      printf("nome neg: %s\n",A[i].nome_neg);
      printf("nome prod: %s\n", A[i].nome_prod);
      printf("\n");
    }
    free(A);
    // ===============================================================================

    printf("In attesa della struct da scrivere..\n");

    // =========================== RICEZIONE STRUCT FINALE ===========================
    B = (Trasporto *)calloc(1, sizeof(Trasporto));
    if(recvfrom(sock_neg, B, sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, &len_n)<0){
        perror("Errore invio pacchetto al server N");
        exit(-1);
    }
    // ===============================================================================
    printf("Ricevuta!\n");
    switch (B->flag) {
      case 1: // Inserisci Negozio e primo prodotto
        printf("Caso 1 Server M\n");
        size_neg++;
        Negozio_App = (Negozio *)realloc(Negozio_App, size_neg*sizeof(Negozio));
        Negozio_App[size_neg-1].id_neg = B->id_neg;
        Negozio_App[size_neg-1].id_client = B->id_client;
        strcpy(Negozio_App[size_neg-1].nome_neg, B->nome_neg);

        size_prod++;
        Prodotto_App = (Prodotto *)realloc(Prodotto_App, size_prod*sizeof(Prodotto));
        Prodotto_App[size_prod-1].id_neg = B->id_neg;
        strcpy(Prodotto_App[size_prod-1].nome_prod, B->nome_prod);

        for(i = 0; i < size_neg; i ++){
          printf("id client: %d\n",Negozio_App[i].id_client);
          printf("id neg: %d\n", Negozio_App[i].id_neg);
          printf("nome neg: %s\n",Negozio_App[i].nome_neg);
          printf("\n");
        }
        for(i = 0; i < size_prod; i ++){
          printf("id neg: %d\n", Prodotto_App[i].id_neg);
          printf("nome prod: %s\n", Prodotto_App[i].nome_prod);
          printf("\n");
        }
        pthread_mutex_lock(&mutex_file);
        Pf1 = fopen("Negozi.rfm", "wb");
        if(Pf1){
          fseek(Pf1, 0, SEEK_SET);
          fwrite(&size_neg, sizeof(int), 1, Pf1);
          for(i = 0; i < size_neg; i++){
            //fseek(Pf1, size_neg*sizeof(Negozio)+sizeof(int), SEEK_SET);
            fwrite(&Negozio_App[i], sizeof(Negozio),1,Pf1);
          }
          fclose(Pf1);
        }
        else
          printf("Errore durante l’apertura del file.\n");

        Pf2 = fopen("Prodotti.rfm", "wb");
        if(Pf2){
          fseek(Pf2, 0, SEEK_SET);
          fwrite(&size_prod, sizeof(int), 1, Pf2);
          for(i = 0; i < size_prod; i++){
            //fseek(Pf2, size_prod*sizeof(Prodotto)+sizeof(int), SEEK_SET);
            fwrite(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
          }
          fclose(Pf2);
        }
        else
          printf("Errore durante l’apertura del file.\n");


        pthread_mutex_unlock(&mutex_file);

        break;

      case 2: // Inserisci Prodotto per un determinato Negozio
      printf("Caso 2 Server M\n");
      size_prod++;
      Prodotto_App = (Prodotto * )realloc(Prodotto_App,size_prod*sizeof(Prodotto));
      Prodotto_App[size_prod-1].id_neg = B->id_neg;
      strcpy(Prodotto_App[size_prod-1].nome_prod, B->nome_prod);
      pthread_mutex_lock(&mutex_file);
      Pf2 = fopen("Prodotti.rfm", "wb");
      if(Pf2){
        fseek(Pf2, 0, SEEK_SET);
        fwrite(&size_prod, sizeof(int), 1, Pf2);
        for(i = 0; i < size_prod; i++){
          //fseek(Pf2, size_prod*sizeof(Prodotto)+sizeof(int), SEEK_SET);
          fwrite(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
        }
        fclose(Pf2);
      }
      else
        printf("Errore durante l’apertura del file.\n");
      pthread_mutex_unlock(&mutex_file);

        break;

      case 3: //Elimina Negozio
      printf("Caso 3 Server M\n");
      app_size_neg = size_neg;
      app_size_prod = size_prod;
      for(i = 0; i < size_neg; i ++){
        if(Negozio_App[i].id_neg == B->id_neg){
          Negozio_App[i].id_neg = -1;
          app_size_neg --;
        }
      }
        for(i = 0; i < size_prod; i ++){
          if(Prodotto_App[i].id_neg == B->id_neg){
            Prodotto_App[i].id_neg = -1;
            app_size_prod --;
          }
      }
      pthread_mutex_lock(&mutex_file);
      Pf1 = fopen("Negozi.rfm", "wb");
      if(Pf1){
        fseek(Pf1, 0, SEEK_SET);
        fwrite(&app_size_neg, sizeof(int), 1, Pf1);
        for(i = 0; i < size_neg; i++){
          //fseek(Pf1, size_neg*sizeof(Negozio)+sizeof(int), SEEK_SET);
          if(Negozio_App[i].id_neg != -1)
            fwrite(&Negozio_App[i], sizeof(Negozio),1,Pf1);
        }
        fclose(Pf1);
        size_neg = app_size_neg;
      }
      else
        printf("Errore durante l’apertura del file.\n");


      Pf2 = fopen("Prodotti.rfm", "wb");
      if(Pf2){
        fseek(Pf2, 0, SEEK_SET);
        fwrite(&app_size_prod, sizeof(int), 1, Pf2);
        for(i = 0; i < size_prod; i++){
          //fseek(Pf2, size_prod*sizeof(Prodotto)+sizeof(int), SEEK_SET);
          if(Prodotto_App[i].id_neg != -1)
            fwrite(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
        }
        fclose(Pf2);
        size_prod = app_size_prod;
      }
      else
        printf("Errore durante l’apertura del file.\n");


      pthread_mutex_unlock(&mutex_file);

        break;
      case 4: //Elimina prodotto
      printf("Caso 4 Server M\n");
      app_size_prod = size_prod;
      for(i = 0; i < size_prod; i ++){
        if(strcmp(Prodotto_App[i].nome_prod, B->nome_prod)==0){
          Prodotto_App[i].id_neg = -1;
          app_size_prod --;
        }
      }
      pthread_mutex_lock(&mutex_file);
      Pf2 = fopen("Prodotti.rfm", "wb");
      if(Pf2){
        fseek(Pf2, 0, SEEK_SET);
        fwrite(&app_size_prod, sizeof(int), 1, Pf2);
        for(i = 0; i < size_prod; i++){
          //fseek(Pf2, size_prod*sizeof(Prodotto)+sizeof(int), SEEK_SET);
          if(Prodotto_App[i].id_neg != -1)
            fwrite(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
        }
        fclose(Pf2);
        size_prod = app_size_prod;
      }
      else
        printf("Errore durante l’apertura del file.\n");


      int check = 0;
      for(i = 0; i < size_prod; i ++){
        if(Prodotto_App[i].id_neg == B->id_neg)
          check = 1;
      }
      int app_size_neg = size_neg;
      if(check == 0){
        for(i = 0; i < size_neg; i ++){
          if(Negozio_App[i].id_neg == B->id_neg){
            Negozio_App[i].id_neg = -1;
            app_size_neg --;
          }
        }
      }
      printf("Errore durante l’apertura del file.\n");

      if(check==0){
        Pf1 = fopen("Negozi.rfm", "wb");
        if(Pf1){
          fseek(Pf1, 0, SEEK_SET);
          fwrite(&app_size_neg, sizeof(int), 1, Pf1);
          for(i = 0; i < size_neg; i++){
            //fseek(Pf1, size_neg*sizeof(Negozio)+sizeof(int), SEEK_SET);
            if(Negozio_App[i].id_neg != -1)
              fwrite(&Negozio_App[i], sizeof(Negozio),1,Pf1);
          }
          fclose(Pf1);
          size_neg = app_size_neg;
        }
        else
          printf("Errore durante l’apertura del file.\n");
      }



        pthread_mutex_unlock(&mutex_file);
        break;
    }

    free(B);
    free(Negozio_App);
    free(Prodotto_App);
  }
  return 0;
}

void* cliente (void* arg)
{
    FILE *Pf1,*Pf2;
    int size_neg,size_prod,size_finale, i,k,j,flag;
    Trasporto *A;
    Negozio *Negozio_App;
    Prodotto *Prodotto_App;

    printf("Secondo step");


    if(recvfrom(sock_cl, &flag, sizeof(int), 0, (struct sockaddr *)&addr_c, &len_c)<0){
        perror("Errore invio size al server N");
        exit(-1);
    }

    while(1){
      // Lettura dal file
      Pf1 = fopen("Negozi.rfm", "rb");
      if(Pf1){
        fseek(Pf1, 0, SEEK_SET);
        fread(&size_neg, sizeof(int), 1, Pf1); //printf("Il size e: %d\n\n", size_neg);
        /*fseek(Pf1, sizeof(int)*sizeof(Negozio), SEEK_SET);*/
        Negozio_App = (Negozio *)calloc(size_neg, sizeof(Negozio));
        for(i = 0; i < size_neg; i++){
          fread(&Negozio_App[i], sizeof(Negozio),1,Pf1);
          /*printf("Dopo Apertura file, Neg: %s\n", Negozio_App[i].nome_neg);
          printf("Dopo Apertura file, Id Neg: %d\n", Negozio_App[i].id_neg);
          printf("Dopo Apertura file, Id client: %d\n", Negozio_App[i].id_client);*/
        }
        fclose(Pf1);
      }
      else
        printf("Il seguente id client non ha negozi.\n");

      Pf2 = fopen("Prodotti.rfm", "rb");
      if(Pf2){
        fseek(Pf2, 0, SEEK_SET);
        fread(&size_prod, sizeof(int), 1, Pf2); //printf("Il size e: %d\n\n", size_prod);
        /*fseek(Pf2, sizeof(int)*sizeof(Prodotto), SEEK_SET);*/
        Prodotto_App = (Prodotto *)calloc(size_prod, sizeof(Prodotto));
        for(i = 0; i < size_prod; i++){
          fread(&Prodotto_App[i], sizeof(Prodotto),1,Pf2);
          /*printf("Dopo Apertura file, Prod: %s\n", Prodotto_App[i].nome_prod);
          printf("Dopo Apertura file, Id Neg: %d\n", Prodotto_App[i].id_neg);*/
        }
        fclose(Pf2);
      }
      else{
        printf("Errore durante l’apertura del file.\n");
      }
      // =================


      // ======================= COSTRUZIONE DELLA STRUCT FINALE =======================
      if(size_neg >= size_prod)
        size_finale = size_neg;
      else
        size_finale = size_prod;

      A = (Trasporto *)calloc(size_finale, sizeof(Trasporto));


      for(i = 0, k = 0; i < size_neg; i++){
        for(j = 0; j < size_prod; j++){
          if(Negozio_App[i].id_neg == Prodotto_App[j].id_neg){
            A[k].id_client = Negozio_App[i].id_client;
            A[k].id_neg = Negozio_App[i].id_neg;
            strcpy(A[k].nome_neg, Negozio_App[i].nome_neg);
            strcpy(A[k].nome_prod, Prodotto_App[j].nome_prod);
            A[k].flag = 0;
            k++;
          }
        }
      }
      free(Negozio_App);
      free(Prodotto_App);
      // =========================== Invio Negozi e Prodotti ============================
      if(sendto(sock_neg, &size_finale, sizeof(int), 0, (struct sockaddr *)&addr_c, len_c)<0){
          perror("Errore invio size al server N");
          exit(-1);
      }//size negozio
      if (size_finale > 0) {
        if(sendto(sock_neg, A, size_finale*sizeof(Trasporto), 0, (struct sockaddr *)&addr_c, len_c)<0){
            perror("Errore invio pacchetto al server N");
            exit(-1);
        }
      }

      free(A);
    }
    return 0;
}
