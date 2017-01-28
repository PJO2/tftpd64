This is the complete source code for Tftpd32 and tftpd64.

The code has been splitted into 2 parts : 
  - the GUI management
  - the background process

They communicate together by messages sent through a TCP socket. In addition to separate processing and display, this allow a remote monitoring for the service edition.

GUI processing is in the _gui directory, daemon or background processing is found into _services directory. 


All code is released under the European Public License, which is compatible with the GPLv2.