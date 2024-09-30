#include <stdio.h>
#include <wiringPi.h>
#include <softTone.h>
#include <pthread.h>

#define SPKR 6
#define TOTAL 32
#define SW 5
#define LED 1
#define CDS 0  

int notes[] = {
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 391, 329.63, 329.63, 293.66, 293.66, 293.66, 0,
    391, 391, 440, 440, 391, 391, 329.63, 329.63,
    391, 329.63, 293.66, 329.63, 261.63, 261.63, 261.63, 0
};

int alarmOn = 0;
int musicPlaying = 0;
int dark = 0;

void *musicPlay(void *arg)
{
    softToneCreate(SPKR);
    while (1) {
        if (musicPlaying && alarmOn) {
            for (int i = 0; i < TOTAL; ++i) {
                if (!musicPlaying || !alarmOn) break;
                softToneWrite(SPKR, notes[i]);
                delay(280);
            }
        } else {
            softToneWrite(SPKR, 0);
            delay(100);
        }
    }
    return NULL;
}

void *switchControl(void *arg)
{
    pinMode(SW, INPUT);
    pinMode(LED, OUTPUT);
    int lastButtonState = HIGH;
    int buttonState;

    while (1) {
        buttonState = digitalRead(SW);
        if (buttonState == LOW && lastButtonState == HIGH) {
            alarmOn = !alarmOn;
            digitalWrite(LED, alarmOn);
            musicPlaying = alarmOn;
            delay(200);
        }
        lastButtonState = buttonState;
        delay(10);
    }
    return NULL;
}

void *cdsControl(void *arg)
{
    pinMode(SW, INPUT);
    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);
    while (1) {
        if (digitalRead(CDS) == HIGH) {
            dark = 0;
            if (alarmOn) {
                digitalWrite(LED, HIGH);
                musicPlaying = 1;
            } else {
                digitalWrite(LED, LOW);
                musicPlaying = 0;
            }
        } else {
            dark = 1;
            digitalWrite(LED, LOW);
            musicPlaying = 0;
        }
        delay(100);
    }
    return NULL;
}

int main()
{
    wiringPiSetup();

    pthread_t music_thread, switch_thread, cds_thread;

    pthread_create(&music_thread, NULL, musicPlay, NULL);
    pthread_create(&switch_thread, NULL, switchControl, NULL);
    pthread_create(&cds_thread, NULL, cdsControl, NULL);

    pthread_join(music_thread, NULL);
    pthread_join(switch_thread, NULL);
    pthread_join(cds_thread, NULL);

    return 0;
}
