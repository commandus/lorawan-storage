#if defined(_MSC_VER) || defined(__MINGW32__)
#include <ctime>

char * strptime(const char *buf, const char *format, struct tm *timeptr);
#endif
