# SLN – Smart Light Network

## Lista dei componenti
-	30 diodi led bianchi;
-	30 resistenze da 100 Ω;
-	3 resistenze da 10 KΩ;
-	3 sensori PIR;
-	3 mosfet IRF540;
-	1 fotoresistenza;
-	1 NodeMCU;
-	3 BreadBoard;
-	3 Convertitori di livello logico doppio 3.3V;
-	Cavi elettrici per alimentazione e collegamenti;
-	Nastro Isolante;
-	Alimentatore USB;

## Introduzione
<p align="justify">
In questo report verrà presentato il progetto per la realizzazione di un sistema smart per l’illuminazione urbana. Il fine è quello di realizzare un sistema che permetta di evitare l’accensione costante delle luci al 100% della potenza anche quando non sono presenti persone e veicoli nella strada, in modo tale da avere un netto risparmio sui costi dell’illuminazione pubblica. <br>
Verranno proposti due algoritmi che, in base alla presenza di persone o veicoli nella strada, permettano la variazione dell’intensità luminosa dei vari apparati di illuminazione, col fine di avere dei consumi di potenza ottimizzati. In entrambi gli algoritmi le luci avranno un’intensità diversa a seconda dell’intensità luminosa presente nell’ambiente in cui il sistema verrà installato. Infatti, nei momenti della giornata in cui si avrà una luminosità ambientale maggiore (pomeriggio), l’intensità luminosa degli apparati di illuminazione sarà minore rispetto all’intensità luminosa che si avrebbe in altri momenti della giornata dove è presente invece una luminosità ambientale minore (sera e notte). <br>
L’accensione delle singole unità di illuminazione sarà supportata dalla rilevazione della presenza, in modo tale che al passaggio di una persona o di un veicolo le luci non si accendano tutte con la stessa intensità luminosa ma varino in funzione della rilevazione della presenza. In linea con ciò si sono sviluppati due modalità di funzionamento: <br>
- Bassa luminosità dell’ambiente: lampade in prossimità della quale si è rilevata una presenza con un’intensità pari al 100%, mentre le altre con un’intensità dell’80% <br>
- Alta luminosità dell’ambiente: lampade in prossimità della quale si è rilevata una presenza con un’intensità pari all’80%, mentre le altre con un’intensità del 50% <br>
Il sistema sarà poi in grado di trasmettere le informazioni di consumo istantaneo e consumo medio su una piattaforma di IoT (ThingSpeak), dando quindi la possibilità di monitorare le prestazioni del sistema e di confrontare tali prestazioni con quelle di consumo massimo, ossia il caso in cui le singole unità di illuminazione sono costantemente accese al 100% della potenza. Questo è estremamente importante per valutare l’effettivo risparmio energetico. Per questo motivo nell’interfaccia sarà possibile visualizzare l’energia percentuale risparmiata istante per istante, calcolata come differenza tra il consumo massimo e il consumo istantaneo effettivo.<br>
Infine un’ulteriore caratteristica sarà quella di poter utilizzare Telegram (piattaforma di messaggistica molto conosciuta), permettendo al gestore dell’illuminazione di controllare tramite lo scambio di messaggi, l’accensione e lo spegnimento delle luci e di recuperare le informazioni riguardanti i consumi di potenza istantanei dell’apparato di illuminazione.
</p>

## Realizzazione Smart Light System
<p align="justify">
Per realizzare un prototipo del sistema di illuminazione smart si è optato per dei led bianchi che ci consentono di avere una lampada con un’intensità abbastanza forte da vedere le differenze tra le varie intensità luminose.<br>
La presenza viene invece rilevata con dei sensori PIR, mentre la luminosità viene rilevata con una fotoresistenza direttamente collegata al controllore che in questo progetto è il NodeMCU. Tale controllore è particolarmente adatto al nostro scopo perché oltre ad avere svariati pin digitali e un pin analogico per la fotoresistenza, ha anche già integrata una scheda Wifi di tipo ESP8266 ESP12. Poichè tale controllore ha solo l’uscita a 3.3V, sia come VCC che nei pin digitali, per poterlo usare con la stragrande maggioranza dei dispositivi in commercio si rende indispensabile l’uso di un convertitore logico a doppio livello 3.3V-5V.<br>
Per la rilevazione della potenza istantanea assorbita dal sistema si è ricorsi all’uso di un multimetro, con il quale si è misurata la potenza che le singole lampade (create con i led) assorbono quando sono al 50%, all’80% e al 100% dell’intensità luminosa. Una volta conosciute le potenze per le singole intensità, è possibile risalire alla potenza massima assorbita dal sistema andando a vedere quali sono i sensori PIR che rilevano una presenza. Infatti le lampade dove si rileva una presenza sono quelle che si devono accendere al massimo mentre le altre invece staranno al minimo, dove minimo e massimo della potenza dipendono dalla luminosità ambientale.<br>
Per quanto riguarda l’invio dei dati alla piattaforma ThingSpeak si ha il vincolo, imposto dalla piattaforma stessa, di massimo un invio ogni 15 secondi che è esattamente il rate che abbiamo scelto di usare per l’invio dei dati.<br>
Infine, per quanto riguarda Telegram, si avrà la possibilità di spegnere o di accendere tutte le luci e di recuperare informazioni quali potenza istantanea, potenza media, potenza massima e energia percentuale risparmiata tutto espresso rispetto ai mW.<br>
Questa funzionalità è stata pensata per un uso privato del sistema in ambienti sia indoor che outdoor, più che per l’illuminazione pubblica.
</p>

## Assemblaggio Hardware
<p align="justify">
  <img align="center">
    
  </img>
  In progress...
</p>
