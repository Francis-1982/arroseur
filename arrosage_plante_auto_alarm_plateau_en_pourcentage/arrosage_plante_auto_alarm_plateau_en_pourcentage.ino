


// Section 1 - Description ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
  Programmation d'un projet réalisé par TRONIK AVENTUR afin de vérifier au 12h si la plante à besoin d'eau.
  Crée par TRONIK AVENTUR 

  Modifié par Francis 
  Modification exécuté: Ajout d'une sonde d'eau qui permet d'émmettre un message et un signial audio et visuel(led rouge) lorsque le réservoir d'eau est vide. 
                        Quand l'avertissement est fait, le système d'arrosage est désactivé soit au départ ou pendant les 15 cycles d'arrosage prévus.
                        Ajout d'une sonde d'humidité dans le plateau de la plante. 
                        La sonde désactive l'arrosage losque le plateau est rempli d'eau pour éviter le débordement. 
                        Ajout d'une note de musique pour l'activation du système.
                        Ajuster à 12 heures la vérification du sol avec un RTC.
                        Ajout d'un écran LCD.
                        Ajout d'un bouton pour activer manuellement le système.

  Date de création : 21 février 2024
  Dernière modification : 26 février 2025
*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Section 2 - Les variables ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Inclusion des bibliothèques nécessaires
#include <LCDI2C_Multilingual.h>                                              // Bibliothèque LCD
#include "pitches.h"                                                          // Réserve de notes déjà définies.
#include <Wire.h>                                                             // Bibliothèque RTC
#include <TimeLib.h>                                                          // Bibliothèque RTC
#include <DS1307RTC.h>                                                        // Bibliothèque RTC

// Déclaration des constantes des pin, avec leur usage
const int sondeEau = 5;                                                       // Assignation d'une constante nombre entier qui ne changera jamais et assigné à la pin 5 pour la sonde eau (SONDE EXTERNE: Model: XKC-Y25-NPN).
const int delRouge = 12;                                                      // Assignation de l'anode de la delRouge à la pin 12.
const int delVerte = 10;                                                      // Assignation de l'anode de la delVerte à la pin 10.
const int boutonOn = 3;                                                       // Création d'une constante nombre entier qui ne changera jamais et assigné à la Pin 3.
const int hautParleur = 2;                                                    // Pin du haut-parleur
//int onoffLCD = 9;                                                           // Assignation de l'alimentation de l'écran LCD à la pin 9.
const int vccsondeEau = 13;                                                   // Assignation de l'alimentation de la sonde eau à la pin 13.
const int relaisPomp = 8;                                                     // Assignation du relais à la pin 8.
const int sondeHum = 4;                                                       // Assignation de la sonde d'humidité à la pin 4.
const int sondeHum2 = 7;                                                      // Assignation de la sonde d'humidité #2 à la pin 7.
const int captAna = A1;                                                       // Assignation de la pin A1 pour le capteur analogique de la sonde d'humidité.
const int captAna2 = A0;                                                      // Assignation de la pin A0 pour le capteur analogique de la sonde d'humidité #2.

// Déclaration des constantes de paramétrage
const int sec = 1008;       // Valeur de la sonde lorsque la terre est sèche, correspond à 1%.
const int humide = 285;     // Valeur de la sonde lorsqu'elle est dans l'eau, correspond à 100%.
const int seuilTerre = 70;  // Pourcentage d'humidité de la terre en dessous duquel arroser
const int seuilPlato = 36;  // Pourcentage d'humidité du plateau en dessous duquel il est possible d'arroser

