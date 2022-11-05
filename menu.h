#ifndef ROMSELECT
#define ROMSELECT
#define ROMINFOFILE "/currentloadedrom.txt"
void RomSelect_SetLineBuffer(WORD *p, WORD size);
void menu(uintptr_t NES_FILE_ADDR, char *errorMessage, bool isFatalError);

#endif