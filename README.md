# Negozio

## Descrizione del progetto
Si vuole realizzare un sistema per la gestione di negozi virtuali costituito dalle seguenti entità:
- ServerM: mantiene la lista dei negozi virtuali e dei prodotti di ogni ne- gozio virtuale. Interagisce con ServerN e ServerC.
- ServerN: consente ai ClientN di operare sul ServerM. In particolare con- sente di creare un nuovo negozio virtuale, eliminare un negozio virtuale ed aggiungere ed eliminare prodotti da un negozio virtuale.
- ServerC: consente ai Client di operare sul ServerM. In particolare con- sente di ricevere l’elenco dei negozi virtuali, ricevere l’elenco dei prodotti di un negozio virtuale e ricercare un prodotto in un negozio virtuale.
- ClientN: consente al negoziante di gestire i propri negozi virtuali (ed i relativi prodotti) memorizzati sul ServerM, usando il ServerN come tramite. Ogni negoziante pu‘o gestire piu‘ negozi virtuali.
- Client: consente all’utente di interagire con i negozi virtuali memoriz- zati sul ServerM usando il ServerC come tramite. In particolare, consente all’utente di inserire i prodotti contenuti in diversi negozi virtuali in una lista di acquisti e di visualizzare la lista di acquisti.
<img width="839" alt="Diagramma" src="https://user-images.githubusercontent.com/46711940/112025062-15c9b400-8b35-11eb-83eb-6f86a4603991.png">

## Protocollo Applicazione
Il client e il server si scambiano il pacchetto formato dai seguenti campi: 
- flag: variabile utilizzata per fare controlli e gestire le varie operazioni in base a ciò che vuole fare il client;
- id-client: identifica l’id del cliente che si vuole connettere ed è unico;
- id-neg: chiave che identifica il negozio;
- nome-neg: stringa in cui si memorizza il nome del negozio; 
- nome-prod: stringa che memorizza il nome del prodotto.
<img width="514" alt="ProtocolloApplicazione" src="https://user-images.githubusercontent.com/46711940/112025548-9092cf00-8b35-11eb-94af-d9d15293243c.png">

## Dettagli Implementativi
### Lato Server
Il ServerM è il server principale che mantiene la lista dei negozi e dei prodotti e comunica solo tramite i due server sottostanti, ovvero il ServerN e il ServerC. Il ServerN si occupa di comunicare con i negozianti che vogliono apportare modifiche ai loro negozi e prodotti. Il ServerC si occupa di interagire con gli utenti che visualizzano la lista dei prodotti e dei negozi. Il server principale è quello che aggiorna le due liste mentre i due server fanno da tramite tra gli utenti e il ServerM. Tra i server si è deciso di utilizzare il protocollo UDP in quanto si vuole che le informazioni arrivino in modo rapido. Poiché i server devono gestire più client si è deciso di utilizzare dei thread, ovvero dei processi che avvengono in parallelo in modo tale da poter comunicare con più utenti contemporaneamente e I/O Multiplexing per gestire al meglio i vari utenti.
