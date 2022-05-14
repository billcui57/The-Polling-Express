#include "my_assert.h"
#include "kprintf.h"

uart *term;

void assert_init(uart *t) { term = t; }

void my_assert(int id) {

  // sets the uart used to be term's uart
  print_uart = term;

  printf("                  "
         "_-====-__-======-__-========-_____-============-__\r\n");
  printf("               _(                                              "
         "     _)\r\n");
  printf("            OO(             UH OH! FAILED ASSERTION !!!!!      "
         "       )_\r\n");
  printf("           0  (_              SAD CHOO CHOO NOISES   ):        "
         "     _)\r\n");
  printf("         o0     (_                                             "
         "   _)\r\n");
  printf(

      "        o         '=-___-===-_____-========-___________-===-==0-='\r\n");
  printf("      .o                                _________\r\n");
  printf("     . ______          ______________  |         |      _____\r\n");
  printf(

      "   _()_||__|| ________ |            |  |_________|   __||___||__\r\n");
  printf(

      "  ( CS 452  | |      | |            | __Y______00_| |_         _|\r\n");
  printf(" /"
         "-OO----OO\"\"=\"OO--OO\"=\"OO--------OO\"=\"OO-------OO\"=\"OO-"
         "------OO\"=P\r\n");
  printf("###############################################################"
         "######\r\n");
  printf("                                                               "
         "      \r\n");
  printf("                                ID: %d                               "
         "\r\n",
         id);
  printf("                                                               "
         "      \r\n");
  printf("###############################################################"
         "######\r\n");
}
