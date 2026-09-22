// michelle schepers // sep 22 // hello world 
#include <unistd.h>

int main( int argc, char* argv[] ){ //
  write(STDOUT_FILENO, "Hello, world!\n", 1000);
  return 0; 
  }

//Replace the contents of this file with your hello_write.c
