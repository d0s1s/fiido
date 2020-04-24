
                   Versión Con y Sin Acelerador (DAC)
                           Arduino Fiido 3.0
------------------------------------------------------------------------
PRINCIPALES NOVEDADES:
 * Detección automática de acelerador al inicio.
 * La detección de pedaleo se hace con millis().
 * Evita pedaleo inverso.
 * Función crucero en el acelerador.
 * Sensor PAS ajustado a la normativa europea.
 * Progresivos y auto_progresivos no lineales.
 * Posibilidad de usar asistencia a 6 km/h con botón.
 * Añadido buzzer para emitir avisos.
 * Parámetros configurables antes de compilar en fichero config.h.
------------------------------------------------------------------------
CÓDIGO FUENTE:
 * https://github.com/d0s1s/fiido
------------------------------------------------------------------------
SELECCIÓN DE MODOS EN EL ARRANQUE:
 * Al encender la bici se realiza una comprobación automática para
 * detectar si el acelerador está presente --> Si lo está selecciona el
 * MODO_ACELERADOR. Si no está el acelerador o su lectura es incorrecta,
 * se selecciona el MODO_SIN ACELERADOR. 
------------------------------------------------------------------------
MODO_ACELERADOR:
 * Este modo da la posibilidad de utilizar el acelerador que trae la
 * Fiido de manera legal.
 * 
 * Básicamente lo que hace es detectar pulsos mediante una interrupción
 * en el pin (pin_pedal). Si no se pedalea, no asiste el acelerador.
------------------------------------------------------------------------
MODO_SIN ACELERADOR:
 * Este modo inhabilita el acelerador si está presente. Si no lo está,
 * es seleccionado automáticamente en el inicio.
 * 
 * Básicamente lo que hace es asistir cuando se pedalea.
 * 
 * Podemos ir en tres niveles de asistencia:
 * 
 * - 25 km/h. (Modo eléctrico).
 * - 15 km/h. (Modo asistido).
 * -  6 km/h. (Modo eléctrico presionando botón).
 * 
 * El nivel de asistencia es el máximo que da la bici (25 km/h).
 * 
 * Si se desea ir a un nivel inferior de asistencia, se debe habilitar
 * el switch que trae la Fiido para entrar en el modo asistido de la
 * bici, el cual en la D1 y D2 se va a 15 km/h (En mi modelo D1 se
 * conserva la precisión de detección de pedaleo en este modo y los
 * progresivos).
 * 
 * Si se vuelva al modo de 25 km/h (desde asistido) y estamos pedalenado
 * en ese momento, hay que dar un toque (ligero) de freno para que
 * entregue potencia de nuevo. Si se cambia sin pedalear, no hay que
 * hacer nada.
 * 
 * Si se configura la asistencia de 6 km/h para usarla mientras se
 * presione el botón, la bicicleta asiste a dicha velocidad tanto si se
 * pedalea como si no (esto lo permite la normativa).
------------------------------------------------------------------------
CRUCERO EN MODO ACELERADOR:
 * Se trata de guardar el último valor del acelerador para no tener que
 * estar sujetando el acelerador.
 * 
 * La idea es fijar el acelerador a la velocidad deseada y soltar de
 * golpe para guardar el voltaje como nivel de asistencia.
 * 
 * Al parar y volver a pedalear, se va incrementando voltaje
 * gradualmente hasta llegar al nivel fijado. Si se vuelve a mover el
 * acelerador se toma este como nuevo valor.
 * 
 * Usamos un pin analógico de entrada conectado al
 * acelerador, por otra parte, mandaremos a la placa DAC mediante
 * comunicacion i2c el valor de salida hacia la controladora.
 * 
 * Para desfijar el nivel de asistencia, simplemente accionar un poco
 * el acelerador, hasta escuchar un pitido de anulación de crucero y
 * soltar. 
------------------------------------------------------------------------
PROGRESIVOS:
 * Una vez fijemos una velocidad de crucero con el acelerador, si
 * paramos de pedalear durante más de 10 segundos o accionamos el freno,
 * al volver a reanudar el pedaleo, la asistencia se iniciará desde 0
 * progresivamente hasta volver a alcanzar la velocidad anteriormente
 * fijada.
------------------------------------------------------------------------
AUTO_PROGRESIVOS:
 * Si se deja de pedalear, el motor se para como de costumbre, pero si
 * continuamos pedaleando antes de transcurridos 10 segundos no inciará
 * el progresivo desde 0 si no que el motor continuará a una velocidad
 * ligeramente inferior a la que íbamos.
 * 
 * Si se frena antes de los 10 segundos se anula la función y comenzará
 * el progresivo desde cero.
------------------------------------------------------------------------
ASISTENCIA A 6 KM/H DESDE PARADO:
 * Se puede accionar este modo mientras el botón esté accionado --> se
 * asiste a 6 km/h, ajustándose a la normativa si se usa sin pedalear.
 * 
 * Por defecto viene deshabilitado, hay que activarlo en el config antes
 * de compilar el programa.
------------------------------------------------------------------------
DEVELOPERS:
 * d0s1s a partir de la versión de ciberus y fulano con las
 * aportaciones de chusquete.
 * Varias funcionalidades de esta versión están escritas por dabadg.
------------------------------------------------------------------------
AGRADECIMIENTOS:
 * Grupo de Telegram de desarrollo privado y toda su gente --> pruebas,
 * ideas, feedback, etc.
 * 
 * Gracias a zereal por sus ideas de concepto.
 * Gracias a faeletronic, basbonald, Manoplas, etc por el testing.
------------------------------------------------------------------------
LINKS:
 * Ayuda, sugerencias, preguntas, etc. en el grupo Fiido Telegram:
 * 
 *                      http://t.me/FiidoD1Spain
 * 
 * Grupo Telegram de desarrollo privado. Si vas a montar el circuito y
 * necesitas ayuda o colaborar pide acceso en el general de arriba.
 *  
 * Canal con montaje, enlaces, programas, etc.
 * 
 *                       http://t.me/fiidolegal
------------------------------------------------------------------------
