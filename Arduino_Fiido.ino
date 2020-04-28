#include <Adafruit_MCP4725.h>
#include <Arduino.h>
#include "I2CScanner.h"
#include "Tones.h"
#include "config.h"

const char* version = "Arduino Fiido 3.0";

I2CScanner i2cScanner;
Adafruit_MCP4725 dac;
ConfigContainer cnf;

//======= INSTRUCCIONES DE USO =========================================

// En el README.md

//======= PARÁMETROS CONFIGURABLES =====================================

// En el config.h

//======= PINES ========================================================

// Pin del acelerador.
const int pin_acelerador = A0;
// Pin sensor PAS, en Nano/Uno usar 2 ó 3.
const byte pin_pedal = 2;
// Pin de activación del freno.
const byte pin_freno = 3;
// Pin botón de pito.
const byte pin_boton = 7;
// Pin del zumbador.
const byte pin_piezo = 11;

//======= VARIABLES PARA CÁLCULOS ======================================

/* Valores del acelerador.
 * 209  --> 1.02 voltios.
 * 235  --> 1.15 voltios.
 * 329  --> 1.66 voltios.
 * 440  --> 2.15 voltios.
 * 842  --> 4.12 voltios.
 * 1023 --> 5.00 voltios.
 */
const int a0_valor_reposo = 209;
const int a0_valor_corte = 235;
const int a0_valor_minimo = 329;
const int a0_valor_6kmh = 440;
const int a0_valor_limite = 842;

// Contadores de paro, aceleración y auto_progresivo.
int contador_retardo_aceleracion = 0;
unsigned long contador_retardo_inicio_progresivo = 0;
int bkp_contador_retardo_aceleracion = 0;
boolean auto_progresivo = false;

// Progresivos.
const float fac_p = 1.056 - 0.056 * cnf.suavidad_progresivos;
// Aviso de final de progresivo.
boolean aviso = true;
// Variables para auto_progresivos.
float fac_b = 0.0;
float fac_a = 0.0;
const float fac_c = cnf.suavidad_autoprogresivos / 10.0;

// Almacena el último valor asignado al DAC.
int nivel_aceleracion_prev = a0_valor_reposo;
// Valor recogido del acelerador.
int v_acelerador = a0_valor_reposo;
// Valor de crucero del acelerador.
int v_crucero = a0_valor_reposo;
// Variable que almacena el estado de notificación de fijar crucero.
boolean crucero_fijado = false;

// Controles de tiempo.
unsigned long establece_crucero_ultima_ejecucion_millis;
unsigned long anula_crucero_ultima_ejecucion_millis;
unsigned long control_contadores;
boolean actualizacion_contadores = false;

// Almacena la velocidad de crucero del loop anterior.
int vl_acelerador_prev = 0;
// Contador para el loop del crucero.
byte contador_loop_crucero = 0;
// Contador de frenadas en cada vuelta del loop.
byte frenadas = 0;
// Estado del freno. True -HIGH- Desactivado. False -LOW- Activado.
boolean freno = true;
// Estado del botón. True -HIGH- Desactivado. False -LOW- Activado.
boolean boton = true;
// Con acelerador o sin acelerador.
boolean modo_acelerador = true;

//======= Variables de interrupción ====================================

// Último tiempo de cambio de estado del sensor PAS.
volatile unsigned long ultimo_evento_pas = millis();
// Variable para la detección del pedaleo.
volatile boolean pedaleo = false;
// Variable donde se suman los pulsos del sensor PAS.
volatile byte a_pulsos = 0;
// Variable donde se suman las frenadas.
volatile byte a_frenadas = 0;
// Medidas de tiempo para la señal Low-High del PAS.
volatile int tiempo_pas_rising = 0;
// Medidas de tiempo para la señal High-Low del PAS.
volatile int tiempo_pas_falling = 0;

//======= FUNCIONES ====================================================

// --------- Utilidades

// Pasamos de escala acelerador --> DAC.
int aceleradorEnDac(int vl_acelerador) {
	return map(vl_acelerador, 0, 1023, 0, 4095); 
}

// --------- Pedal

