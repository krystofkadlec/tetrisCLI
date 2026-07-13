#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

struct termios orig_termios;

void set_canon_terminal_mode(){
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  printf("\x1b[?25h");
}

void handle_signal(int sig){
  set_canon_terminal_mode();
  fflush(stdout);
  exit(0);
}

void set_conio_terminal_mode(){
  struct termios new_termios;

  tcgetattr(STDIN_FILENO, &orig_termios);

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  new_termios = orig_termios;
  new_termios.c_lflag &= ~(ICANON | ECHO);

  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

  printf("\x1b[?25l");
}

bool kbhit(){
  // waiting for enter needs to be disabled for stdin too
  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0); // storing old settings
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // adding new flag

  char ch = getchar(); // test if there is a char in the buffer

  fcntl(STDIN_FILENO, F_SETFL, oldf); // return stdin settings

  // if there is a input in the buffer, put it back and return 1
  if(ch != EOF){
    ungetc(ch, stdin);
    return 1;
  }
  return 0;
}
