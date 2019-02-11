/***************************************************
  Game Timer by @jerware
 ****************************************************/

#define PROGMEM

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();

bool buttonDown = false;
bool heldActive = false;
bool timerPaused = true;
bool timerActive = false;
bool pauseAllowed = true;
int buttonPin = A0;
int buzzerPin = A4;
float buzzerMultiplier = 1.0f;
int minutesRemaining = 0;
int timerLength = 2 * 60;
long timeRemaining = timerLength * 60 * 1000; // milliseconds on the timer
long pTime = 0; // previous time
long buttonPressedTime = 0;
int ledColor = 0; // 2=green 1=yellow 0=red

int gpio1 = D2; // HIGH for duration of timer (LOW when paused)
int gpio2 = D3; // HIGH for duration of timer
int gpio3 = D4; // LOW for duration of timer (HIGH when paused)
int gpio4 = D5; // LOW for duration of timer

int setLength(String command);
int addTime(String command);
int setMinLeft(String command);
int cloudReset(String command);
int cloudPause(String command);
int cloudEnablePause(String command);

void setup() {
  Serial.begin(57600);

  // setup gpio1
  pinMode(gpio1, OUTPUT);
  pinMode(gpio2, OUTPUT);
  pinMode(gpio3, OUTPUT);
  pinMode(gpio4, OUTPUT);
  digitalWrite(gpio1, HIGH);
  digitalWrite(gpio2, HIGH);
  digitalWrite(gpio3, LOW);
  digitalWrite(gpio4, LOW);

  Particle.function("setLength", setLength);
  Particle.function("addTime", addTime);
  Particle.function("setMinLeft", setMinLeft);
  Particle.function("Reset", cloudReset);
  Particle.function("Pause", cloudPause);
  Particle.function("toggleAllowPause", cloudEnablePause);

  Particle.variable("minRemaining", minutesRemaining);
  Particle.variable("isPaused", timerPaused);
  Particle.variable("pauseAllowed", pauseAllowed);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  matrix.begin(0x70);
  matrix.clear();
  matrix.writeDisplay();

  RGB.control(true);
  RGB.color(255, 0, 0);
  RGB.brightness(255);

  toneStart();
}

void loop()
{
  ledColorManager();

  // button check
  if (digitalRead(A0) == LOW && !buttonDown)
  {
    Serial.println("down!");
    buttonDown = true;
    buttonPressedTime = millis();
    if (pauseAllowed || (!pauseAllowed && timeRemaining == timerLength * 60 * 1000))
    {
      pauseTimer();
    }
    delay(10);
  }
  else if (digitalRead(A0) == HIGH && buttonDown)
  {
    Serial.println("up!");
    buttonDown = false;
    heldActive = false;
    delay(10);
  }

  // button held (reset)
  if (digitalRead(A0) == LOW && buttonDown && !heldActive && millis() > buttonPressedTime + 2000)
  {
    if (pauseAllowed) resetTimer();
  }

  // decrement timer
  if (timeRemaining > 0 && !timerPaused)
  {
    long cTime = millis();
    timeRemaining -= cTime - pTime;
    pTime = cTime;
  }

  // expired
  else if (timeRemaining <= 0)
  {
    if (timerActive)
    {
      RGB.color(255, 0, 0);
      timerActive = false;
      Particle.publish("timer", "ended");
      toneGameOver();
      digitalWrite(gpio1, LOW);
      digitalWrite(gpio2, LOW);
      digitalWrite(gpio3, HIGH);
      digitalWrite(gpio4, HIGH);
    }
    if ((millis() / 500) % 2)
    {
      matrix.writeDigitNum(0, 0);
      matrix.writeDigitNum(1, 0);
      matrix.writeDigitNum(3, 0);
      matrix.writeDigitNum(4, 0);
    }
    else
    {
      matrix.clear();
    }
    matrix.writeDisplay();
  }

  drawClock();
}