// Déclaration et initialisation des variables
int pourcentageHumiditeTerre;                                                 // Assignation de la valeur 0 au capteur analogique de la sonde d'humidité.
int pourcentageHumiditePlato;                                                 // Assignation de la valeur 0 au capteur analogique de la sonde d'humidité #2.
bool reservoirVide;                                                           // true si le réservoir est vide, false sinon
int melody1[] = { NOTE_G4, NOTE_B4, NOTE_D5, 0, NOTE_G4, NOTE_B4, NOTE_D5 };  // Assignation de la mélodie déterminée par les notes choisies ci dessous:
int melodydep[] = { NOTE_G4 };                                                // Assignation de la note choisie pour le départ.
LCDI2C_Latin lcd(0x27, 20, 4);                                                // Adresse I2C: 0x27; Dimension de l'affichage: 20x4.

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------- MESSAGE ET SIGNAL D'ALARME AUDITIF ET VISUEL AFIN DE REMPLIR LE RÉSERVOIR -----------------------------------------------------------------------------------
void alerteReservoirVide() {
  digitalWrite(delRouge, HIGH);   // Allume la DELrouge.

  lcd.home();                     // Ligne 0
  lcd.print("      REMPLIR");     // Affiche la phrase: REMPLIR.
  lcd.setCursor(0, 1);            // Ligne 1
  lcd.print("   RESERVE D'EAU");  // Affiche la phrase: RESERVE D'EAU.

  for (int thisNote = 0; thisNote < 8; thisNote++) {      // Procédé pour faire jouer les notes de la mélodie ???
    const int noteDurations = 200;                       // Le temps pour toutes les notes.
    tone(hautParleur, melody1[thisNote], noteDurations);  // Active la mélodie.
    delay(noteDurations * 1.30);                          // Pause entre les notes.
  }
  noTone(hautParleur);                                    // Arrêt de la note.

  delay(3000);                    // Durée de l'affichage (3 secondes).
  lcd.clear();                    // Effacer l'écran
}

