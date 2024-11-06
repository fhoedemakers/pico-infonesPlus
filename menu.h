#ifndef ROMSELECT
#define ROMSELECT
#define ROMINFOFILE "/currentloadedrom.txt"
#define SWVERSION "VX.X"
void RomSelect_SetLineBuffer(WORD *p, WORD size);
void menu(uintptr_t NES_FILE_ADDR, char *errorMessage, bool isFatalError, bool showSplash);
char getcharslicefrom8x8font(char c, int rowInChar);
#endif