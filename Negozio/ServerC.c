#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define BACKLOG 10
#define MAXLINE 1024

typedef struct prodotti {
  char nome_prod[20];
  int id_neg;
} Prodotto;
typedef struct negozio {
  int id_client;
  int id_neg;
  char nome_neg[20];
} Negozio;

typedef struct trasporto {
  int flag;
  int id_client;
  int id_neg;
  char nome_neg[20];
  char nome_prod[20];
} Trasporto;

pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_descriptor = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_comunicazione;
void *comunicazione(void *arg);
//-----------------------------------------------------------------------------
// Dichiarazioni variabili Globali
int fd_open[FD_SETSIZE];
fd_set fset;
int max_fd;
int sock_client, list_fd;
struct sockaddr_in in_c, addr_client;
unsigned int len_c = sizeof(in_c);
unsigned int len_client = sizeof(addr_client);
int sock_server, n = 10, num = 1, id_client, size_string_neg;
struct sockaddr_in in_s, addr_server, addr_n;
unsigned int len_s = sizeof(in_s);
unsigned int len_server = sizeof(addr_server);
unsigned int len_n = sizeof(addr_n);
int size_trasp;
short i = 0, j = 0,k = 0;
Trasporto *A, *B;
int size_Carrello, check;
//---------------------------------------------------------------------------

int main(int argc, const char *argv[]) {
  if ((sock_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Errore creazione Socket Server");
    exit(1);
  }
  in_s.sin_family = AF_INET;
  in_s.sin_port = htons(1045);
  if (inet_pton(AF_INET, argv[1], &in_s.sin_addr) < 0) {
    fprintf(stderr, "inet_pton error for %s\n", argv[1]);
    exit(1);
  }

  if (sendto(sock_server, &num, sizeof(int), 0, (struct sockaddr *)&in_s, len_s) < 0) {
    perror("errore nella sendto");
    exit(-1);
  }

  // Creazione del thread che gestisce la comunicazione con i clientC
  pthread_create(&thread_comunicazione, NULL, comunicazione, NULL);
  while (1) {
    // Ciclo che legge la lista dei negozi
    if (recvfrom(sock_server, &size_trasp, sizeof(int), 0, (struct sockaddr *)&addr_n, &len_n) < 0) {
      perror("Errore nella recv");
      exit(-1);
    }

    if (size_trasp > 0) {
      A = (Trasporto *)calloc(size_trasp, sizeof(Trasporto));
      if (recvfrom(sock_server, A, size_trasp * sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, &len_n) < 0) {
        perror("Errore nella recv");
        exit(-1);
      }
      pthread_mutex_unlock(&mutex_descriptor);
    }
  }

  pthread_join(thread_comunicazione, NULL);
  exit(0);
}

void *comunicazione(void *arg) {
  int n, i, control;
  struct timeval td;
  pthread_mutex_lock(&mutex_descriptor);
  B = (Trasporto *)calloc(1, sizeof(Trasporto));
  if ((list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Errore creazione Socket Client");
    exit(1);
  }
  in_c.sin_family = AF_INET;
  in_c.sin_addr.s_addr = htonl(INADDR_ANY);
  in_c.sin_port = htons(1030);

  if (bind(list_fd, (struct sockaddr *)&in_c, sizeof(in_c)) < 0) {
    perror("Errore bind Client");
    exit(1);
  }
  if (listen(list_fd, 1024) < 0) {
    perror("listen");
    exit(1);
  }
  // Imposto il massimo dell'array dei descrittori al descrittore di ascolto
  max_fd = list_fd;
  // Assegno 1 per dire che è disponibile
  fd_open[max_fd] = 1;

  while (1) {
    // Assegnazione di un timer per la select
    td.tv_sec = 2;
    td.tv_usec = 0;

    // Azzeramento del fset
    FD_ZERO(&fset);
    // Ciclo per andare a controllare i descrittori
    for (i = list_fd; i <= max_fd; i++) {
      printf("%d\n", i);
      // Se c'è uno disponibile lo vado a settare
      if (fd_open[i] != 0)
        FD_SET(i, &fset);
    }
    // La select controlla tutti i descrittori pronti
    n = select(max_fd + 1, &fset, NULL, NULL, NULL);

    // Se il descrittore della listen è diverso da 0
    if (FD_ISSET(list_fd, &fset)) {
      // Accetto la connessione dal client
      sock_client = accept(list_fd, (struct sockaddr *)&in_c, &len_c);
      printf("sock_client = %d\n", sock_client);
      fd_open[sock_client] = 1;

      read(sock_client, &control, sizeof(int));
      write(i, &size_trasp, sizeof(int));

      for (j = 0; j < size_trasp; j++) {
        write(i, &A[j], sizeof(Trasporto));
      }

      if (max_fd < sock_client) {
        max_fd = sock_client;
      }

      n--;
    }

    i = list_fd + 1;
    while (n != 0) {
      if (FD_ISSET(i, &fset)) {
        // Lettura della variabile di controllo
        read(i, &control, sizeof(int));
        if(control == 1){
          // Se control è 1, al Client bisognerà
          // mandare la lista aggiornata dei negozi e dei prodotti
          printf("Invio negozi.\n");
          write(i, &size_trasp, sizeof(int));

          for (j = 0; j < size_trasp; j++) {
            write(i, &A[j], sizeof(Trasporto));
          }
        } else if (control == 2){
          // Altrimenti si tenta di fare un acquisto
          printf("Prova acquisto.\n");
          read(i, &size_Carrello, sizeof(int));
          // Leggendo prima il carrello
          for(j = 0; j < size_Carrello; j++){
            read(i, &B[j], sizeof(Trasporto));
          }
          // E controlliamo se eistono nel negozio
          for(k = 0; k < size_trasp; k++){
            for(j = 0; j < size_Carrello; j++){
              if(strcmp(A[k].nome_prod, B[j].nome_prod) == 0){
                // Se si, li settiamo a 1
                B[j].flag = 1;
              }
            }
          }
          // Se non esistono si settano a -1 per essere poi
          // Riconosciuti dal Client C e notificati al cliente
          for(j = 0; j < size_Carrello; j++){
            if(B[j].flag < 1)
              B[j].flag = -1;
          }
          // Inviamo il carrello restituendo i valori delle flag modificati
          for(j = 0; j < size_Carrello; j++){
            write(i, &B[j], sizeof(Trasporto));
          }
        }
        n--;
      }
      FD_CLR(i, &fset);
      i++;
    }

  }



  return 0;
}
