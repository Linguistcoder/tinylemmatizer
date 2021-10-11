// tinylemmatizer.h : Include file for standard system include files,
// or project specific include files.

extern void startProc(const char* flexrulesFileName, int* err);
extern void stringEval(const char* s, const char** out, int* err);
extern void endProc(void);