void pedal() {
	if (ultimo_evento_pas > (unsigned long)(millis() - 10))
		return;

	// Lectura del Sensor PAS.
	boolean estado_pas = digitalRead(pin_pedal);

	// Tomamos medidas.
	if (estado_pas) {
		// Señal Low-High del sensor.
		tiempo_pas_rising = millis() - ultimo_evento_pas;
	} else {
		// Señal High-Low del sensor.
		tiempo_pas_falling = millis() - ultimo_evento_pas;
	}

	ultimo_evento_pas = millis();
	int pas_factor = tiempo_pas_rising;

	// Sólo consideramos pedaleo si estamos en Falling.
	if (!estado_pas)
		a_pulsos++;

	if (a_pulsos >= cnf.activa_pedaleo) {
		// Activamos pedaleo.
		pedaleo = true;
		a_pulsos = 0;
	} else {
		// Si el ciclo de Rising tarda mucho.
		if (pas_factor > 1500) {
			// Evita pedaleo inverso.
			a_pulsos = 0;
		}
	}
}

// --------- Freno

void frenar() {
	// Comprobación para evitar lecturas fantasmas.
	if (digitalRead(pin_freno) == LOW) {
		a_frenadas++;
		// Reiniciamos contadores.
		contador_retardo_inicio_progresivo = cnf.retardo_inicio_progresivo;
		contador_retardo_aceleracion = 0;
		bkp_contador_retardo_aceleracion = 0;
		// Control del flag para el aviso sonoro del progresivo.
		aviso = true;
	}
}

// --------- Acelerador

// Lectura del acelerador.
int leeAcelerador(byte nmuestras) {
	int cl_acelerador = 0;

	// Leemos nivel de acelerador.
	for (byte f = 1; f <= nmuestras; f++) {
		cl_acelerador = cl_acelerador + analogRead(pin_acelerador);
	}

	cl_acelerador = (int) cl_acelerador / nmuestras;

	// No dejamos que la lectura salga de los límites.
	cl_acelerador = constrain(cl_acelerador, a0_valor_reposo, a0_valor_limite);

	return cl_acelerador;
}

// Progresivo no lineal.
int calculaAceleradorProgresivoNoLineal() {
	if (!modo_acelerador)
		v_crucero = a0_valor_limite;
	
	float fac_n = a0_valor_reposo + 0.2 * v_crucero;
	float fac_m = (v_crucero - a0_valor_reposo) / pow (cnf.retardo_aceleracion, fac_p);
	int nivel_aceleraciontmp = (int) freno * (fac_n + fac_m * pow (contador_retardo_aceleracion, fac_p));

	return constrain(nivel_aceleraciontmp, a0_valor_reposo, v_crucero);
}

void validaAcelerador() {
	if (leeAcelerador(30) < a0_valor_corte) {
		repeatTones(pin_piezo, cnf.buzzer_activo, 1, 3100, 100, 0);
	} else {
		modo_acelerador = false;
		repeatTones(pin_piezo, cnf.buzzer_activo, 3, 3100, 50, 50);
	}
}

// --------- Crucero

void estableceNivel(int vl_acelerador) {
	// Esperamos 50 ms para ejecutar.
	if ((unsigned long) (millis() - establece_crucero_ultima_ejecucion_millis) > 50) {
		contador_loop_crucero++;
		establece_crucero_ultima_ejecucion_millis = millis();
	}

	// Si las vueltas del loop son mayores a las vueltas para fijar crucero.
	if (contador_loop_crucero > 1) {
		// Reinicio de contador.
		contador_loop_crucero = 0;

		// Si tenemos accionado el acelerador y la comparación de valores está dentro del rango de tolerancia.
		if (vl_acelerador_prev < vl_acelerador + 20 && vl_acelerador_prev > vl_acelerador - 20 && vl_acelerador > a0_valor_minimo) {
			v_crucero = vl_acelerador;
			crucero_fijado = true;
			vl_acelerador_prev = 0;
		} else {
			// Asignamos el valor actual a [vl_acelerador_prev].
			vl_acelerador_prev = vl_acelerador;
		}
	}
}

