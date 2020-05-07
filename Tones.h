#include <Arduino.h>

//======= FUNCIONES DE TONOS ===========================================

//================== TONES ===================
// Frecuencias 4 octavas de
int c[5]={131,262,523,1046,2093};       // Do
int cs[5]={139,277,554,1108,2217};      // Do#
int d[5]={147,294,587,1175,2349};       // Re
int ds[5]={156,311,622,1244,2489};      // Re#
int e[5]={165,330,659,1319,2637};       // Mi
int f[5]={175,349,698,1397,2794};       // Fa
int fs[5]={185,370,740,1480,2960};      // Fa#
int g[5]={196,392,784,1568,3136};       // Sol
int gs[5]={208,415,831,1661,3322};      // Sol#
int a[5]={220,440,880,1760,3520};       // La
int as[5]={233,466,932,1866,3729};      // La#
int b[5]={247,494,988,1976,3951};       // Si

void nota(int pin_piezo, int frec, int ttime) {
	tone(pin_piezo,frec);
	delay(ttime);
	noTone(pin_piezo);
}

void SOS_TONE(int pin_piezo) {
	nota(pin_piezo, b[3],150);delay(40);
	nota(pin_piezo, b[3],150);delay(40);
	nota(pin_piezo, b[3],150);delay(70);
	nota(pin_piezo, b[3],100);delay(40);
	nota(pin_piezo, b[3],100);delay(40);
	nota(pin_piezo, b[3],100);delay(70);
	nota(pin_piezo, b[3],150);delay(40);
	nota(pin_piezo, b[3],150);delay(40);
	nota(pin_piezo, b[3],150);delay(100);
}

void DAC_ERR_TONE(int pin_piezo) {
	for (int i = 1; i <= 6; i++) {
		nota(pin_piezo, i%2?b[3]:c[3],90);delay(500);
	}
}

void repeatTones(int pin_piezo, boolean trigger, int steps, int frequency, int duration, int delayTime) {
	if (trigger) {
		int cont = steps;
		while (cont-- > 0) {
			tone(pin_piezo,frequency,duration);
			if (delayTime > 0)
				delay(delayTime);
			//noTone(pin_piezo);
		}
	}
}

// EOF
