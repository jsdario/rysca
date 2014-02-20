rysca
=====

Implementations of IP, ARP, UDP, RIP over a tiny ethernet library
designed from rawnetcc. Works on Wifi interfaces also.

By jachrimo &amp; gfiorav &amp; riveraonada 

# Laboratory of Networks Advanced Services

To compile the source the library 'rawnet.h' is needed.
You can run the services opening the one at higher level
but it will always need services at lower ones.

All layers have been developed acording to the RFC's and should
interact properly in a linux debian system.

* Example of execution:

cd path/to/rip_server
rawnetcc server *.c ../files/*.c
./server [--verbose]