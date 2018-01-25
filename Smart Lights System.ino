#include <ESP8266WiFi.h>
#include <ESP8266TelegramBOT.h>
#include <WiFiClientSecure.h>

#define pir1 2 // passive infraRed 1 collegato al pin D5
#define pir2 4 // passive infraRed 2 collegato al pin D6
#define pir3 7 // passive infraRed 3 collegato al pin D7

#define lamp1 3 // transistor 1 collegato al pin D1
#define lamp2 5 // transistor 2 collegato al pin D2
#define lamp3 6 // transistor 3 collegato al pin D3

#define photoR A0 // fotoresistenza collegata al pin A0
#define currSensor A1 // sensore di corrente continua

int presence[] = {0, 0, 0}; // valori letture sensori pir1, pir2 e pir3
int transistorList[] = {3, 5, 6}; // lista dei transistor 1, 2 e 3
float Pavg[] = {0, 0, 0, 0}; // serve per salvare i valori di potenza dell'ultimo minuto
int PavgLenght = 4; // serve per inviare la Potenza media ogni 15*4=60sec
float Pmax = 0.0; //                                                                                    <--------- Da inserire
float lastPavg = 0.0; // serve per salvare l'ultima media delle potenze
float lastEsav = 0.0; // serve per salvare l'ultima percentuale di energia risparmiata
float lastPist = 0.0; // serve per salvare l'ultima potenza istantanea
// --------------- WiFi Variables -------------------
const char* MY_SSID = " "; // nome della rete WiFi                                                      <--------- Da inserire
const char* MY_PWD = " "; // password della rete WiFi                                                   <--------- Da inserire
// --------------------------------------------------
// --------- Thingspeak Variables -------------------
const char* server = "api.thingspeak.com";
String apiKey = " "; // chiave in scrittura ThingSpeak                                                  <--------- Da inserire
long TS_lastTime; // da usare con millis per sapere il tempo dell'ultimo dato inviato a ThingSpeak
int TS_delay = 15000; // delay prima di poter inviare un altro dato a ThingSpeak
int sendPavgcount = 0; // contatore per l'invio della potenza media
// --------------------------------------------------
// --------- Telegram Variables ---------------------
#define BOTtoken " " // chiave del bot Telegram                                                        <--------- Da inserire
#define BOTname " " // nome del bot Telegram                                                           <--------- Da inserire
#define BOTusername " " // username del bot Telegram                                                   <--------- Da inserire
TelegramBOT bot(BOTtoken, BOTname, BOTusername); // dichiarazione e inizializzazione dell'oggetto bot Telegram
long Bot_lastTime; // da usare con millis per sapere il tempo dell'ultimo messaggio inviato al bot Telegram
int Bot_delay = 1000; // delay prima di poter inviare un altro massaggio al bot Telegram
bool startControlWithTelegram = 0; // iniziare a controllare il sistema con il bot Telegram
bool controlWithTelegram = 0; // vale 0 per il funzionamento normale, vale uno per il funzionamento controllato tramite bot Telegram
// --------------------------------------------------

void setup() {
  pinMode(lamp1, OUTPUT);
  pinMode(lamp2, OUTPUT);
  pinMode(lamp3, OUTPUT);
  pinMode(pir1, INPUT);
  pinMode(pir2, INPUT);
  pinMode(pir3, INPUT);
  pinMode(photoR, INPUT);
  pintMode(currSensor, INPUT);

  connectWifi(); // connessione al WiFi
  bot.begin(); // inizializzazione del bot Telegram
}

void loop() {
  if (controlWithTelegram == 0) { // funzionamento con rilevazione di luminosità e presenza
    lightController();
  }
  if (millis() > Bot_lastTime + Bot_delay) { // in questa sezione si risponde ai messaggi in ingresso, prima di iniziare bisogna inserire la parola START!
    bot.getUpdates(bot.message[0][1]);
    if (startControlWithTelegram == 0 && bot.message[0][0].toInt() == 1) {
      initializeBotTelegramControl();
      Bot_lastTime = millis();
    }
    if (startControlWithTelegram == 1 && bot.message[0][0].toInt() == 1) {
      getMessageAndAnswer();
      Bot_lastTime = millis();
    }
  }
  if (millis() > TS_lastTime + TS_delay) { // invio dati alla piattaforma ThingSpeak
    sendValuesToThingSpeak();
    TS_lastTime = millis();
  }
}

