/*
 * gf-puzzle (Girlfriend Puzzle)
 *
 * ATtiny85-based hardware project,
 * a physical puzzle as a gift to the girlfriend.
 *
 * The project will use two LEDs to flash out a binary-encoded
 * message. It is up to the girlfriend to figure out that the flashing
 * leds represent binary, binary represents decimal, decimal
 * represents ASCII characters, which form words in Pig Latin,
 * which can be translated to English.
 *
 * @author Ando Roots <ando@sqroot.eu> 2017
 * @link https://sqroot.eu/2017/girlfriend-puzzle
 * @licence MIT
 */
#include <avr/sleep.h> 
#include <avr/wdt.h> 

const byte BLINK_TIME = 50;

// Secret message to flash out
//
// I LOVE YOU (ENGLISH)
// IWAY OVELAY OUYAY (PIG LATIN)
// 73 87 65 89 79 86 69 76 65 89 79 85 89 65 89 (PIG LATIN IN DECIMAL ASCII)
const byte MESSAGE[] = {73, 87, 65, 89, 32, 79, 86, 69, 76, 65, 89, 32, 79, 85, 89, 65, 89};
const byte MESSAGE_LENGTH = 17;

const byte RED_PIN = 1;
const byte GREEN_PIN = 0;
const byte BLUE_PIN = 2;

ISR(WDT_vect) {
  // Don't do anything. This is just here so that we wake up.
}

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(4, INPUT);
  digitalWrite(4, HIGH);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
}

bool is_activated() {
  if (digitalRead(4) == LOW) {
    delay(10);
    if (digitalRead(4) == LOW) {
      return true;
    }
  }
  return false;
}

void blink(byte pin) {
  digitalWrite(pin, HIGH);
  delay(BLINK_TIME);
  digitalWrite(pin, LOW);
  delay(BLINK_TIME);
}

void sendCharacter(int characterIndex){
  for (int j = 0; j < 8; j++) {

      if (bitRead(MESSAGE[characterIndex], 7 - j)) {
        blink(GREEN_PIN);
      } else {
        blink(RED_PIN);
      }
    }
}

void sendMessage(){
  for (int i = 0; i < MESSAGE_LENGTH; i++) {
    sendCharacter(i);
    blink(BLUE_PIN);
  }
}


//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
//From: http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Limit incoming amount to legal settings

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
}

void loop() {

  ADCSRA &= ~(1<<ADEN); // Disable ADC, saves ~230uA
  setup_watchdog(8); // Setup watchdog to go off after 1sec
  sleep_mode(); // Go to sleep! Wake up 1sec later and check water
  ADCSRA |= (1<<ADEN); // Enable ADC
  
  if (is_activated()) {
    sendMessage();
  }
}
