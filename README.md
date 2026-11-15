# gaming chair

## Offset dinamici

Gli offset di memoria non sono più costanti compilate: vengono risolti a runtime
come fanno i progetti tipo *cs2-offsets*, così il cheat sopravvive agli
aggiornamenti del gioco senza dover essere ricompilato.

### Come funziona

`offsets::resolve()` (chiamato all'avvio in `src/main.cpp`) cerca `offsets.json`
in tre posizioni, dalla meno alla più specifica:

1. **directory di lavoro** (da dove è stato lanciato il programma);
2. **radice del progetto** (un livello sopra `build\`, dove il repository tiene
   `offsets.json`: è il caso di quando lanci l'exe da Esplora file);
3. **accanto all'eseguibile** (`build\offsets.json`, override per installazione).

I file vengono applicati in quest'ordine, quindi un valore nel file più
specifico vince; i file vengono letti una volta sola anche se due percorsi
puntano allo stesso file. Un valore mancante o non valido (0, stringa non
numerica) **non** viene applicato: resta il fallback compilato. All'avvio la
console stampa cosa è stato usato, per esempio:

```
[offsets] 21/21 resolved from 1 source(s), last: C:\...\gaming-chair\offsets.json
[offsets] dwEntityList=0x2709010 dwLocalPlayerPawn=0x2553D38 dwViewMatrix=0x2559640 m_iHealth=0x34C entityListEntrySize=0x70
```

Se il file non viene trovato compare `no offsets.json found ... using the
compiled-in defaults`. Dopo aver modificato il file basta riavviare il
programma: **nessuna ricompilazione**.

In alternativa si può definire a build time la macro `OFFSETS_URL` per scaricare
gli offset da un URL:

   ```
   /DOFFSETS_URL="https://raw.githubusercontent.com/<utente>/<repo>/main/offsets.json"
   ```

### Formato JSON

Oggetto piatto di coppie `"nome": valore`, dove il valore può essere un numero
decimale, un numero esadecimale (con o senza virgolette) o una stringa:

```json
{
    "dwLocalPlayerPawn": 25581640,
    "dwEntityList": "0x1A146E8",
    "m_iHealth": 844,
    "entityListEntrySize": "0x70"
}
```

Oltre ai campi dello schema (`dw*` e `m_*`) sono risolvibili anche i valori
strutturali che di solito cambiano con gli aggiornamenti del gioco:

| nome | significato | default |
| --- | --- | --- |
| `entityListEntrySize` | passo di una entry della entity list | `0x70` |
| `entityListChunkOffset` | offset dell'array di chunk in `CGameEntitySystem` | `0x10` |
| `entityListChunkSize` | entità per chunk | `512` |
| `modelStateBoneArray` | campo di `CModelState` con il puntatore ai bone | `0x80` |

Vengono accettati anche JSON annidati (per esempio `offsets.json` e
`client_dll.json` prodotti da [a2x/cs2-dumper](https://github.com/a2x/cs2-dumper)),
perché il resolver cerca semplicemente ogni nome noto come chiave JSON. Se un
nome compare più volte, viene usata la prima occorrenza valida: per i nomi
ambigui è quindi preferibile usare il file piatto `offsets.json` con i valori
esatti.

## Server di configurazione

All'avvio il programma apre un piccolo server HTTP su
`http://127.0.0.1:<SERVER_PORT>/` (porta predefinita 12345). La pagina permette
di modificare la configurazione; i valori vengono validati prima di essere
applicati e le modifiche vengono salvate in `src\server\settings.bin`.

* Un valore mancante o non valido (per esempio un campo numerico svuotato)
  viene rifiutato con `HTTP 400` e un messaggio che indica il campo: la
  richiesta non fa più terminare il programma.
* Le risposte includono `Content-Length` e `Connection: close`, quindi il
  browser non resta in attesa di una connessione ancora aperta.
* `SERVER_PORT` e `DISPLAY_*` richiedono il riavvio del programma; le altre
  impostazioni si applicano al frame successivo.

