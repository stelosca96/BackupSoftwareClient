# Backup Software PDS

## Network Application Protocol

1. JSON della classe Message contenente
    - Path
    - Hash
    - Dimensione del file
1. Seguito dal file binario della dimensione precedentemente contrassegnata

ATTENZIONE: se il file viene modificato durante l'upload con un file di dimensione minore si rischia che non funzioni più nulla, quindi va gestito anche un timeout lato server e un controllo dell'hash per verificare che il file sia corretto e quello atteso.