/* ************************************************************************
*   File: alias.c				   A utility to CircleMUD *
*  Usage: writing/reading player's aliases                                *
*                                                                         *
*  Code done by Jeremy Hess and Chad Thompson				  *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "structs.h"
#include "class.h"
#include "utils.h"
#include "interpreter.h"

void            write_aliases(struct char_data * ch)
{
    FILE           *file;
    char            fn[127],
    *buf = NULL;
    struct alias   *temp;
    int             length;

    get_filename(GET_NAME(ch), fn, ALIAS_FILE);
    //    unlink(fn);
    if (!GET_ALIASES(ch))
        return;

    file = fopen(fn, "wt");
    if (file == 0) {
        sprintf(buf, "SYSERR: Can't create alias file: %s!", fn);
        log(buf);
        return;
    }
    temp = GET_ALIASES(ch);

    while (temp) {
        length = strlen(temp->alias);
        length = MIN(length, 120);
        *(temp->alias + length) = 0;
        fprintf(file, "%d\n", length);
        fprintf(file, "%s\n", temp->alias);

        length = strlen(temp->replacement);
        length = MIN(length, 120);
        *(temp->replacement + length) = 0;

        buf = strdup(temp->replacement);
        while (*buf == ' ') {
            length--;
            buf++;
        }

        fprintf(file, "%d\n", length);
        fprintf(file, "%s\n", buf);
        fprintf(file, "%d\n", temp->type);
        temp = temp->next;
    }

    fclose(file);
}

void            read_aliases(struct char_data * ch)
{
    FILE           *file;
    char            fn[127];
    struct alias   *t2;
    int             length;
    char            temp_buf[127],
    buf[127];

    get_filename(GET_NAME(ch), fn, ALIAS_FILE);

    file = fopen(fn, "rt");

    if (!file)
        return;

    CREATE(GET_ALIASES(ch), struct alias, 1);
    t2 = GET_ALIASES(ch);
    do {
        fscanf(file, "%d\n", &length);
        fgets(buf, length + 1, file);
        t2->alias = strdup(buf);
        fscanf(file, "%d\n", &length);
        fgets(buf, length + 1, file);
        strcpy(temp_buf, " ");
        strcat(temp_buf, buf);
        t2->replacement = strdup(temp_buf);
        fscanf(file, "%d\n", &length);
        t2->type = length;
        if (!feof(file)) {
            CREATE(t2->next, struct alias, 1);
            t2 = t2->next;
        }
    } while (!feof(file));

    fclose(file);
}
