rysca
=====

jachrimo &amp; gfiorav &amp; riveraonada 

#Reciente

* Comprobado que ip funciona en más de un salto (wireshark)
* Arreglado un error que confundia host_addr y target_addr en ipsend & arp
* Otra vez tocado arp para que funcione en máquina de jesus

#Pasos terminados

* (TERMINADO) UDP no es funcional aún. Está implementado pero hay que depurarlo.

* (HECHO) Hay que arreglar ARP para que coja nuestra IP
	-mejora: crear una cache ARP
	[ MOVIDON ENTRE ORDENADORES ]

* (FUNCIONAL) Hay que hacer que ip_send() distinga las direcciones broadcast y multicast
	-posibilidad: que mande a MAC broadcast
	-mejora: que las mapee a MAC multicast

#Pendiente

* Terminar cliente : { servidor que procese request con peticiones especificas, 
cliente tiene que imprimir lo que recibe }

* Quitar las estructuras (como ip_header, udp_header...) de *.h y ponerlas en *.c

* Que cada fichero *.c solo llame como cabecera a su *.h
	-le pasa a algun fichero (es por limpieza)

* Hay algo en trigger update que hace que actualice siempre que le llega un mensaje
	-revisar, si las tablas no cambian no debe hacer nada

* (HECHO) --Random timers para update--

* Mapeo real de ip-mac multicast

* checksum udp

* Split Horizon

¡Y creo que tendremos un diez!
