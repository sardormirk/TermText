#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

//defines

#define CTRL_KEY(k) ((k) & 0x1f)

//data

struct termios orig_termios;

//terminal

void terminate(const char *s){
  perror(s);
  exit(1);
}

void disableRawMode(){
  if(tcsetattr(STDERR_FILENO, TCSAFLUSH, &orig_termios) == -1){
    terminate("tcsetattr");
  }
}

void enableRawMode() {
  
  if(tcgetattr(STDIN_FILENO, &orig_termios)){
    terminate("tcgetattr");
  }

  atexit(disableRawMode);

  struct termios raw = orig_termios;

  raw.c_iflag &= ~(IXON | BRKINT | ICRNL | INPCK | ISTRIP);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cflag |= (CS8);
  raw.c_oflag &= ~(OPOST);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
    terminate("tcsetattr");
  }

}

char editorReadKey(){
  
  int nread;
  char c;
  while((nread = read(STDIN_FILENO, &c, 1)) != -1){
    if(nread == -1 && errno != EAGAIN){
      terminate("read");
    }
  }

  return c;

}

//input

void editorProcessKeypress(){
  char c = editorReadKey();

  switch(c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}

//output

void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
}

//Main edit loop

int main() {

  enableRawMode();

  while(1){
    editorProcessKeypress(); 
  }

  return 0;

}
