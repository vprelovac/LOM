
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "akey.hpp"
#ifdef DMALLOC
# include "dmalloc.h"
#endif
//unsigned long   my_rand();

char           *akey::getrndreply()
{
    char           *defaultmsg = "Mmm, sounds interesting...";

    if (totalwates == 0)
        return defaultmsg;

    int             rndnum = rand()%totalwates,
                             total = 0;

#ifdef DEBUG
    printf("grr: rndnum=%d totalwates=%d\n", rndnum, totalwates);
#endif

    for (int i = 0; i < numreplys; i++) {
#ifdef DEBUG
        printf("i=%d total=%d wate=%d\n", i, total, replys[i].wate);
#endif

        if ((total += (replys[i].wate)) > rndnum) {
#ifdef DEBUG
            puts("grr f");
#endif

#ifdef REDUCEWATES
            if (replys[i].wate > 1) {
                totalwates--;
                replys[i].wate--;
            }
#endif
            return replys[i].sent;
        }
    }
#ifdef DEBUG
    puts("getrndreply should have found a string");
#endif
    return defaultmsg;
}

akey::akey()
{
    logic = NULL;
    replys = NULL;
    numreplys = totalwates = 0;
#ifdef TEST
    /*    // puts("constructor akey()");*/
#endif
}

int             akey::addlogic(char *logicstr)
{
    if (logic != NULL)
        DISPOSE(logic);
    return (logic = strdup(logicstr)) != NULL;
}

int             akey::addreply(int w, char *r)
{
    reply          *t;
#ifdef CHECKMEM
    static int      called = 0;

    called++;
#endif
    numreplys++;

    if ((t = (reply *) realloc(replys, numreplys * sizeof(reply))) == NULL) {
#ifdef CHECKMEM
        printf("realloc error in addreply in call %d\n", called);
#endif
        return 0;
    }
    replys = t;
    totalwates += w;
    replys[numreplys - 1].wate = w;

    if ((replys[numreplys - 1].sent = strdup(r)) == NULL) {
#ifdef CHECKMEM
        puts("out of mem for strdup in addreply");
#endif
        return 0;
    }
#ifdef DEBUG
    printf("reply added:%s", replys[numreplys].sent);
#endif
    return 1;
}

akey::~akey()
{
    if (logic != NULL)
        DISPOSE(logic);

    if (replys != NULL) {
        for (int i = 0; i < numreplys; i++)
            DISPOSE(replys[i].sent);

        DISPOSE(replys);
    }
}
