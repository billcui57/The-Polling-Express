#include "my_assert.h"
#include "kprintf.h"

uart *term;

void assert_init(uart *t) { term = t; }

void my_assert(char *str) {
  assert_thrown = true;

  printf(term, "                  "
               "_-====-__-======-__-========-_____-============-__\r\n");
  printf(term, "               _(                                              "
               "     _)\r\n");
  printf(term, "            OO(             UH OH! FAILED ASSERTION !!!!!      "
               "       )_\r\n");
  printf(term, "           0  (_              SAD CHOO CHOO NOISES   ):        "
               "     _)\r\n");
  printf(term, "         o0     (_                                             "
               "   _)\r\n");
  printf(
      term,

      "        o         '=-___-===-_____-========-___________-===-==0-='\r\n");
  printf(term, "      .o                                _________\r\n");
  printf(term,
         "     . ______          ______________  |         |      _____\r\n");
  printf(
      term,

      "   _()_||__|| ________ |            |  |_________|   __||___||__\r\n");
  printf(
      term,

      "  ( CS 452  | |      | |            | __Y______00_| |_         _|\r\n");
  printf(term, " /"
               "-OO----OO\"\"=\"OO--OO\"=\"OO--------OO\"=\"OO-------OO\"=\"OO-"
               "------OO\"=P\r\n");
  printf(term, "###############################################################"
               "######\r\n");
  printf(term, "                                                               "
               "      \r\n");
  printf(term, "Cause: %s\r\n", str);
  printf(term, "                                                               "
               "      \r\n");
  printf(term, "###############################################################"
               "######\r\n");
}
