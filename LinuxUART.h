#pragma once
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

class UART
{
private:
  const char*	 port;
  struct termios serial;
  int            file;
  char           buffer[1];
public:
  UART(const char* port_);
  bool upd();
  char readChar();
  void writeStr(char* str);
  void endl();
};
UART::UART(const char* port_):port(port_)
{
  
  file = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
  if (tcgetattr(file, &serial) < 0)	perror("Getting configuration");
  serial.c_iflag = 0;
  serial.c_oflag = 0;
  serial.c_lflag = 0;
  serial.c_cflag = 0;

  serial.c_cc[VMIN]  = 0;
  serial.c_cc[VTIME] = 0;

  serial.c_cflag = B9600 | CS8 | CREAD;
  tcsetattr(file, TCSANOW, &serial);
}


char
UART::readChar()
{
  return buffer[0];
}
bool UART::upd()
{

  return (read(file, buffer, sizeof(char)) > 0);
}
void
UART::writeStr(char* str)
{
write(file, str, strlen(str));
}

void
UART::endl()
{
  char* str = "\n";
  write(file, str, strlen(str));
}