void anulaCruceroAcelerador() {
	if (crucero_fijado) {
		if ((unsigned long) (millis() - anula_crucero_ultima_ejecucion_millis) > 180) {
			int velocidad = leeAcelerador(30);

			if (velocidad < a0_valor_minimo && velocidad > a0_valor_corte) {
				v_crucero = a0_valor_reposo;
				crucero_fijado = false;
				repeatTones(pin_piezo, cnf.buzzer_activo, 1, 2000, 290, 100);
			}

			anula_crucero_ultima_ejecucion_millis = millis();
		}
	}
}

// --------- Debug

void testSensores() {
	Serial.print("Pedal Rising: ");
	Serial.print(tiempo_pas_rising);
	Serial.print(" ");
	Serial.print("Pedal Falling: ");
	Serial.print(tiempo_pas_falling);
	Serial.print(" ");
	Serial.print("Acelerador: ");
	Serial.print(leeAcelerador(30));
	Serial.print(" ");
	Serial.print("Freno: ");
	Serial.print(frenadas);
	Serial.println("");
}

// --------- Generales

void setup() {
	// Si cnf.dir_dac está a 0 se autodetecta la dirección del DAC.
	if (cnf.dir_dac == 0) {
		i2cScanner.Init();
	} else {
		i2cScanner.Init(cnf.dir_dac);
	}

	if (i2cScanner.isDacDetected()) {
		// Configura DAC.
		dac.begin(i2cScanner.getDacAddress());
		// Fija voltaje inicial en DAC.
		dac.setVoltage(aceleradorEnDac(a0_valor_reposo), false);

		// Configura pines.
		pinMode(pin_piezo, OUTPUT);
		pinMode(pin_pedal, INPUT_PULLUP);
		pinMode(pin_freno, INPUT_PULLUP);
		pinMode(pin_acelerador, INPUT);
		pinMode(pin_boton, INPUT_PULLUP);

		// Interrupciones.
		attachInterrupt(digitalPinToInterrupt(pin_pedal), pedal, CHANGE);
		attachInterrupt(digitalPinToInterrupt(pin_freno), frenar, FALLING);

		// Inicia serial.
		if (cnf.habilitar_consola) {
			Serial.begin(19200);
			while (!Serial) {};
			Serial.print("Arduino E-Bike ");
			Serial.println(version);
		}

		// Comprobamos si hay acelerador.
		validaAcelerador();

		// Ajusta configuración.
		cnf.retardo_aceleracion = cnf.retardo_aceleracion * (1000 / cnf.tiempo_act);
		cnf.retardo_inicio_progresivo = cnf.retardo_inicio_progresivo * (1000 / cnf.tiempo_act);

		// Anulamos el retardo por seguridad para que empiece progresivo al encender la bici.
		contador_retardo_inicio_progresivo = cnf.retardo_inicio_progresivo;

		// Cálculo de factores para auto_progresivo.
		if (cnf.retardo_inicio_progresivo > 0) {
			fac_b = (1.0 / cnf.retardo_aceleracion - 1.0) / (pow ((cnf.retardo_inicio_progresivo - 1.0), fac_c) - pow (1.0, fac_c));
			fac_a = 1.0 - pow (1.0, fac_c) * fac_b;
		}

		// Estabiliza imanes.
		cnf.activa_pedaleo = constrain(cnf.activa_pedaleo, 1, 6);
		// Estabiliza suavidad de los progresivos.
		cnf.suavidad_progresivos = constrain(cnf.suavidad_progresivos, 1, 10);
		// Estabiliza suavidad de los auto_progresivos.
		cnf.suavidad_autoprogresivos = constrain(cnf.suavidad_autoprogresivos, 1, 10);
		// Tono de finalización configuración del sistema.
		repeatTones(pin_piezo, cnf.buzzer_activo, 3, 3000, 90, 90);
	} else {
		// Tonos de error en detección de DAC.
		DAC_ERR_TONE(pin_piezo);
	}
}