void drawClock()
{
  // blink if paused
  if (timerPaused && ((millis() / 1000) % 2))
  {
    matrix.clear();
    matrix.writeDisplay();
  }
  else if (timeRemaining > 0)
  {
    int h = (timeRemaining / 1000 / 60 / 60) % 12;
    int m = (timeRemaining / 1000 / 60) % 60;
    int s = (timeRemaining / 1000) % 60;

    minutesRemaining = (h*60)+m;

    int hm = h * 100 + m;
    matrix.print(hm);

    // blink seconds
    if ((timeRemaining / 500) % 2) matrix.drawColon(true);
    else matrix.drawColon(false);

    // draw zeros if necessary
    if (h < 1) matrix.writeDigitNum(1, 0, false);
    if (m < 10) matrix.writeDigitNum(3, 0, false);
    matrix.writeDisplay();
  }
}

int setLength(String command)
{
  timerLength = atoi(command);
  return atoi(command);
}

int cloudReset(String command)
{
  resetTimer();
  return timerLength;
}

int cloudPause(String command)
{
  pauseTimer();
  if (timerPaused) return 1;
  else return 0;
}

int cloudEnablePause(String command)
{
  if (command == "") pauseAllowed = !pauseAllowed;
  else if (atoi(command) == 1) pauseAllowed = true;
  else if (atoi(command) == 0) pauseAllowed = false;
  return pauseAllowed;
}

int addTime(String command)
{
  timeRemaining += atoi(command) * 1000 * 60;
  return atoi(command);
}

int setMinLeft(String command)
{
  timeRemaining = atoi(command) * 1000 * 60 + 1000;
  return atoi(command);
}

void resetTimer()
{
  timerActive = false;
  heldActive = true;
  timeRemaining = timerLength * 60 * 1000; // 2 hours

  drawClock();

  timerPaused = true;

  Particle.publish("timer", "reset");

  toneReset();

  digitalWrite(gpio1, HIGH);
  digitalWrite(gpio2, HIGH);
  digitalWrite(gpio3, LOW);
  digitalWrite(gpio4, LOW);
}

void pauseTimer()
{
  // pause timer
  if (timeRemaining > 0 && !timerPaused)
  {
    digitalWrite(gpio1, LOW);
    digitalWrite(gpio3, HIGH);
    RGB.color(255, 0, 0);
    char buffer[10];
    itoa(minutesRemaining, buffer, 10);
    String str1 = "paused (";
    String str2 = " minutes remaining)";
    String contents = str1 + buffer + str2;
    Serial.println(contents);
    Particle.publish("timer", contents);
    timerPaused = true;
    tonePause();
  }
  // unpause or start timer
  else if (timeRemaining > 0 && timerPaused)
  {
    digitalWrite(gpio1, HIGH);
    digitalWrite(gpio3, LOW);
    RGB.color(0, 255, 0);
    // reset time check
    pTime = millis();
    if (!timerActive)
    {
      timerActive = true;
      Particle.publish("timer", "started");
    }
    else Particle.publish("timer", "resumed");
    timerPaused = false;
    toneUnpause();
  }
}

void toneStart()
{
  tone(buzzerPin, 1000 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 2000 * buzzerMultiplier);
  delay(50);
  noTone(buzzerPin);
}

void tonePause()
{
  tone(buzzerPin, 800 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 700 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 600 * buzzerMultiplier);
  delay(50);
  noTone(buzzerPin);
}

void toneUnpause()
{
  tone(buzzerPin, 800 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 900 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1000 * buzzerMultiplier);
  delay(50);
  noTone(buzzerPin);
}

void toneReset()
{
  tone(buzzerPin, 1000 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1500 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 2000 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 2500 * buzzerMultiplier);
  delay(50);
  noTone(buzzerPin);
}

void toneGameOver()
{
  tone(buzzerPin, 2000 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1800 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1600 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1400 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1200 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 1000 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 800 * buzzerMultiplier);
  delay(50);
  tone(buzzerPin, 600 * buzzerMultiplier);
  delay(50);
  noTone(buzzerPin);
}

void ledColorManager()
{
  if (timerPaused || !timerActive && ledColor != 0)
  {
    RGB.color(255, 0, 0);
    ledColor = 0;
  }
  else if (!timerPaused && timerActive)
  {
    if (timeRemaining < 1000 * 60 * 15 && ledColor != 1)
    {
      RGB.color(255, 255, 0);
      ledColor = 1;
    }
    else
    {
      RGB.color(0, 255, 0);
      ledColor = 2;
    }
  }
}