//---------------------------------- Funzioni -----------------------------------------

// permette di controllare il funzionamento delle luci tramite fotoresistenza nelle seguenti modalità:
// - valore fotoresistenza High: non viene accesa nessuna luce
// - valore fotoresistenza Medium: viene accesa la luce in cui c'è una presenza all'80%, mentre le altre al 50%
// - valore fotoresistenza Low: viene accesa la luce in cui c'è una presenza al 100%, mentre le altre all'80%
void lightController() {
	String brightnessValue = detectBrightness();

	if (brightnessValue == "High") {
		for (int i = 0; i < 3; i++) {
			analogWrite(transistorList[i], 0);
		}
		return;
	}
	else if (brightnessValue == "Medium") {
		detectPresence(); // aggiorna il vettore delle presenze
		if (oneIsInArray() == 1) {
			for (int i = 0; i < 3; i++) {
				if (presence[i] == 1) {
					analogWrite(transistorList[i], 205); //80%
				}
				else {
					analogWrite(transistorList[i], 128); //50%
				}
			}
			return;
		}
		else {
			for (int i = 0; i < 3; i++) {
				analogWrite(transistorList[i], 0);
			}
			return;
		}
	}
	else if (brightnessValue == "Low") {
		detectPresence(); // aggiorna il vettore delle presenze
		if (oneIsInArray() == 1) { // accendo le luci in cui c'è una presenza al 100%, mentre le altre all'80%
			for (int i = 0; i < 3; i++) {
				if (presence[i] == 1) {
					analogWrite(transistorList[i], 255); //100%
				}
				else {
					analogWrite(transistorList[i], 204); //80%
				}
			}
			return;
		}
		else { // spengo tutte le luci
			for (int i = 0; i < 3; i++) {
				analogWrite(transistorList[i], 0);
			}
			return;
		}
	}
}

// permette di controllare il funzionamento delle luci tramite comando via Telegram
// I comandi disponibili sono:
// ON, OFF per accendere o spegnere le luci
// PMAX, PIST, PAVG, ESAV per conoscere quali sono le varie potenze e l'energia percentuale risparmiata
String lightControllerWithInput(String comand) {
	if (comand == "ON") {
		controlWithTelegram = 0;
		return "1";
	}
	else if (comand == "OFF") {
		for (int i = 0; i < 3; i++) {
			analogWrite(transistorList[i], 0);
		}
		controlWithTelegram = 1;
		return "1";
	}
	else if (comand == "PMAX") {
		return "La potenza massima vale: " + (String)Pmax + "mW";
	}
	else if (comand == "PIST") {
		return "La potenza istantanea vale: " + (String)lastPist + "mW";
	}
	else if (comand == "PAVG") {
		return "La potenza media vale: " + (String)lastPavg + "mW";
	}
	else if (comand == "ESAV") {
		return "L'energia risparmiata vale: " + (String)lastEsav + "%";
	}
	else if (comand == "EXIT") {
		controlWithTelegram = 0;
		startControlWithTelegram = 0;
		return "1";
	}
	else {
		return "0";
	}
}

// riempie l'array delle presenze con 0 (nessuna presenza) o 1 (presenza rilevata) per ogni pir del sistema(3 pir)
void detectPresence() {
	presence[0] = digitalRead(pir1);
	presence[1] = digitalRead(pir2);
	presence[2] = digitalRead(pir3);
}

// rende 1 se nell'array passato è presente almeno un 1, altrimenti rende 0
bool oneIsInArray() {
	for (int i = 0; i < 3; i++) {
		if (presence[i] == 1) {
			return 1;
		}
	}
	return 0;
}

// rende una stringa con le soglie di luminosità: High, Medium, Low
// bisogna testare la luminosità dell'ambiente per decidere le due soglie!!
String detectBrightness() {
	int brightness = analogRead(photoR);
	if (brightness < "Soglia1") {                                                               // <--------- Da inserire
		return "Low";
	}
	else if (brightness > "Soglia1" && brightness < "Soglia2") {                                // <--------- Da inserire
		return "Medium";
	}
	else if (brightness > "Soglia2") {                                                          // <--------- Da inserire
		return "High";
	}
}

// misura la corrente nel sistema con un ACS712 da 5A
float detectPist(){
    return (.0264 * analogRead(currSensor) -13.51) / 1000;
  }

}

