# Istruzioni repo

Le intenzioni di questo repository sono quelle di creare una libreria compatta
e specializzata per heap-priority-queue.

Il progetto non deve diventare una libreria generica di priority-queue basata
su famiglie eterogenee di strutture dati. Per mantenere il codice più mirato e
confrontabile, le implementazioni supportate e previste devono rimanere
heap-based:

- heap binari;
- heap di Fibonacci;
- heap di Fibonacci revisited (qui chiamati heap di Kaplan).

Implementazioni basate su strutture dati non heap-based non fanno parte dello
scopo del repository.




## Tassonomia

La seguente tassonomia serve per chiarire cosa deve essere implementato. Le
strutture concrete sono tutte varianti heap-based della stessa API pubblica:

- ADT pubblico:
    + priority_queue
- famiglia implementativa:
    + heaps/
- implementazioni concrete:
    + binary_heap
    + fibonacci_heap
    + kaplan_heap




## Note architetturali

- esiste una sola API astratta pubblica: priority_queue;
- la famiglia heaps/ è la categoria organizzativa principale del progetto;
- ogni implementazione concreta heap-based fornisce direttamente la vtable della priority_queue;
- non sono previste vtable intermedie per heap, salvo necessità future.




## Implementazione codice

Si richiede che il codice abbia le seguenti caratteristiche:

- deve essere implementato interamente in C99 (per avere massima portabilità);
- deve essere strutturato in modo modulare, separando interfaccia e implementazione delle strutture dati;
- ogni heap-priority-queue sarà esposta tramite un’API astratta comune, mentre le diverse varianti concrete heap-based forniranno implementazioni distinte della stessa interfaccia logica;
- l’organizzazione del codice ricalca il modello tipico dei linguaggi object-oriented come Java o Scala, ma adattato idiomaticamente al C.




## Operazioni

Un primo set di operazioni delle priority-queue è il seguente:

- create;
- destroy;
- push;
- peek;
- pop;
- size;
- empty.

Operazioni più complesse aggiunte dopo il primo nucleo dell'API:

- decrease_key;
- remove;
- contains.

`decrease_key` e `remove` lavorano tramite handle restituiti all'inserimento,
così la posizione del nodo è nota come richiesto dagli heap di Fibonacci.




# Sviluppo repo

Le seguenti note descrivono la direzione sperimentale del repository. La C API
ha già acquisito handle espliciti per le operazioni mirate; le predizioni
learning-augmented restano invece, per ora, prototipi Python costruiti sopra la
priority queue nativa.




## Fase 1: esperimenti Python sulle predizioni

Prima validiamo che le predizioni abbiano senso. In `tests/pq_experiments/`
manteniamo funzioni riusabili per Dijkstra con trace:

```
dijkstra_with_trace(graph, source, implementation="binary_heap")
```

che ritorna:

```
{
    "distances": ...,
    "extraction_order": ...,
    "inserted_keys": ...,
    "push_trace": ...,
    "stats": {
        "pushes": ...,
        "pops": ...,
        "stale_pops": ...,
        "extractions": ...,
        "reached_nodes": ...
    }
}
```

I predittori principali sono:
- `build_node_rank_predictor(previous_extraction_order)`
- `build_key_rank_predictor(previous_inserted_keys)`

Per usare dataset reali di shortest path in formato DIMACS, manteniamo anche
un parser riusabile:

- `load_dimacs_graph(path)`

Il parser deve leggere file `.gr` con righe `c`, `p sp <nodes> <arcs>` e
`a <tail> <head> <weight>`, come i grafi stradali del 9th DIMACS Challenge.
La rappresentazione in memoria dei grafi DIMACS deve essere CSR, usando `array`
della standard library per ridurre l'overhead sui grafi grandi.
I test automatici devono restare leggeri: lo script learning-augmented può
limitare il numero di archi caricati, mentre i dataset DIMACS reali non devono
essere tracciati in git.

Testiamo:
1. Dijkstra produce distanze corrette;
2. una run precedente genera predizioni;
3. le predizioni hanno errori misurabili;
4. le predizioni non rompono la correttezza perché la C queue resta il backend reale.

Per questa fase, raccogliamo nei test e nei risultati generati dati che:

- usa davvero un grafo DIMACS scaricato localmente;
- esegue Dijkstra da più sorgenti;
- usa ogni run per predire la successiva;
- confronta predizioni node-rank e key-rank;
- stampa statistiche aggregate su errori di predizione, push, pop, stale entries
  e nodi raggiunti.

Questo ci dà dati concreti senza introdurre una directory separata per script
sperimentali finché il progetto resta piccolo.




## Fase 2: possibile hint opzionale nella C API

Solo se i dati Python mostrano un beneficio chiaro, potremo aggiungere:

```
int priority_queue_push_with_hint(
    struct priority_queue *queue,
    void *item,
    const void *hint
);
```

All’inizio ogni backend può implementarla come `return priority_queue_push(queue, item);`
Poi aggiorniamo il binding Python con qualcosa tipo:
    - `queue.push(item, hint=...)`
    - o una funzione separata: `queue.push_with_hint(item, hint)`

Io preferirei `push_with_hint` separata, perché rende esplicito che stai usando una feature sperimentale.
