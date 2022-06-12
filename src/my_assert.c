#include "my_assert.h"
#include "kprintf.h"

bool assert_thrown;

void my_assert(char *str) {
  assert_thrown = true;

  printf(BW_COM2, "                  "
                  "_-====-__-======-__-========-_____-============-__\r\n");
  printf(BW_COM2,
         "               _(                                              "
         "     _)\r\n");
  printf(BW_COM2,
         "            OO(             UH OH! FAILED ASSERTION !!!!!      "
         "       )_\r\n");
  printf(BW_COM2,
         "           0  (_              SAD CHOO CHOO NOISES   ):        "
         "     _)\r\n");
  printf(BW_COM2,
         "         o0     (_                                             "
         "   _)\r\n");
  printf(
      BW_COM2,

      "        o         '=-___-===-_____-========-___________-===-==0-='\r\n");
  printf(BW_COM2, "      .o                                _________\r\n");
  printf(BW_COM2,
         "     . ______          ______________  |         |      _____\r\n");
  printf(
      BW_COM2,

      "   _()_||__|| ________ |            |  |_________|   __||___||__\r\n");
  printf(
      BW_COM2,

      "  ( CS 452  | |      | |            | __Y______00_| |_         _|\r\n");
  printf(BW_COM2,
         " /"
         "-OO----OO\"\"=\"OO--OO\"=\"OO--------OO\"=\"OO-------OO\"=\"OO-"
         "------OO\"=P\r\n");
  printf(BW_COM2,
         "###############################################################"
         "######\r\n");
  printf(BW_COM2,
         "                                                               "
         "      \r\n");
  printf(BW_COM2, "Cause: %s\r\n", str);
  printf(BW_COM2,
         "                                                               "
         "      \r\n");
  printf(BW_COM2,
         "###############################################################"
         "######\r\n");
}
