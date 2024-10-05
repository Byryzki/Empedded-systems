//prossun riippuvuudet
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <string.h>

//lisälaitteiden riippuvuudet
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
#include <LiquidCrystal_I2C.h>

// (Portti)vakiot
#define POWER_SWITCH PD2

// Global muuttujat
volatile uint8_t POWER_STATE = 0;
volatile uint8_t TIME = 0;
volatile uint16_t ADC_VALUE = 0;

SoftwareSerial SWSerial( 3, 2 ); // RX, TX
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );
String tagId = "None", dispTag = "None";
byte nuidPICC[4];

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

struct player
{
  uint8_t newPlayer = 1;
  int score;
  uint8_t id[7];
};

int player_count = 5; // viiden pelaajan maksimi vaikka
int players_added = 0;
struct player PlayerRegister[5];

void setup(void)
{
  Display_setup();
  NFCsetup();
}

void NFCsetup()
{
  Serial.begin(115200); //Baud rate
  nfc.begin();
  // Configure board to read RFID tags
  nfc.SAMConfig();
}

void Display_setup()
{
  lcd.begin(16,2); //define the character spaces
  lcd.noBacklight(); //power on the display
  //lcd.backlight(); //power off the display
}
 
void loop()
{
  readNFC(); // Näyttöön valot vasta kun kortti havaitaan
}

boolean CardPresent() //näytetäänkö korttia paraikaa lukijaan? oispa interrupt service tähän :/
{
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
} 

void readNFC()  //kortinlukijan logiikka
{
  boolean success; 
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success)
  {
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++){Serial.print(" "); Serial.print(uid[i], DEC);}
    Serial.println();

    delay(1000);  // 1 second halt

    uint8_t registerPos = PlayerFinder(uid, uidLength);
    Serial.print("registerPos: "); Serial.print(registerPos); Serial.println();
    printDisplay(registerPos); //onpa nätti :D nuidPICC = kortin raakatunnus
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Waiting for a card...");
  }
}

void printDisplay(int registerPos)
{
  uint8_t player = registerPos + 1;
  uint8_t score = PlayerRegister[registerPos].score;
  
  lcd.backlight();

  if(PlayerRegister[registerPos].newPlayer == 1) // uusi käyttäjä
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

  lcd.setCursor(0,0);
  lcd.print("Hold card to");
  lcd.setCursor(0,1);
  lcd.print("increase score");
  delay(2000);
  lcd.clear();
  delay(1000);

  while(true)
  {
    if(!CardPresent()){break;}
    lcd.setCursor(0,0); lcd.print("Player ");
    lcd.setCursor(7,0); lcd.print(player);
    lcd.setCursor(0,1); lcd.print("Score: ");
    lcd.setCursor(8,1); lcd.print(score);
    delay(1500);
    ++score;
  }
  if(PlayerRegister[player-1].score != score){PlayerRegister[player-1].score = score-1;}

  lcd.clear();
  lcd.noBacklight();
}

uint8_t PlayerFinder(uint8_t uid[], uint8_t uidlength)  //palauttaa tunnistekortin ID:tä vastaavan pelaajan paikan pelaajarekisterissä
{
  for (uint8_t i = 0; i < uidlength; i++){Serial.print(uid[i]);} Serial.println();

  if(players_added != 0)
  {
    uint8_t old_player = 0;
    for(int i=0;i<players_added;i++) //etsitään pelaajaa rekisteristä
    {
      uint8_t new_player = 0;
      for (uint8_t ii = 0; ii < uidlength; ii++)
      {
        Serial.print("playerregisterID: "); Serial.print(PlayerRegister[i].id[ii]); Serial.println();
        Serial.print("uid: "); Serial.print(uid[ii]); Serial.println();

        if(PlayerRegister[i].id[ii] != uid[ii]) //tämä ei kai toimi! Johtuu kai siitä että C:llä ei voida vertailla string keskenään. Keksi kierto
        {
          new_player = 1;
          break; 
        }
      }
      if(new_player == 0)
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

  Serial.print("uuden pelaajan id: ");
  for (uint8_t i = 0; i < uidlength; i++){Serial.print(newp.id[i]);}
  Serial.println();

  PlayerRegister[players_added] = newp;
  ++players_added;
  Serial.print("pelaajia rekisterissä:"); Serial.println(players_added);

  return players_added-1;

}

/*
void LowPower() // Odottaa, että korttia näytetään lukijalle
{
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}


ISR(INT0_vect) // Interrupt service, joka herättäisi laitteen kun korttia näytetään
{
	// Clear INT0 flag
	EIFR |= (1 << INTF0);

	// Wake up from sleep mode
	sleep_disable();
}
*/