void loop() {
	// Si el DAC es detectado.
	if (i2cScanner.isDacDetected()) {
		// Reposo al nivel de aceleración.
		int nivel_aceleracion = a0_valor_reposo;

		// Esperamos [tiempo_act] para verificar la desactivación del pedaleo.
		if (((unsigned long)(millis() - ultimo_evento_pas) > cnf.tiempo_act)) {
			// Si el sensor PAS no cambia, no estamos pedaleando.
			pedaleo = false;
		}

		// Control para el incremento de contadores.
		if (((unsigned long)(millis() - control_contadores) > cnf.tiempo_act)) {
			actualizacion_contadores = true;
			frenadas = a_frenadas;
			a_frenadas = 0;
			control_contadores = millis();
		}

		// Lectura de acelerador.
		if (modo_acelerador)
			v_acelerador = leeAcelerador(30);

		// Lecturas de los sensores.
		freno = digitalRead(pin_freno);
		boton = digitalRead(pin_boton);

		// Si no se pedalea.
		if (!pedaleo) {
			if (actualizacion_contadores)
				contador_retardo_inicio_progresivo++;
	
			// Lanzamos auto_progresivo.
			auto_progresivo = true;

			if (contador_retardo_aceleracion > 2) {
				bkp_contador_retardo_aceleracion = contador_retardo_aceleracion;
			}

			// Desacelera al parar los pedales.
			if (contador_retardo_aceleracion > 0 && cnf.desacelera_al_parar_pedal) {
				contador_retardo_aceleracion = contador_retardo_aceleracion - 1;
				nivel_aceleracion = calculaAceleradorProgresivoNoLineal();

				// Calculamos el nivel de aceleración.
				nivel_aceleracion = calculaAceleradorProgresivoNoLineal();

				if (contador_retardo_aceleracion < 0) {
					contador_retardo_aceleracion = 0;
				}
			} else {
				// Reiniciamos contador.
				contador_retardo_aceleracion = 0;
			}

			// Anula crucero con acelerador sin pedalear.
			if (modo_acelerador)
				anulaCruceroAcelerador();
		// Si se pedalea.
		} else {
			if (auto_progresivo && contador_retardo_inicio_progresivo < cnf.retardo_inicio_progresivo) {
				if (bkp_contador_retardo_aceleracion > cnf.retardo_aceleracion) {
					bkp_contador_retardo_aceleracion = cnf.retardo_aceleracion;
				}

				contador_retardo_aceleracion = (int) freno * bkp_contador_retardo_aceleracion * (fac_a + fac_b * pow (contador_retardo_inicio_progresivo, fac_c)) * v_crucero / a0_valor_limite;
			}

			// Quitamos auto_progresivo.
			auto_progresivo = false;
			// Reiniciamos contador.
			contador_retardo_inicio_progresivo = 0;

			if (actualizacion_contadores && contador_retardo_aceleracion < cnf.retardo_aceleracion) {
				contador_retardo_aceleracion++;
			}

			// Control del crucero.
			if (modo_acelerador)
				estableceNivel(v_acelerador);

			// Anula crucero con acelerador.
			if (modo_acelerador)
				anulaCruceroAcelerador();

			// Calculamos el nivel de aceleración.
			nivel_aceleracion = calculaAceleradorProgresivoNoLineal();
		}

		// Si el botón está pulsado.
		if (cnf.pulsador && boton == 0) {
			nivel_aceleracion = a0_valor_6kmh;
		}

		if (contador_retardo_aceleracion == cnf.retardo_aceleracion && aviso) {
			// Tono de Final de progresivo.
			repeatTones(pin_piezo, cnf.buzzer_activo, 1, 3000, 190, 1);
			aviso = false;
		}

		// Fijamos el acelerador si el valor anterior es distinto al actual.
		if (nivel_aceleracion_prev != nivel_aceleracion) {
			dac.setVoltage(aceleradorEnDac(nivel_aceleracion), false);
			nivel_aceleracion_prev = nivel_aceleracion;
		}

		// Reinicio de variable.
		actualizacion_contadores = false;

		// Debug.
		if (cnf.habilitar_consola) {
			testSensores();
		}
	}
}

// EOF
