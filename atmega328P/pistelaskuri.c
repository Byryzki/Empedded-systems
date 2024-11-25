//prosessorin kirjastot
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <string.h>

//lisälaitteiden kirjastot
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
#include <LiquidCrystal_I2C.h>

// Oheislaitekirjastojen määrittelyjä
SoftwareSerial SWSerial( 3, 2 ); // RX, TX
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );
String tagId = "None", dispTag = "None";
byte nuidPICC[4];

LiquidCrystal_I2C lcd(0x27,16,2);

struct player
//Tietorakenne rekisteröidylle pelaajalle
{
  uint8_t newPlayer = 1;
  int score;
  uint8_t id[7];
};

// Global muuttujat
uint8_t player_count = 10; // vaikka kymmenen pelaajan maksimi
uint8_t players_added = 0;
player PlayerRegister[10];

void setup(void)
//määritykset omissa funktioissaan modulaarisuuden parantamiseksi
{
  Display_setup();
  NFCsetup();
}

void NFCsetup()
//RFID lukijan määritykset
{
  Serial.begin(115200); //Baud rate
  nfc.begin();
  nfc.SAMConfig();
}

void Display_setup()
//LCD näytön määritykset
{
  lcd.begin(16,2);
  lcd.noBacklight(); //näyttö pois päältä
  //lcd.backlight();
}
 
void loop()
//Pääsilmukka
{
  readNFC(); // Näyttöön valot vasta kun kortti havaitaan
}

uint8_t CardPresent() //näytetäänkö korttia paraikaa lukijaan? oispa interrupt service tähän :/
{
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Bufferi tunnisteen varastointiin
  uint8_t uidLength;
  return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
} 

void readNFC()  //kortinlukijan logiikka
{
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  delay(500);  //UX

  if(nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength))
  {
    uint8_t registerPos = PlayerFinder(uid, uidLength); //käydään Pelaajarekisteri läpi
    printDisplay(registerPos); //näyttö esitys alkaa
  }
}

void printDisplay(uint8_t registerPos)
{
  uint8_t player = registerPos + 1;
  uint8_t score = PlayerRegister[registerPos].score;
  
  lcd.backlight();

  if(PlayerRegister[registerPos].newPlayer == 1)
  // uusi käyttäjä tervehdys
  {
    lcd.setCursor(0,0); lcd.print("Hello");
    delay(1000);
    lcd.setCursor(0,1); lcd.print("stranger");
    delay(2000);
    lcd.clear();
    delay(1000);

    lcd.setCursor(0,0); lcd.print("You're ");
    delay(1000);
    lcd.setCursor(7,0); lcd.print("player ");
    lcd.setCursor(14,0); lcd.print(player);
    delay(1000);

    lcd.setCursor(0,1); lcd.print("with score: ");
    delay(1000);
    lcd.setCursor(12,1); lcd.print(score);
    delay(2000);
    lcd.clear();
    delay(1000);
  }
  else
  // vanhan käyttäjän tervehtiminen
  {
    lcd.setCursor(0,0);
    lcd.print("Hello ");
    delay(1000);
    lcd.setCursor(6,0); lcd.print("player ");
    lcd.setCursor(13,0); lcd.print(player);
    delay(1000);

    lcd.setCursor(0,1);
    lcd.print("Your score: ");
    delay(1000);
    lcd.setCursor(12,1);
    lcd.print(score);
    delay(2000);
    lcd.clear();
    delay(1000);
  }
  // ohjeet
  lcd.setCursor(0,0);
  lcd.print("Hold card to");
  lcd.setCursor(0,1);
  lcd.print("increase score");
  delay(2000);
  lcd.clear();
  delay(1000);

  while(CardPresent() || CardPresent() || CardPresent())
  // Pisteiden lisäys. Kolme tarkastusta joka pisteen kohdalla viansiedollisista syistä
  {
    lcd.setCursor(0,0); lcd.print("Player ");
    lcd.setCursor(7,0); lcd.print(player);
    lcd.setCursor(0,1); lcd.print("Score: ");
    lcd.setCursor(8,1); lcd.print(score);
    delay(1000);
    ++score;
  }

  if(PlayerRegister[player-1].score != score){PlayerRegister[player-1].score = score-1;}

  lcd.clear();
  lcd.noBacklight();
}

uint8_t PlayerFinder(uint8_t uid[], uint8_t uidlength)
//Luo tarvittaessa pelaajan ja palauttaa tunnistekortin ID:tä vastaavan pelaajan paikan pelaajarekisterissä
{
  if(players_added != 0)
  //Pelaajarekisteri tyhjä
  {
    for(int i=0;i<players_added;i++) //etsitään pelaajaa rekisteristä
    //per pelaaja
    {
      uint8_t new_player = 0;
      for (uint8_t ii = 0; ii < uidlength; ii++)
      {
        if(PlayerRegister[i].id[ii] != uid[ii])
        //per tunnisteen merkki
        {
          new_player = 1;
          break; 
        }
      }
      if(new_player == 0)
      //tunniste löytyy pelaajarekisteristä
      {
        PlayerRegister[i].newPlayer = 0;
        return i;
      }
    }
  }

  struct player newp;
  for (uint8_t i = 0; i < uidlength; i++){newp.id[i] = uid[i];}
  newp.score = 0;
  newp.newPlayer = 1;

  PlayerRegister[players_added] = newp;
  ++players_added;

  return players_added-1;
}


//  Mitä voisi vielä laittaa: Interrupt service

/*
void sandman()
// Odottaa, että korttia näytetään lukijalle
{
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}

ISR(INT0_vect)
// Interrupt service, joka herättää laitteen kun korttia näytetään
{
	// Clear INT0 flag
	EIFR |= (1 << INTF0);

	// Wake up from sleep mode
	sleep_disable();
}
*/
