# C.M-Ofelia-vol.-1



Este proyecto consiste en la creación de un controlador MIDI (Interfaz Digital de Instrumentos Musicales) versátil y funcional, basado en la placa Arduino Leonardo. El objetivo es proporcionar una interfaz física para interactuar con software de música digital (DAWs, sintetizadores virtuales, etc.) a través de mensajes MIDI estándar.

Componentes Principales y Funcionalidades:

Potenciómetros (Controladores Continuos): Se utilizan hasta 9 potenciómetros analógicos, leídos a través de un multiplexor (74HC4067), para enviar mensajes MIDI Control Change (CC). Estos potenciómetros pueden ser asignados a diversos parámetros de software musical como volumen, panorama, filtros, envolventes, etc., permitiendo un control granular y expresivo.

Joystick Analógico (Controladores Continuos Bidimensionales): Se incorpora un joystick analógico de dos ejes (X e Y), leído a través de otro multiplexor (74HC4067). Cada eje del joystick envía mensajes MIDI Control Change, lo que permite controlar dos parámetros simultáneamente con un solo componente físico. Esto es ideal para modulación, pitch bend, o cualquier otro par de parámetros interrelacionados.

Teclado Matricial 4x4 (Disparador de Notas y Acordes): Se utiliza un teclado matricial de 16 botones para disparar notas MIDI (Note On/Off). El teclado se puede utilizar para tocar melodías, ritmos o incluso acordes. El rango de notas que emite el teclado se puede desplazar a través de botones dedicados para acceder a diferentes octavas o registros.

Pantalla OLED (Retroalimentación Visual): Una pantalla OLED alfanumérica proporciona retroalimentación visual al usuario, principalmente mostrando el rango de notas MIDI actual seleccionado para el teclado matricial. Esto ayuda al usuario a saber en qué registro se encuentra el teclado.

Cambio de Rango de Notas (Control de Navegación): Se implementan dos botones dedicados para subir y bajar el rango de notas MIDI que se emiten al presionar las teclas del teclado matricial. Esto permite acceder a un amplio espectro de notas sin necesidad de un teclado físico grande.

Funcionamiento General:

El Arduino Leonardo lee continuamente el estado de los potenciómetros y el joystick a través de los multiplexores. Cualquier cambio detectado que supere un umbral se traduce a un mensaje MIDI Control Change y se envía a través de la conexión USB, que el software musical interpreta como entrada MIDI.

El teclado matricial se escanea constantemente en busca de pulsaciones y liberaciones de teclas. Cada tecla presionada se mapea a un número de nota MIDI dentro del rango seleccionado, y se envía un mensaje Note On. Al liberar la tecla, se envía el correspondiente mensaje Note Off. Se permite presionar múltiples teclas simultáneamente para generar acordes o texturas sonoras.

Los botones de subir y bajar rango permiten al usuario desplazar el conjunto de 16 notas MIDI asignadas al teclado matricial, ofreciendo acceso a diferentes partes del espectro musical. El rango actual se muestra en la pantalla OLED.

