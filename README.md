# Backup Software PDS

## Network Application Protocol
1. Invio credenziali
1. Il server risponde
    - "OK" credenziali accettate
    - "KO" credenziali rifiutate e chiude la conessione
1. Invio file
    1. Invio un JSON della classe SyncedFile contenente
        - Path
        - Hash
        - Dimensione del file
        - È un file regolare (false se cartella o il file non esiste)
    1. Attendo la risposta del server e si possono verificare due casi
        - "OK" -> Il file è già presente sul server e il client non deve fare altro
        - "NO" -> Il file non è presente sul server quindi bisogna procedere nel seguente modo:
            1. Invio del file binario
            2. Attendo la risposta che può essere:
                - "OK" caricamento completato, il client non deve fare altro
                - "KO" si sono verificati errori, il client deve rieffettuare l'invio a partire dal punto 3.1

ATTENZIONE: se il file viene modificato durante l'upload con un file di dimensione minore si rischia che non funzioni più nulla, quindi va gestito anche un timeout lato server e un controllo dell'hash per verificare che il file sia corretto e quello atteso.