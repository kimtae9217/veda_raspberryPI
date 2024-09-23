#include <wiringPi.h>
#include <stdio.h>

#define SW 5
#define LED 1

int switchControl()
{
    int i;
    pinMode(SW, INPUT);
    pinMode(LED, OUTPUT);
    int ledstate = 0;
    int lastButtonState = HIGH;
    int buttonState;

    for(;;){
        buttonState = digitalRead(SW);
        if(digitalRead(SW) == LOW && lastButtonState == HIGH){
            ledstate = !ledstate;
            digitalWrite(LED, ledstate);
            delay(200);
        }
        lastButtonState = buttonState;
        delay(10);
    }
    return 0;
}

int main(int argc, char **argv)
{
    wiringPiSetup();
    switchControl();
    return 0;
}
