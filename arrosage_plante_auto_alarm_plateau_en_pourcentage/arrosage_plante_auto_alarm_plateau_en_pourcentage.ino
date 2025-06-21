
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
  
  #include <LCDI2C_Multilingual.h>  // Bibliothèque LCD
  #include "pitches.h"     // Réserve de notes déjà définies. 
  #include <Wire.h>        // Bibliothèque RTC
  #include <TimeLib.h>     // Bibliothèque RTC
  #include <DS1307RTC.h>   // Bibliothèque RTC
  const int sec = 1008;    // Valeur de la sonde lorsque la terre est sèche, correspond à 1%.
  const int humide = 285;  // Valeur de la sonde lorsqu'elle est dans l'eau, correspond à 100%.
  const int sondeEau = 5;  // Assignation d'une constante nombre entier qui ne changera jamais et assigné à la pin 5 pour la sonde eau (SONDE EXTERNE: Model: XKC-Y25-NPN).
  const int delRouge = 12; // Assignation de l'anode de la delRouge à la pin 12.
  const int delVerte = 10; // Assignation de l'anode de la delVerte à la pin 10.
  const int boutonOn = 3;  // Création d'une constante nombre entier qui ne changera jamais et assigné à la Pin 3. 
  int boutonOnEtat = 0;    // Création d'une variable nombre entier de l'état du bouton On.
  int onoffLCD = 9;        // Assignation de l'alimentation de l'écran LCD à la pin 9. 
  int vccsondeEau = 13;    // Assignation de l'alimentation de la sonde eau à la pin 13.
  int delEtat = 0;         // Assignation de la valeur 0 à la DELrouge.
  int relaisPomp = 8;      // Assignation du relais à la pin 8.
  int sondeHum = 4;        // Assignation de la sonde d'humidité à la pin 4.
  int sondeHum2 = 7;       // Assignation de la sonde d'humidité #2 à la pin 7. 
  int captAna = A1;        // Assignation de la pin A1 pour le capteur analogique de la sonde d'humidité.
  int captAna2 = A0;       // Assignation de la pin A0 pour le capteur analogique de la sonde d'humidité #2. 
  int detValeurana = 0;    // Assignation de la valeur 0 au capteur analogique de la sonde d'humidité.
  int detValeurana2 = 0;   // Assignation de la valeur 0 au capteur analogique de la sonde d'humidité #2.
  int melody1[] = { NOTE_G4, NOTE_B4, NOTE_D5, 0, NOTE_G4, NOTE_B4, NOTE_D5 };  // Assignation de la mélodie déterminée par les notes choisies ci dessous:
  int noteDurations1[] = {};        // Le temps pour chaque note. 
  int melodydep[] = { NOTE_G4 };    // Assignation de la note choisie pour le départ.
  int noteDurations2[] = {};        // Le temps pour la note choisie pour le départ.
  LCDI2C_Latin lcd(0x27, 20, 4);    // Adresse I2C: 0x27; Dimension de l'affichage: 20x4.
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// Section 3 - Les instructions au démarrage --------------------------------------------------------------------------------------------------------------------------------------------------------------
  

  void setup() {
  pinMode(delRouge, OUTPUT);        // Assignation du mode de fonctionnement de la pin 12 = exécuter une action.
  pinMode(delVerte, OUTPUT);        // Assignation du mode de fonctionnement de la pin 10 = exécuter une action.
  pinMode(relaisPomp, OUTPUT);      // Assignation de la Pin 8 qui doit être un sorti d'information   (alimentation du relais qui active la pompe).
  pinMode(sondeHum, OUTPUT);        // Assignation de la Pin 4 qui doit être un sorti d'information   (alimentation de la sonde d'humidité).
  pinMode(sondeHum2,OUTPUT);        // Assignation de la Pin 7 qui doit être un sorti d'information   (alimentation de la sonde d'humidité #2).
  pinMode(vccsondeEau, OUTPUT);     // Assignation de la Pin 13 qui doit être un sorti d'information  (alimentation de la sonde eau).
  pinMode(sondeEau, INPUT_PULLUP);  // Assignation de la Pin 5 qui doit être une entrée d'information (lire un résultat de la sonde eau).
  pinMode(onoffLCD, OUTPUT);        // Assignation de la Pin 9 qui doit être un sorti d'information (alimentation de l'écran LCD).
  pinMode(boutonOn, INPUT);         // Assignation du mode de fonctionnement de la pin 3 = lire un résultat.
  Serial.begin(9600);               // Initialisation du moniteur série à la vitesse de 9600 baud.
  digitalWrite(relaisPomp, LOW);    // Directive qui détermine au départ l'état de la pompe - FERMÉ - (Désactivation du relais qui active la pompe).
  digitalWrite(sondeHum, LOW);      // Directive qui détermine au départ l'état de la sonde d'humidité - FERMÉ - (Désactivation de la sonde d'humidité) protection de l'oxydation de la sonde. 
  digitalWrite(sondeHum2, LOW);     // Directive qui détermine au départ l'état de la sonde d'humidité #2 - FERMÉ - (Désactivation de la sonde d'humidité #2) protection de l'oxydation de la sonde.
  lcd.init();                       // Initialisation de l'écran LCD.
  lcd.backlight();                  // Activation des backlight.
  }
   

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Section 4 - Le coeur de la programmation -----------------------------------------------------------------------------------------------------------------------------------------------------------------


  void loop() {
  
  tempo:                              // Le terme "tempo" fait revenir la programmation à cet endroit par la fonction "goto" placé à la fin de l'avertissement (remplir le réservoir). 
  digitalWrite(onoffLCD, LOW);        // Fermer l'écran LCD.
  digitalWrite(delVerte, LOW);        // Fermer la delVerte.
  digitalWrite(sondeHum, LOW);        // Fermer la sonde d'humidité pendant la temporisation(12h) pour protection de l'oxydation de la sonde.
  digitalWrite(sondeHum2, LOW);       // Fermer la sonde d'humidité #2 pendant la temporisation(12h) pour protection de l'oxydation de la sonde.
  digitalWrite(vccsondeEau, LOW);     // Fermer la sonde eau pendant la temporisation(12h). 

//------------------------------------------- UN TEMPS DE 12 HEURE EST PROGRAMMÉ AFIN DE VÉRIFIER SI LA PLANTE DOIT ÊTRE ARROSÉ ------------------------------------------------------------------------------

  boutonOnEtat = digitalRead(boutonOn);                 // Lire position actuel du bouton On.
 

  tmElements_t tm;                                      // Initialisation du RTC 
  if (RTC.read(tm)) {                                   // Activation du RTC.
  if (tm.Hour==20 && tm.Minute==10 && tm.Second==10||   // L'heure programmé pour l'arrosage du soir.
     (boutonOnEtat==1)){                                // Si le bouton On est enfoncé, exécute la suite du programme.
    
      digitalWrite(onoffLCD, HIGH);                      // Allume l'écran LCD.
      lcd.setCursor(0,1);                                // Ligne 1
      lcd.print("     Minuterie");                       // Affiche la phrase: Minuterie 
      lcd.setCursor(0,2);                                // Ligne 2
      lcd.print("    Activation ! ");                    // Affiche la phrase: Activation ! 
      delay(3000);                                       // Durée de l'affichage (3 secondes).
      lcd.clear();                                       // Effacer l'écran.
    
    for (int thisNote2 = 0; thisNote2 < 1; thisNote2++) {  // Procédé pour faire jouer la note de départ.
    int noteDurations2 = 200;                              // Le temps que dure la note de départ.
    tone(2, melodydep[thisNote2], noteDurations2);         // Active la note.
    int pauseBetweenNotes = noteDurations2 * 1.30;         // Pause entre les notes.
    delay(pauseBetweenNotes);                              // Pause entre les notes.
    noTone(2);                                             // Arrêt de la note sur la pin 2.
  }

//------------------------------------------------------------ VÉRIFICATION DU NIVEAU D'EAU DANS LE RÉSERVOIR ------------------------------------------------------------------------------------------------


  delEtat = digitalRead(delRouge);                         // Lire état actuel de la delRouge.
  delEtat = digitalRead(delVerte);                         // Lire état actuel de la delVerte.
  
  digitalWrite(delVerte, HIGH);              // Allume la delVerte.
  digitalWrite(vccsondeEau, HIGH);           // Activer la sonde eau.
  delay(2000);                               // Pause de 2 secondes.
  if (digitalRead(sondeEau)){                // Si tu détecte la sonde eau (la Sonde émet un signal lorsqu'il n'y a pas d'eau):


 //---------------------------------------------- MESSAGE ET SIGNAL D'ALARME AUDITIF ET VISUEL AFIN DE REMPLIR LE RÉSERVOIR -----------------------------------------------------------------------------------
  
  lcd.setCursor(0,0);                                       // Ligne 0
  lcd.print("      REMPLIR");                               // Affiche la phrase: REMPLIR.
  lcd.setCursor(0,1);                                       // Ligne 1
  lcd.print("   RESERVE D'EAU");                            // Affiche la phrase: RESERVE D'EAU.
  delay(3000);                                              // Durée de l'affichage (3 secondes).
  lcd.clear();                                              // Effacer l'écran
  digitalWrite(delRouge, HIGH);                             // Allume la DELrouge.
  
                                                            
  for (int thisNote1 = 0; thisNote1 < 8; thisNote1++) {     // Procédé pour faire jouer les notes de la mélodie ???
    int noteDurations1 = 200;                               // Le temps pour toutes les notes.
    tone(2, melody1[thisNote1], noteDurations1);            // Active la mélodie.
    int pauseBetweenNotes = noteDurations1 * 1.30;          // Pause entre les notes.
    delay(pauseBetweenNotes);                               // Pause entre les notes.
    noTone(2);                                              // Arrêt de la note sur la pin 2.
                              
  }
goto tempo;                                                // La fonction "goto" placé à la fin de l'avertissement (REMPLIR RÉSERVE D'EAU), 
                                                           // permet à la programmation de revenir au terme "tempo" pour s'arrêter pendant 12h.
}
  
// ----------------------------------------------- UNE PREMIÈRE LECTURE D'HUMIDITÉ EST EFFECTUÉ (2 sondes) ET LE RAPPORT APPARAIT SUR LE MONITEUR SÉRIE ----------------------------------------------------------
 

  else  {                                      // Sinon si la sonde eau n'émet pas de signal (indique la présence d'eau).
  digitalWrite(delRouge, LOW);                 // Ferme la DELrouge.       
  digitalWrite(sondeHum, HIGH);                // Ouvre la sonde de l'humidité. 
  digitalWrite(sondeHum2, HIGH);               // Ouvre la sonde de l'humidité #2.
  delay(1000);                                 // Un delais d'une seconde ensuite.  
  lcd.print("HUMID. PLATEAU= ");               // Affiche la phrase: HUMID. PLATEAU =
  detValeurana2 = analogRead(captAna2);        // Mesure le résultat analogique de la sonde d'humidité #2.

  int pourcentageHumidite2 = map(detValeurana2,humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité #2.
  delay(3000);                                 // Durée de l'affichage (3 secondes). 
  lcd.print(pourcentageHumidite2);             // Afficher ensuite la valeur en % de la sonde d'humidité #2.
  lcd.println("%");                   
  delay(3000);                                 // Durée de l'affichage (3 secondes).
  lcd.setCursor(0,2);                          // Ligne 2
  lcd.print("HUMID. TERRE= ");                 // Affiche la phrase: HUMID. TERRE =
  detValeurana = analogRead(captAna);          // Mesure le résultat analogique de la sonde d'humidité.

  int pourcentageHumidite = map(detValeurana, humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité.
  delay(3000);                                 // Durée de l'affichage (3 secondes).   
  lcd.print(pourcentageHumidite);              // Affiche ensuite la valeur en % de la sonde d'humidité.
  lcd.println("%");                      
  delay(3000);                                 // Durée de l'affichage (3 secondes).
  lcd.clear();                                 // Effacer l'écran.

}
  

//------------------------------------------------- COMMANDE POUR ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ INDIQUE UN SOL TROP SEC -----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//--------------------------------------- COMMANDE POUR CESSER D'ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ #2 INDIQUE QUE LE PLATEAU EST REMPLI D'EAU -----------------------------------------------------

  detValeurana2 = analogRead(captAna2);                 // Mesure le résultat analogique de la sonde d'humidité #2.

  int pourcentageHumidite2 = map(detValeurana2,humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité #2.
  delay(2000);                                          // Pause de 3 secondes.
  if (pourcentageHumidite2 > 36) {                      // Si la valeur en % de la sonde d'humidité est plus grande que 36%.
  lcd.setCursor(0,0);                                   // Ligne 0
  lcd.print("PLATEAU PLEIN D'EAU");                     // Affiche la phrase: PLATEAU PLEIN D'EAU 
  lcd.setCursor(0,1);                                   // Ligne 1
  lcd.print(" 3mm:30% / 1.5cm:80%");                    // Affiche la phrase: 3mm:30% / 1.5cm:80%
  lcd.setCursor(0,3);                                   // Ligne 2
  lcd.print("ARRET DE L'ARROSAGE");                     // Affiche la phrase: ARRET DE L'ARROSAGE
  delay(5000);                                          // Durée de l'affichage (5 secondes).
  lcd.clear();                                          // Effacer l'écran.

  goto tempo;                                           // La fonction "goto" permet à la programmation de revenir au terme "tempo" pour s'arrêter pendant 12h.
  }                                                     // La fonction évite ici que le système continu de fonctionner (d'arroser) pour éviter que le plateau déborde pendant les 15 tests d'arrosage.
  else {
  detValeurana = analogRead(captAna);                   // Mesure le résultat analogique de la sonde d'humidité.
  int pourcentageHumidite = map(detValeurana, humide, sec, 100, 0);
  delay(3000);                                          // Pause de 3 secondes.
  (pourcentageHumidite < 70);                           // Si la valeur analogique de la sonde d'humidité est plus petite que 70%.
  
  }
  for(int arrosage = 1 ; arrosage <= 15 ; arrosage = arrosage+1){  // Un cycle maximun de 15 tests de mesure et d'arrosage sont effectués.


  //----------------------------------------------- VÉRIFICATION DU NIVEAU D'EAU DANS LE RÉSERVOIR APRÈS CHAQUE ARROSAGE -----------------------------------------------------------------------------------------

 
  delEtat = digitalRead(delRouge);                      // Lire état actuel de la DELrouge.

  if (digitalRead(sondeEau)){                           // Si tu détecte la sonde eau (la Sonde émet un signal lorsqu'il n'y a pas d'eau).

 //---------------------------------------------- MESSAGE ET SIGNAL D'ALARME AUDITIF ET VISUEL AFIN DE REMPLIR LE RÉSERVOIR --------------------------------------------------------------------------------------


  lcd.setCursor(0,0);                                   // Ligne 0
  lcd.print("      REMPLIR");                           // Affiche la phrase: REMPLIR.
  lcd.setCursor(0,1);                                   // Ligne 1
  lcd.print("   RESERVE D'EAU");                        // Affiche la phrase: RESERVE D'EAU.
  delay(3000);                                          // Durée de l'affichage (3 secondes).
  lcd.clear();                                          // Effacer l'écran.
  digitalWrite(delRouge, HIGH);                         // Allume la DELrouge.
                                                     

  for (int thisNote1 = 0; thisNote1 < 8; thisNote1++) { // Procédé pour faire jouer les notes de la mélodie ???
    int noteDurations1 = 200;                           // Le temps pour toutes les notes.
    tone(2, melody1[thisNote1], noteDurations1);        // Active la mélodie.
    int pauseBetweenNotes = noteDurations1 * 1.30;      // Pause entre les notes.
    delay(pauseBetweenNotes);                           // Pause entre les notes.
    noTone(2);                                          // Arrêt de la note sur la pin 2.
  }                                           
   goto tempo;                        // La fonction "goto" placé à la fin de l'avertissement (remplir le réservoir) permet à la programmation de revenir au terme "tempo" pour s'arrêter pendant 12h.
  }
  //----------------------------------------------- APRÈS LE TEST DU NIVEAU D'EAU LA COMMANDE POUR ARROSER EST ENCLENCHÉ -----------------------------------------------------------------------------------------


  else  {                                     // Sinon si la sonde eau n'émet pas de signal (indique la présence d'eau).
  digitalWrite(delRouge, LOW);                // Ferme la DELrouge.
  }
  //--------------------------------------- VÉRIFICATION DU NIVEAU D'EAU DANS LE PLATEAU APRÈS CHAQUE ARROSAGE ---------------------------------------------------------------------------------------------------

  detValeurana2 = analogRead(captAna2);       // Mesure le résultat analogique de la sonde d'humidité #2.

  int pourcentageHumidite2 = map(detValeurana2,humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité #2.
  delay(2000);                                // Pause de 2 secondes.  
  if (pourcentageHumidite2 > 36) {            // Si la valeur en % de la sonde d'humidité est plus grande que 36%.
  lcd.setCursor(0,0);                         // Ligne 0
  lcd.print("PLATEAU PLEIN D'EAU");           // Affiche la phrase: PLATEAU PLEIN D'EAU 
  lcd.setCursor(0,1);                         // Ligne 1
  lcd.print(" 3mm:30% / 1.5cm:80%");          // Affiche la phrase: 3mm:30% / 1.5cm:80%
  lcd.setCursor(0,3);                         // Ligne 2          
  lcd.print("ARRET DE L'ARROSAGE");           // Affiche la phrase: ARRET DE L'ARROSAGE
  delay(5000);                                // Durée de l'affichage (5 secondes).
  lcd.clear();                                // Effacer l'écran.
  goto tempo;                                 // La fonction "goto" permet à la programmation de revenir au terme "tempo" pour s'arrêter pendant 12h.
  }                                           // La focntion évite ici que le système continu de fonctionner (d'arroser) pour éviter que le plateau déborde pendant les 15 tests d'arrosage.


  detValeurana = analogRead(captAna);         // Mesure le résultat analogique de la sonde d'humidité.

  int pourcentageHumidite = map(detValeurana, humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité.
  delay(2000);                                // Pause de 2 secondes.     
  if (pourcentageHumidite < 70) {             // Si la valeur analogique de la sonde d'humidité est plus petite que 50%.
  delay(2000);                                // Pause de 2 secondes.
  lcd.setCursor(0,1);                         // Ligne 1
  lcd.print(  pourcentageHumidite);           // Affiche la valeur en % de la sonde de l'humidité.
  lcd.print("% ; ");
  lcd.print(arrosage);                        // Affiche le nombre d'arrosage effectué depuis le début sur l'écran LCD.
  lcd.print(" arrosage ");                    // Affiche le mot: ; arrosage 
  digitalWrite(relaisPomp, HIGH);             // Active la pompe.
  delay(15000);                               // Durée de l'activation pompe (15 secondes).
  digitalWrite(relaisPomp, LOW);              // Ferme la pompe.
  delay(20000);                               // Pause de la pompe pour laisser à l'eau le temps de pénétrer la terre (20 secondes).
  }
  

  //--------------------------------------- COMMANDE POUR CESSER D'ARROSER QUAND LA VALEUR DE LA SONDE D'HUMIDITÉ INDIQUE UN SOL HUMIDE -------------------------------------------------------------------------


  else  {                                     // Sinon (si la valeur de la sonde d'humidité est plus petite que 440).

  lcd.clear();                                // Effacer l'écran.
  lcd.setCursor(0,1);                         // Ligne 1
  lcd.print(" HUMID. TERRE = ");              // Affiche la phrase: / HUMID. TERRE =
  detValeurana = analogRead(captAna);         // Mesure le résultat analogique de la sonde d'humidité.

  int pourcentageHumidite = map(detValeurana, humide, sec, 100, 0); // La fonction MAP détermine en pourcentage le résultat de la sonde d'humidité.
  delay(3000);                                // Durée de l'affichage (3 secondes).   
  lcd.print(pourcentageHumidite);             // Affiche ensuite la valeur en % de la sonde d'humidité.
  lcd.println("%");
  delay(3000);                                // Durée de l'affichage (3 secondes).
  lcd.clear();                                // Effacer l'écran.
  lcd.setCursor(0,0);                         // Ligne 0
  lcd.print("TERRE ASSEZ HUMIDE");            // Affiche la phrase: TERRE ASSEZ HUMIDE.
  lcd.setCursor(0,2);                         // Ligne 2
  lcd.print("Haut dessus de 70%");            // Imprimer la phrase: Haut dessus de 50%.
  delay(3000);                                // Durée de l'affichage (3 secondes).
  lcd.clear();                                // Effacer l'écran.
  lcd.setCursor(0,1);                         // Ligne 1
  lcd.print("*** FIN DU CYCLE ***");          // Imprime la phrase: *** FIN DU CYCLE *** 
  lcd.setCursor(0,2);                         // Ligne 2
  lcd.print("**** D'ARROSAGE ****");          // Imprime la phrase: **** D'ARROSAGE ****
  delay(3000);                                // Durée de l'affichage (3 secondes).
  lcd.clear();                                // Effacer l'écran.


  goto tempo;                                 // La fonction "goto" permet à la programmation de revenir au terme "tempo" pour s'arrêter pendant 12h.
  }
  }
  } 
  }
  }
  
   
 //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
   


     