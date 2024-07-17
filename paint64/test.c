#include <stdio.h>

#define ANSI_FONT_COL_RESET     "\x1b[0m"
#define FONT_COL_CUSTOM_RED     "\e[38;2;240;0;240m" // where rrr;ggg;bbb in 38;2;rrr;ggg;bbbm can go from 0 to 255 respectively
#define FONT_COL_CUSTOM_GREEN   "\e[38;2;0;200;0m" // where rrr;ggg;bbb in 38;2;rrr;ggg;bbbm can go from 0 to 255 respectively
#define FONT_COL_CUSTOM_BLUE    "\e[38;2;0;0;200m" // where rrr;ggg;bbb in 38;2;rrr;ggg;bbbm can go from 0 to 255 respectively
#define BCKGRD_COL_CUSTOM_RED   "\e[48;2;200;0;0m" // where rrr;ggg;bbb in 48;2;rrr;ggg;bbbm can go from 0 to 255 respectively
#define BCKGRD_COL_CUSTOM_GREEN "\e[48;2;0;200;0m" // where rrr;ggg;bbb in 48;2;rrr;ggg;bbbm can go from 0 to 255 respectively
#define BCKGRD_COL_CUSTOM_BLUE  "\e[48;2;0;0;200m" // where rrr;ggg;bbb in 48;2;rrr;ggg;bbbm can go from 0 to 255 respectively

int main (int argc, char const *argv[]) {

  printf(FONT_COL_CUSTOM_RED     "This font color is CUSTOM_RED!"           ANSI_FONT_COL_RESET "\n");
  printf(FONT_COL_CUSTOM_GREEN   "This font color is CUSTOM_GREEN!"         ANSI_FONT_COL_RESET "\n");
  printf(FONT_COL_CUSTOM_BLUE    "This font color is CUSTOM_BLUE!"          ANSI_FONT_COL_RESET "\n");
  printf(BCKGRD_COL_CUSTOM_RED   "This background color is CUSTOM_RED!"     ANSI_FONT_COL_RESET "\n");
  printf(BCKGRD_COL_CUSTOM_GREEN "This background color is CUSTOM_GREEN!"   ANSI_FONT_COL_RESET "\n");
  printf(BCKGRD_COL_CUSTOM_BLUE  "This background color is CUSTOM_BLUE!"    ANSI_FONT_COL_RESET "\n");
  printf(FONT_COL_CUSTOM_GREEN BCKGRD_COL_CUSTOM_RED "This font color is CUSTOM_GREEN with background CUSTOM_RED!"    ANSI_FONT_COL_RESET "\n");
  printf(                        "This font color is NORMAL!\n");

  return 0;
}