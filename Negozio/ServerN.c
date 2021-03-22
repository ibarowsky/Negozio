//
//  main.c
//  Prova
//
//  Created by Fabio Barone on 13/02/2020.
//  Copyright © 2020 Fabio Barone. All rights reserved.
//

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
// Dichiarazioni variabili
int fd_open[FD_SETSIZE];
fd_set fset;
int check, max_fd, sock_client, list_fd, size_trasp;
struct sockaddr_in in_c, addr_client;
unsigned int len_c = sizeof(in_c);
unsigned int len_client = sizeof(addr_client);
int sock_server, num = 1;
struct sockaddr_in in_s, addr_server, addr_n;
unsigned int len_s = sizeof(in_s);
unsigned int len_server = sizeof(addr_server);
unsigned int len_n = sizeof(addr_n);

short i = 0, j = 0;
Trasporto *A, *B;

//---------------------------------------------------------------------------

int main(int argc, const char *argv[]) {
  if ((sock_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Errore creazione Socket Server");
    exit(1);
  }
  in_s.sin_family = AF_INET;
  in_s.sin_port = htons(1040);
  if (inet_pton(AF_INET, argv[1], &in_s.sin_addr) < 0) {
    fprintf(stderr, "inet_pton error for %s\n", argv[1]);
    exit(1);
  }
  if (sendto(sock_server, &num, sizeof(int), 0, (struct sockaddr *)&in_s, len_s) < 0) {
    perror("errore nella sendto del size 2");
    exit(-1); // ricevo size array da M
  }

  pthread_create(&thread_comunicazione, NULL, comunicazione, NULL);
  while (1) {
    pthread_mutex_lock(&mutex_file);
    if (recvfrom(sock_server, &size_trasp, sizeof(int), 0,
                 (struct sockaddr *)&addr_n, &len_n) < 0) {
      perror("Errore invio pacchetto al server N");
      exit(-1);
    }

    if (size_trasp > 0) {
      A = (Trasporto *)calloc(size_trasp, sizeof(Trasporto));
      if (recvfrom(sock_server, A, size_trasp * sizeof(Trasporto), 0,
                   (struct sockaddr *)&addr_n, &len_n) < 0) {
        perror("Errore invio pacchetto al server N");
        exit(-1);
      }
      pthread_mutex_unlock(&mutex_descriptor);
      for (i = 0; i < size_trasp; i++) {
        printf("id client: %d\n", A[i].id_client);
        printf("id neg: %d\n", A[i].id_neg);
        printf("nome neg: %s\n", A[i].nome_neg);
        printf("nome prod: %s\n", A[i].nome_prod);
        printf("\n");
      }
    }
  }

  pthread_join(thread_comunicazione, NULL);
  exit(0);
}

void *comunicazione(void *arg) {
  int n, max = 0;
  struct timeval td;
  pthread_mutex_lock(&mutex_descriptor);
  int i, size_neg_client;
  B = (Trasporto *)calloc(1, sizeof(Trasporto));
  if ((list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Errore creazione Socket Client");
    exit(1);
  }
  in_c.sin_family = AF_INET;
  in_c.sin_addr.s_addr = htonl(INADDR_ANY);
  in_c.sin_port = htons(1031);

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
  // pthread_mutex_lock(&mutex_descriptor);

  while (1) {
    printf("WHILE\n");
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

    n = select(max_fd + 1, &fset, NULL, NULL, NULL);

    // Se il descrittore della listen è diverso da 0
    if (FD_ISSET(list_fd, &fset)) {
      // Accetto la connessione dal client
      sock_client = accept(list_fd, (struct sockaddr *)&in_c, &len_c);
      fd_open[sock_client] = 1;
      read(sock_client, B, sizeof(Trasporto));
      for (j = 0, size_neg_client = 0; j < size_trasp; j++) {
        if (A[j].id_client == B->id_client)
          size_neg_client++;
      }
      write(i, &size_neg_client, sizeof(int));

      for (j = 0; j < size_trasp; j++) {
        if (A[j].id_client == B->id_client)
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
        read(i, B, sizeof(Trasporto));

        if (B->flag == 0) {

          pthread_mutex_lock(&mutex_descriptor);
          for (j = 0, size_neg_client = 0; j < size_trasp; j++) {
            if (A[j].id_client == B->id_client)
              size_neg_client++;
          }
          write(i, &size_neg_client, sizeof(int));

          for (j = 0; j < size_trasp; j++) {
            if (A[j].id_client == B->id_client)
              write(i, &A[j], sizeof(Trasporto));
          }

        } else {
          // Ricevi cosa vuole fare il client

          // Fare uno switch per gestire i vari casi che vuole fare il
          // negoziante Rimane bloccato nello switch durante la ricezione dei
          // dati e di conseguenza rimane attivo il suo descrittore
          switch (B->flag) {
          case 1:
            max = A[0].id_neg;
            printf("Dopo max\n");
            for (i = 1; i < size_trasp; i++) {
              if (A[i].id_neg > max)
                max = A[i].id_neg;
            }
            B->id_neg = max + 1;

            if (sendto(sock_server, B, sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, len_n) < 0) {
              perror("Errore invio pacchetto al server N");
              exit(-1);
            }
            pthread_mutex_unlock(&mutex_file);
            break;

          case 2:
            if (sendto(sock_server, B, sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, len_n) < 0) {
              perror("Errore invio pacchetto al server N");
              exit(-1);
            }
            pthread_mutex_unlock(&mutex_file);
            break;

          case 3:
            if (sendto(sock_server, B, sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, len_n) < 0) {
              perror("Errore invio pacchetto al server N");
              exit(-1);
            }
            printf("Inviato! case 3\n");
            pthread_mutex_unlock(&mutex_file);
            break;

          case 4:
            if (sendto(sock_server, B, sizeof(Trasporto), 0, (struct sockaddr *)&addr_n, len_n) < 0) {
              perror("Errore invio pacchetto al server N");
              exit(-1);
            }
            printf("Inviato! case 4\n");
            pthread_mutex_unlock(&mutex_file);
            break;
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
