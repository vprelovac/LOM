
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "akey.hpp"
#include "allkeys.hpp"
#include "eliza.hpp"
#include "chatmain.hpp"

const char      eliza_title[] = "chat by Christopher Busch  Copyright (c)1993";
const char      eliza_version[] = "version 1.0.0";

eliza          *chatter;

extern          "C" void startchat(char *filename)
{
    chatter = new eliza;
    if (chatter == NULL) {
        fprintf(stderr, "Chat not enough memory");
        exit(1);
    }
    chatter->reducespaces("");
    chatter->loaddata(filename);
}

extern          "C" char *dochat(char *talker, char *msg, char *target)
{
    return chatter->process(talker, msg, target);
}

extern          "C" void endchat(void)
{
    delete          chatter;
}
