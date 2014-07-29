/*#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/telnet.h>
#include <netinet/in.h>*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/telnet.h>
#include <netinet/in.h>

#include "structs.h"
#include "class.h"
#include "rooms.h"
#include "objs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "clan.h"


void            logs(const char *format,...)
{
    va_list         args;
    time_t          ct = time(0);
    char           *time_s = asctime(localtime(&ct));

    *(time_s + strlen(time_s) - 1) = '\0';

    fprintf(stderr, "%-15.15s :: ", time_s+4);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\r\n");
}

void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char(buf, ch);
}
