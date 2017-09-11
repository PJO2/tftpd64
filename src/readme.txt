This is the complete source code for Tftpd32 and Tftpd64, the industry standard TFTP server.

The code is splitted into 2 parts : 
  - the GUI management (main thread)
  - the background process (other threads)

They communicate together by messages sent through a TCP socket. In addition to separate processing and display, this allow a remote monitoring for the service edition.

_common and _libs directories contains files which belongs to both parts.
GUI processing is in the _gui directory.
Demon and background processing is found into _services directory.
Initializations are in the _main directory

tftpd32.sln and .vcxproj files are the project files necessary for building the executables with Visual Studio.

All code is released under the European Public License, which is compatible with the GPLv2.