// connessione al WiFi
void connectWifi() {
	WiFi.begin(MY_SSID, MY_PWD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
	}
}

// invia i dati sulla potenza alla piattaforma ThingSpeak
void sendValuesToThingSpeak() {
	WiFiClient client;
	if (client.connect(server, 80)) {
		updateLastPist();
		computeEsaved();
		
		String postStr = apiKey;
		postStr += "&field1=";
		postStr += String(PistList[3] * 3);  // potenza massima
		postStr += "&field2=";
		postStr += String(lastPist);
		postStr += "&field3=";
		postStr += String(lastEsav);
        
		if (sendPavgcount == PavgLenght) { //ogni 30 sec invio
			Paverage();
			postStr += "&field4=";
			postStr += String(lastPavg);
			sendPavgcount = 0;
		}

		postStr += "\r\n\r\n";

		client.print("POST /update HTTP/1.1\n");
		client.print("Host: api.thingspeak.com\n");
		client.print("Connection: close\n");
		client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
		client.print("Content-Type: application/x-www-form-urlencoded\n");
		client.print("Content-Length: ");
		client.print(postStr.length());
		client.print("\n\n");
		client.print(postStr);
	}
	client.stop();
}

void Paverage() {
	float sum = 0;
	for (int i = 0; i < PavgLenght; i++) {
		sum = sum + Pavg[i];
	}
	lastPavg = sum / PavgLenght;
}

// aggiorna il valore di potenza istantanea
void updateLastPist() {
	lastPist = detectPist();
	Pavg[sendPavgcount] = lastPist; // iserisce il valore di potenza nel vettore che verrà usato per fare la media
	sendPavgcount++; // incrementa il contatore che verrà usato per inviare i dati sulla potenza media
}

// calcola la media del vettore delle potenze
void computeEsaved() {
	lastEsav = ((lastPist - Pmax) / Pmax) * 100; 
}

// permette di controllare il sistema e ricevere informazioni tramite bot se viene ricevuto il messaggio START a cui viene risposto con la lista dei comandi disponibili
void initializeBotTelegramControl() {
	for (int i = 1; i < 2; i++) {
		while (true) {
			String mess;
			mess = bot.message[i][5]; // inserisce il testo del messaggio ricevuto nella variabile mess
			mess.toUpperCase(); // rende il testo tutto maiuscolo perché non è case sensitive
			if (0 == mess.compareTo("START")) {
				bot.sendMessage(bot.message[i][4], "Inizializzo sistema", "");
				bot.sendMessage(bot.message[i][4], "Comandi disponibili:", "");
				bot.sendMessage(bot.message[i][4], "- ON, OFF permettono di accendere e spegnere il sistema", "");        
				bot.sendMessage(bot.message[i][4], "- PMAX rende il valore di potenza massima del sistema", "");
				bot.sendMessage(bot.message[i][4], "- PIST rende il valore di potenza intantanea del sistema", "");
				bot.sendMessage(bot.message[i][4], "- PAVG rende il valore di potenza media del sistema", "");
				bot.sendMessage(bot.message[i][4], "- ESAV rende la percentuale di energia risparmiata dal sistema", "");
				bot.sendMessage(bot.message[i][4], "- EXIT per uscire dal bot telegram", "");
				startControlWithTelegram = 1;
				break;
			}
			else {
				bot.sendMessage(bot.message[i][4], "Comando iniziale sbagliato!", "");
				break;
			}
		}
    bot.message[0][0] = "";
  }
}

// permette di eseguire dei comandi dopo l'inizializzazione del controllo del sistema con Telegram. Risponde con l'esito del comando
void getMessageAndAnswer() {
	for (int i = 1; i < 2; i++) {
		String mess;
		mess = bot.message[i][5];
		mess.toUpperCase();
		String answer = lightControllerWithInput(mess); // richiama la funzione per il controllo del sistema tramite input
		if (answer == "1") {
			bot.sendMessage(bot.message[i][4], "Comando Eseguito", "");
		}
		else if (answer == "0") {
			bot.sendMessage(bot.message[i][4], "Comando non esistente, riprovare!", "");
		}
		else {
			bot.sendMessage(bot.message[i][4], answer, "");
		}
		bot.message[0][0] = "";
	}
}
//-------------------------------------------------------------------------------------------------------