void mesureHumidite() {
  pourcentageHumiditePlato = analogRead(captAna2);  // Mesure le résultat analogique de la sonde d'humidité #2.
  pourcentageHumiditePlato = map(pourcentageHumiditePlato, humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité #2.
  pourcentageHumiditePlato = constrain(pourcentageHumiditePlato, 0, 100); // La fonction constrain permet de s'assurer que la valeur reste entre 0 et 100.

  pourcentageHumiditeTerre = analogRead(captAna);  // Mesure le résultat analogique de la sonde d'humidité.
  pourcentageHumiditeTerre = map(pourcentageHumiditeTerre, humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité.
  pourcentageHumiditeTerre = constrain(pourcentageHumiditeTerre, 0, 100); // La fonction constrain permet de s'assurer que la valeur reste entre 0 et 100.
}

// Section 3 - Les instructions au démarrage --------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  pinMode(delRouge, OUTPUT);        // Assignation du mode de fonctionnement de la pin 12 = exécuter une action.
  pinMode(delVerte, OUTPUT);        // Assignation du mode de fonctionnement de la pin 10 = exécuter une action.
  pinMode(relaisPomp, OUTPUT);      // Assignation de la Pin 8 qui doit être un sorti d'information   (alimentation du relais qui active la pompe).
  pinMode(sondeHum, OUTPUT);        // Assignation de la Pin 4 qui doit être un sorti d'information   (alimentation de la sonde d'humidité).
  pinMode(sondeHum2, OUTPUT);       // Assignation de la Pin 7 qui doit être un sorti d'information   (alimentation de la sonde d'humidité #2).
  pinMode(vccsondeEau, OUTPUT);     // Assignation de la Pin 13 qui doit être un sorti d'information  (alimentation de la sonde eau).
  pinMode(sondeEau, INPUT_PULLUP);  // Assignation de la Pin 5 qui doit être une entrée d'information (lire un résultat de la sonde eau).
//  pinMode(onoffLCD, OUTPUT);        // Assignation de la Pin 9 qui doit être un sorti d'information (alimentation de l'écran LCD).
  pinMode(boutonOn, INPUT);         // Assignation du mode de fonctionnement de la pin 3 = lire un résultat.
  lcd.init();                       // Initialisation de l'écran LCD.
  lcd.backlight();                  // Activation des backlight.
  lcd.print("Initialisation");
//  Serial.begin(9600);               // Initialisation du moniteur série à la vitesse de 9600 baud.
  digitalWrite(relaisPomp, LOW);    // Directive qui détermine au départ l'état de la pompe - FERMÉ - (Désactivation du relais qui active la pompe).
  digitalWrite(sondeHum, LOW);      // Directive qui détermine au départ l'état de la sonde d'humidité - FERMÉ - (Désactivation de la sonde d'humidité) protection de l'oxydation de la sonde.
  digitalWrite(sondeHum2, LOW);     // Directive qui détermine au départ l'état de la sonde d'humidité #2 - FERMÉ - (Désactivation de la sonde d'humidité #2) protection de l'oxydation de la sonde.
  digitalWrite(delVerte, LOW);      // Fermer la delVerte.
  digitalWrite(vccsondeEau, LOW);   // Fermer la sonde eau pendant la temporisation(12h).
  digitalWrite(delRouge, LOW);      // Ferme la DELrouge.
  lcd.noBacklight();                // Désactivation des backlight.
  lcd.clear();
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Section 4 - Le coeur de la programmation -----------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  //------------------------------------------- UN TEMPS DE 12 HEURE EST PROGRAMMÉ AFIN DE VÉRIFIER SI LA PLANTE DOIT ÊTRE ARROSÉ ------------------------------------------------------------------------------

  tmElements_t tm;                                           // Initialisation du RTC
  if ((RTC.read(tm) && tm.Hour == 20 && tm.Minute == 10 && tm.Second == 10)  // L'heure programmé pour l'arrosage du soir.
      || digitalRead(boutonOn)) {                            // Si le bouton On est enfoncé, exécute la suite du programme.

    //digitalWrite(onoffLCD, HIGH);    // Allume l'écran LCD.
    lcd.backlight();                 // Allume l'écran LCD.
    lcd.setCursor(0, 1);             // Ligne 1
    lcd.print("     Minuterie");     // Affiche la phrase: Minuterie
    lcd.setCursor(0, 2);             // Ligne 2
    lcd.print("    Activation ! ");  // Affiche la phrase: Activation !

    for (int thisNote = 0; thisNote < 1; thisNote++) {        // Procédé pour faire jouer la note de départ.
      const int noteDurations = 200;                          // Le temps que dure la note de départ.
      tone(hautParleur, melodydep[thisNote], noteDurations);  // Active la note.
      delay(noteDurations * 1.30);                            // Pause entre les notes.
    }
    noTone(hautParleur);                                      // Arrêt de la note.

    delay(3000);                     // Durée de l'affichage (3 secondes).
    lcd.clear();                     // Effacer l'écran.

    //------------------------------------------------------------ VÉRIFICATION DU NIVEAU D'EAU DANS LE RÉSERVOIR ------------------------------------------------------------------------------------------------

    digitalWrite(delVerte, HIGH);     // Allume la delVerte.
    digitalWrite(vccsondeEau, HIGH);  // Activer la sonde eau.
    delay(2000);                      // Pause de 2 secondes.

    reservoirVide = digitalRead(sondeEau);
    pourcentageHumiditeTerre = 200;
    pourcentageHumiditePlato = 200;

    if (reservoirVide) {      // Si tu détecte la sonde eau (la Sonde émet un signal lorsqu'il n'y a pas d'eau):
      alerteReservoirVide();
    } else {                   // Sinon si la sonde eau n'émet pas de signal (indique la présence d'eau).

    // ----------------------------------------------- UNE PREMIÈRE LECTURE D'HUMIDITÉ EST EFFECTUÉ (2 sondes) ET LE RAPPORT APPARAIT SUR LE MONITEUR SÉRIE ----------------------------------------------------------

      digitalWrite(delRouge, LOW);           // Ferme la DELrouge.
      digitalWrite(sondeHum, HIGH);          // Ouvre la sonde de l'humidité.
      digitalWrite(sondeHum2, HIGH);         // Ouvre la sonde de l'humidité #2.
      delay(1000);                           // Un delais d'une seconde ensuite.

      mesureHumidite();

      lcd.home();
      lcd.print("HUMID. PLATEAU= ");         // Affiche la phrase: HUMID. PLATEAU =
      lcd.print(pourcentageHumiditePlato);                                     // Afficher ensuite la valeur en % de la sonde d'humidité #2.
      lcd.println("%");
      lcd.setCursor(0, 2);                 // Ligne 2
      lcd.print("HUMID. TERRE= ");         // Affiche la phrase: HUMID. TERRE =
      lcd.print(pourcentageHumiditeTerre);                                    // Affiche ensuite la valeur en % de la sonde d'humidité.
      lcd.println("%");

      delay(3000);  // Durée de l'affichage (3 secondes).
      lcd.clear();  // Effacer l'écran.
    }
    //------------------------------------------------- COMMANDE POUR ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ INDIQUE UN SOL TROP SEC -----------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    int arrosage = 0;
    while (!reservoirVide && (pourcentageHumiditePlato < seuilPlato) && (pourcentageHumiditeTerre < seuilTerre)) {
      arrosage++;                     // incrémente arrosage.
      lcd.setCursor(0, 1);            // Ligne 1
      lcd.print(pourcentageHumiditeTerre); // Affiche la valeur en % de la sonde de l'humidité.
      lcd.print("% ; ");
      lcd.print(arrosage);            // Affiche le nombre d'arrosage effectué depuis le début sur l'écran LCD.
      lcd.print(" arrosage ");        // Affiche le mot: ; arrosage
      digitalWrite(relaisPomp, HIGH); // Active la pompe.
      delay(15000);                   // Durée de l'activation pompe (15 secondes).
      digitalWrite(relaisPomp, LOW);  // Ferme la pompe.
      delay(20000);                   // Pause de la pompe pour laisser à l'eau le temps de pénétrer la terre (20 secondes).

      //------------------------------------- MISE À JOUR DES MESURES DEPUIS LES CAPTEURS -------------------------------------//
      mesureHumidite();
      reservoirVide = digitalRead(sondeEau);
    }

    if (reservoirVide) { // Si le réservoir est vide
      alerteReservoirVide();
    } else if (pourcentageHumiditePlato >= seuilPlato) { // Sinon, si le plateau est rempli d'eau

      //--------------------------------------- COMMANDE POUR CESSER D'ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ #2 INDIQUE QUE LE PLATEAU EST REMPLI D'EAU -----------------------------------------------------

      lcd.home();                         // Ligne 0
      lcd.print("PLATEAU PLEIN D'EAU");   // Affiche la phrase: PLATEAU PLEIN D'EAU
      lcd.setCursor(0, 1);                // Ligne 1
      lcd.print(" 3mm:30% / 1.5cm:80%");  // Affiche la phrase: 3mm:30% / 1.5cm:80%
      lcd.setCursor(0, 3);                // Ligne 2
      lcd.print("ARRET DE L'ARROSAGE");   // Affiche la phrase: ARRET DE L'ARROSAGE
      delay(5000);                        // Durée de l'affichage (5 secondes).

    } else {  // Sinon (si l'humidité de la terre est plus grande ou égale à seuilTerre).

      //--------------------------------------- COMMANDE POUR CESSER D'ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ INDIQUE UN SOL HUMIDE -------------------------------------------------------------------------

      lcd.setCursor(0, 1);                // Ligne 1
      lcd.print(" HUMID. TERRE = ");      // Affiche la phrase: / HUMID. TERRE =
      lcd.print(pourcentageHumiditeTerre);// Affiche ensuite la valeur en % de la sonde d'humidité.
      lcd.println("%");

      delay(3000);                        // Durée de l'affichage (3 secondes).
      lcd.clear();                        // Effacer l'écran.

      lcd.print("TERRE ASSEZ HUMIDE");    // Affiche la phrase: TERRE ASSEZ HUMIDE.
      lcd.setCursor(0, 2);                // Ligne 2
      lcd.print("Au dessus de ");         // Imprimer la phrase: Haut dessus de 50%.
      lcd.print(seuilTerre);
      lcd.print("%");

      delay(3000);                        // Durée de l'affichage (3 secondes).
      lcd.clear();                        // Effacer l'écran.

      lcd.setCursor(0, 1);                // Ligne 1
      lcd.print("*** FIN DU CYCLE ***");  // Imprime la phrase: *** FIN DU CYCLE ***
      lcd.setCursor(0, 2);                // Ligne 2
      lcd.print("**** D'ARROSAGE ****");  // Imprime la phrase: **** D'ARROSAGE ****

      delay(3000);                        // Durée de l'affichage (3 secondes).
    }
  }

  // Ferme tout (sauf la delRouge)
  lcd.clear();                        // Effacer l'écran.
  // digitalWrite(onoffLCD, LOW);     // Fermer l'écran LCD.
  lcd.noBacklight();
  digitalWrite(delVerte, LOW);     // Fermer la delVerte.
  digitalWrite(sondeHum, LOW);     // Fermer la sonde d'humidité pendant la temporisation(12h) pour protection de l'oxydation de la sonde.
  digitalWrite(sondeHum2, LOW);    // Fermer la sonde d'humidité #2 pendant la temporisation(12h) pour protection de l'oxydation de la sonde.
  digitalWrite(vccsondeEau, LOW);  // Fermer la sonde eau pendant la temporisation(12h).
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
