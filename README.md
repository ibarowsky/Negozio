# Negozio

1 Descrizione del progetto
Si vuole realizzare un sistema per la gestione di negozi virtuali costituito dalle seguenti entità:
- ServerM: mantiene la lista dei negozi virtuali e dei prodotti di ogni ne- gozio virtuale. Interagisce con ServerN e ServerC.
- ServerN: consente ai ClientN di operare sul ServerM. In particolare con- sente di creare un nuovo negozio virtuale, eliminare un negozio virtuale ed aggiungere ed eliminare prodotti da un negozio virtuale.
- ServerC: consente ai Client di operare sul ServerM. In particolare con- sente di ricevere l’elenco dei negozi virtuali, ricevere l’elenco dei prodotti di un negozio virtuale e ricercare un prodotto in un negozio virtuale.
- ClientN: consente al negoziante di gestire i propri negozi virtuali (ed i relativi prodotti) memorizzati sul ServerM, usando il ServerN come tramite. Ogni negoziante pu‘o gestire piu‘ negozi virtuali.
- Client: consente all’utente di interagire con i negozi virtuali memoriz- zati sul ServerM usando il ServerC come tramite. In particolare, consente all’utente di inserire i prodotti contenuti in diversi negozi virtuali in una lista di acquisti e di visualizzare la lista di acquisti.
