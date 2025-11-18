# DLT Timestamp Viewer v1.0

## Descrizione

Applicazione web standalone per l'analisi dei timestamp contenuti nei file DLT (Diagnostic Log and Trace).

## Caratteristiche

- ✅ **100% Client-Side**: Funziona completamente nel browser, nessun server richiesto
- ✅ **Supporto DLT v1 e v2**: Compatibile con entrambe le versioni del formato storage header
- ✅ **Parsing Veloce**: Estrazione efficiente dei timestamp da file DLT di qualsiasi dimensione
- ✅ **Interfaccia Intuitiva**: Drag & drop per caricare i file
- ✅ **Ricerca in Tempo Reale**: Filtra i risultati per data, ora o ECU ID
- ✅ **Esportazione CSV**: Esporta i dati in formato CSV per ulteriori analisi
- ✅ **Statistiche**: Visualizza statistiche aggregate (totale messaggi, primo/ultimo timestamp, durata)

## Come Usare

### Metodo 1: Apertura Diretta nel Browser

1. Apri il file `dlt-timestamp-viewer.html` con un browser moderno (Chrome, Firefox, Edge, Safari)
2. Trascina un file `.dlt` nell'area di upload oppure clicca per selezionarlo
3. Attendi il parsing (pochi secondi anche per file grandi)
4. Visualizza i risultati nella tabella

### Metodo 2: Server Locale (opzionale)

```bash
# Con Python 3
python3 -m http.server 8000

# Con Node.js
npx http-server

# Poi apri nel browser
# http://localhost:8000/dlt-timestamp-viewer.html
```

## Funzionalità

### 📊 Statistiche Visualizzate

- **Messaggi Totali**: Numero totale di messaggi DLT con timestamp
- **Primo Timestamp**: Data e ora del primo messaggio
- **Ultimo Timestamp**: Data e ora dell'ultimo messaggio
- **Durata Totale**: Intervallo di tempo coperto dal log

### 🔍 Ricerca

Filtra i messaggi cercando per:
- Data (es: `2024`, `2024/01/15`)
- Ora (es: `14:30`, `14:30:45`)
- ECU ID (es: `ECU1`, `HMIAPP`)
- Timestamp Unix (es: `1705334400`)

### 📥 Esportazione

Esporta i timestamp in formato CSV con le seguenti colonne:
- Index
- Data e Ora
- Timestamp (secondi Unix)
- Microsecondi
- ECU ID
- Dimensione messaggio (bytes)

## Dati Estratti

Per ogni messaggio DLT, l'applicazione estrae:

| Campo | Descrizione |
|-------|-------------|
| **Index** | Numero progressivo del messaggio (1-based) |
| **Data e Ora** | Timestamp formattato (YYYY/MM/DD HH:MM:SS.mmm) |
| **Timestamp (s)** | Secondi Unix dal 1970-01-01 |
| **Microsecondi** | Parte microsecondica del timestamp |
| **ECU ID** | Identificativo dell'ECU che ha generato il messaggio |
| **Dimensione** | Dimensione totale del messaggio in bytes |

## Formato DLT Supportato

### Storage Header v1 (16 bytes)
```
Offset | Size | Field
-------|------|-------------
0      | 4    | Pattern "DLT\x01"
4      | 4    | Seconds (uint32, big endian)
8      | 4    | Microseconds (int32, big endian)
12     | 4    | ECU ID (4 chars)
```

### Storage Header v2 (14+ bytes)
```
Offset | Size | Field
-------|------|-------------
0      | 4    | Pattern "DLT\x02"
4      | 8    | Seconds (uint64, big endian)
12     | 4    | Nanoseconds (uint32, big endian)
13     | 1    | ECU ID Length
14     | var  | ECU ID
```

## Test

Per testare l'applicazione, usa il file di esempio incluso nella repository:

```bash
# File di test disponibile
qdlt/tests/testfile.dlt
```

## Requisiti

- Browser moderno con supporto per:
  - FileReader API
  - DataView (manipolazione dati binari)
  - ES6+ JavaScript

## Limitazioni v1.0

Questa versione si concentra **esclusivamente sui timestamp** e non include:

- ❌ Parsing del payload dei messaggi
- ❌ Decodifica messaggi verbose/non-verbose
- ❌ Visualizzazione Application ID / Context ID
- ❌ Filtri avanzati per tipo di messaggio
- ❌ Grafici timeline
- ❌ Import da PCAP/MF4

Queste funzionalità potranno essere aggiunte in versioni future.

## Struttura Codice

L'applicazione è contenuta in un singolo file HTML standalone con:

```
dlt-timestamp-viewer.html
├── HTML Structure
│   ├── Header
│   ├── Upload Section
│   ├── Statistics Cards
│   ├── Results Table
│   └── Footer
├── CSS Styles (embedded)
│   ├── Responsive Design
│   ├── Gradient UI
│   └── Table Styling
└── JavaScript (embedded)
    ├── DltParser Class
    │   ├── parse()
    │   ├── parseMessage()
    │   ├── readString()
    │   └── timestampToDate()
    └── Application Logic
        ├── File Upload Handling
        ├── Drag & Drop
        ├── Search/Filter
        └── CSV Export
```

## Sicurezza

- ✅ Tutti i dati rimangono nel browser dell'utente
- ✅ Nessun file viene caricato su server esterni
- ✅ Nessuna richiesta di rete effettuata
- ✅ Privacy completa dei dati automotive

## Compatibilità Browser

| Browser | Versione Minima | Testato |
|---------|----------------|---------|
| Chrome  | 60+            | ✅      |
| Firefox | 55+            | ✅      |
| Safari  | 12+            | ✅      |
| Edge    | 79+            | ✅      |

## Sviluppi Futuri (v2.0)

Possibili miglioramenti per versioni future:

- 🔄 Parsing completo messaggi DLT
- 📈 Grafici timeline interattivi
- 🔧 Filtri avanzati (App ID, Context ID, Log Level)
- 💾 Supporto file grandi con streaming parser
- 🌐 Web Workers per parsing in background
- 📊 Statistiche avanzate (message rate, gap analysis)
- 🎨 Temi personalizzabili (dark mode)
- 🔌 Import da PCAP/MF4

## Licenza

Questo tool è stato sviluppato per supportare l'analisi di file DLT conformi allo standard COVESA.

## Autore

Sviluppato come prototipo per l'analisi rapida di timestamp in file DLT.

## Supporto

Per problemi o domande:
- Verifica che il file sia in formato `.dlt` valido
- Controlla la console JavaScript del browser per errori di parsing
- Assicurati di usare un browser moderno aggiornato

---

**Nota**: Questa è una versione 1.0 focalizzata esclusivamente sull'estrazione e visualizzazione dei timestamp. Per un'analisi completa dei file DLT, si consiglia di utilizzare DLT-Viewer desktop application.
