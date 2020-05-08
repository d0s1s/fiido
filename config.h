#include <Arduino.h>

//=================== VARIABLES CONFIGURABLES POR EL USUARIO ===========

struct ConfigContainer {
	// -------------- DAC

	// Dirección del bus I2C [DAC] --> (0x60) suele ser por defecto.
	// Si dejamos el valor a 0 realizará la detección automática al
	// arranque. No cambiar si no sabemos con certeza la dirección de
	// memoria denuestro DAC.
	uint8_t dir_dac = 0;

	// -------------- PEDALEO

	// Tiempo en ms para detectar la desactivación del pedaleo.
	// Valores recomendados 500 ó 750.
	unsigned long tiempo_act = 500;

	// Número de imán a la que activamos pedaleo.
	byte activa_pedaleo = 2;

	// Desacelera al parar los pedales.
	boolean desacelera_al_parar_pedal = true;

	// Rampa de desaceleración al parar los pedales.
	// A mayor número, más rápido desacelera. 1 ó 2.
	byte rampa_desaceleracion = 1;

	// -------------- TONOS

	// Habilita los tonos de inicialización del sistema.
	// Recomendado poner a True si se tiene zumbador en el pin 11.
	boolean buzzer_activo = true;

	// -------------- ASISTENCIA 6 KM/H

	// True si se desea activar la posibilidad de acelerar desde
	// parado a 6 km/h pulsando el botón del pito.
	boolean asistencia_pulsador = false;
	
	// True si se desea activar la posibilidad de acelerar desde
	// parado a 6 km/h accionando el acelerador.
	boolean asistencia_acelerador = true;

	// -------------- PROGRESIVOS
  
	// Retardo en segundos para ponerse a velocidad máxima o crucero.
	byte retardo_aceleracion = 6;

	// Retardo para inciar progresivo tras parar pedales.
	// Freno anula el tiempo.
	byte retardo_inicio_progresivo = 10;

	// Suavidad de los progresivos, varía entre 1-10.
	// Al crecer se hacen más bruscos.
	byte suavidad_progresivos = 1;

	// Suavidad de los autoprogresivos, varía entre 1-10.
	// Al crecer se hacen más bruscos.
	byte suavidad_autoprogresivos = 1;

	// -------------- DEBUG

	// Habilita la salida de datos por consola.
	boolean habilitar_consola = false;
};

// EOF
