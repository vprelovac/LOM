
/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __CONFIG_C__

#include "structs.h"

#define TRUE	1
#define YES	1
#define FALSE	0
#define NO	0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */
int             pk_allowed = YES;

/* is playerthieving allowed? */
int             pt_allowed = YES;

/* minimum level a player must be to shout/holler/gossip/auction */
int             level_can_shout = 1;

/* number of movement points it costs to holler */
int             holler_move_cost = 15;

/* exp change limits */
int             max_exp_gain = 10000;           /* max gainable per kill */
int             max_exp_loss = 10000000;        /* max losable per death */

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int             max_npc_corpse_time = 5;
int             max_pc_corpse_time = 15;
int             max_obj_time = 300;

/* should items in death traps automatically be junked? */
int             dts_are_dumps = NO;

/* "okay" etc. */
char           *OK = "Ok.\r\n";
char           *NOPERSON = "No-one by that name here.\r\n";
char           *NOEFFECT = "Nothing seems to happen.\r\n";

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.
 */
int             free_rent = YES;

/* maximum number of items players are allowed to rent */
int             max_obj_save = 100;

/* receptionist's surcharge on top of item costs */
int             min_rent_cost = 100;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.
 */
int             auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int             autosave_time = 10;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int             crash_file_timeout = 365;

/* Lifetime of normal rent files in days */
int             rent_file_timeout = 365;


/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */
sh_int          mortal_start_room = 3001;

/* virtual number of room that immorts should enter at by default */
sh_int          immort_start_room = 1204;

/* virtual number of room that frozen players should enter at */
sh_int          frozen_start_room = 1202;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.obj1.c if you change the number of non-NOWHERE
 * donation rooms.
 */
sh_int          donation_room_1 = 3063;
sh_int          donation_room_2 = NOWHERE;      /* unused - room for
                                                 * expansion */
sh_int          donation_room_3 = NOWHERE;      /* unused - room for
                                                 * expansion */


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/* default port the game should run on if no port given on command-line */
int             DFLT_PORT = 4000;

/* default directory to use as data directory */
char           *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
int             MAX_PLAYERS = 500;

/* maximum size of bug, typo and idea files (to prevent bombing) */
int             max_filesize = 50000;

/* maximum number of password attempts before disconnection */
int             max_bad_pws = 3;

/* maximum number of new objects allowed by oeditor */
int             max_new_objects = 50;  /* oeditmod */

/* maximum number of new rooms allowed by reditor */
int             max_new_rooms = 50;    /* reditmod */

/* maximum number of new zones allowed by zeditor */
int             max_new_zones = 1;      /* zeditmod */

/* maximum number of new zone commands allowed by zeditor */
int             max_new_zcmds = 50;      /* zeditmod */

/* maximum number of new mobs allowed by meditor */
int             max_new_mobs = 50;     /* meditmod */

/*
 * Some nameservers are very slow and cause the game to lag terribly every
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

int             nameserver_is_slow = YES;

char           *MENU2 =
    "\r\r\n\r\n\r\n\r\n\r\n\n"
    " Welcome   _________             @               @    \r\n"
    "   To    < Lands of |         {|||||}         {|||||} \r\n"
    "         <   Myst   |       ***********      *********  \r\n"
    " __^__    ''''''''''|      /| | | | | |\\----/| | | | |\\                  __^__ \r\n"
    "( ___ )-------------------/ | | | | | | MENU | | | | | \\----------------( ___ )\r\n"
    " | / ||__|__|__|__|__|__|_|==------------==----------==|__|__|__|__|__|__| / |\r\n"
    " | / |_|_IMPLEMENTOR: |__||   0) Exit from Game.       |_|__|__|__|__|___| / |\r\n"
    " | / |__|__|Bog__|__|__|__|   1) Enter the Game.       |___|__|__|__|__|_| / |\r\n"
    " | / ||__|__|__|__|__|__|_|   2) Enter Description.    |__|__|__|__|__|__| / |\r\n"
    " | / |__|__|__|__|__|__|__|   3) Background Story.     |_|__|__|__|__|__|| / |\r\n"
    " | / ||__|__|__|__|__|__|_|   4) Change Password.      |__|__|__|__|__|__| / |\r\n"
    " | / |__|__|__|__|__|__|__|   5) Delete Character.     |__|__|__|__|__|__| / |\r\n"
    " | / |__|__|__|__|___|__|_|                            |__|__|__|__|__|__| / |\r\n"
    " | / ||__|___|_|_|_|__|___|==-----------==-----------==|_|__|__|__|__|__|| / |\r\n"
    " | / |__|__|__|__|__|__|__|   *) Advance on Path.      |__|__|__|__|__|__| / |\r\n"
    "(_____)-----------------------------------------------------------------(_____)\r\n"
    "\r\r\n\n                              Choice: ";

char *boss_screen=
    "total 4536\r\n"
    "-rw-rw-rw-    1 vpopovic user           5 Apr  5 22:05 .146.adf\r\n"
    "-rw-rw-rw-    1 vpopovic user           5 Apr  5 22:05 .146.adf\r\n"
    "drwxrwxrwt    6 sys      sys         4096 Apr  5 22:07 ./\r\n"
    "drwxr-xr-x   24 root     sys         4096 Apr  5 19:58 ../\r\n"
    "-rw-rw-rw-    1 pedjoni  user           5 Apr  5 21:53 .10.48c4a7\r\n"
    "-rw-rw-rw-    1 cegevara user           5 Apr  5 22:06 .10.542ac5\r\n"
    "-rw-rw-rw-    1 sylvia   user           5 Apr  5 17:21 .10.5648fe\r\n"
    "-rw-rw-rw-    1 vpopovic user           5 Apr  5 22:05 .146.adf6d6\r\n"
    "-rw-rw-rw-    1 vpopovic user           5 Apr  5 22:05 .146.adf4fd\r\n"
    "-rw-------    1 root     sys        16384 Apr  5 21:14 .cadminDSSharedArena\r\n"
    "-rw-------    1 root     sys        16384 Apr  5 12:10 .cadminOSSharedArena\r\n"
    "drwxrwxrwt    2 root     sys           24 Apr  5 12:10 .font-unix/\r\n"
    "drwxrwxr-x    2 root     sys           26 Apr  5 12:33 .ps_data/\r\n"
    "-rw-rw-rw-    1 borojev  user        1125 Apr  5 21:51 .tin_log\r\n"
    "-rw-rw-rw-    1 borojev  user        1125 Apr  5 21:51 .mail_rc\r\n"
    "-rw-rw-rw-    1 borojev  user        1125 Apr  5 21:51 .cshrc\r\n"
    "-rw-------    1 vstancic user       55748 Apr  5 14:45 L185490.html\r\n"
    "-rw-------    1 vstancic user         348 Apr  5 15:09 L185491.html\r\n"
    "-rw-------    1 vstancic user           0 Apr  5 15:10 L1854910.html\r\n"
    "-rw-------    1 vstancic user       78411 Apr  5 14:48 L185493.html\r\n"
    "-rw-------    1 vstancic user      116351 Apr  5 14:58 L185496.html\r\n"
    "-rw-------    1 vstancic user       72150 Apr  5 15:00 L185497.html\r\n"
    "-rw-------    1 vstancic user      203021 Apr  5 15:06 L185498.html\r\n"
    "-rw-------    1 vstancic user       30252 Apr  5 15:09 L185499.html\r\n"
    "-rw-------    1 nobody   sys            6 Apr  5 12:10 ircd.pid\r\n";
                     
char *MENU=
"\r\n\r\n"
"                   &Y-=-=-=-=-=-=-=-=-=-=-=-=-\r\n"
"                   &GWelcome to Lands of Myst!\r\n"
"                   &Y-=-=-=-=-=-=-=-=-=-=-=-=-\r\n"
"\r\n"
"\r\n"
"\r\n"
"   &G1&0)&C Enter Lands of Myst                &G4&0)&c Change password\r\n"
"   &G2&0)&C Enter LOM as NPC! (still in beta)  &G5&0)&c Delete character\r\n"
"   &G3&0)&c Read the background story          &G6&0)&c Describe character\r\n"
"   &G0&0)&c Leave LOM                          &G7&0)&c Remort character\r\n"
"\r\n"
"\r\n\r\n"
"                      &GMake your choice:&0 ";

                     
                     
char           *MENU3 =
    "\r\r\n\r\n\r\n\r\n\n"
    "        ____________________________ \r\n"
    "       ()                           )\r\n"
    "        |         &0Main Menu&0         |\r\n"
    "        |                           |\r\n"
    "        |    &G0&0) &cLeave the lands.&0    |\r\n"
    "        |    &G1&0) &cEnter the realm.&0    |\r\n"
    "        |    &G2&0) &cChange description.&0 |\r\n"
    "        |    &G3&0) &cBackground story.&0   |\r\n"
    "        |    &G4&0) &cChange password.&0    |\r\n"
    "        |    &G5&0) &cDelete character.&0   |\r\n"
    "        |                           |\r\n"
    "        |                       /;  /\r\n"
    "         \\  /^\\/^\\/' \\ /\\/^\\/^\\/ \\/^ \r\n"
    "          ;/          \\;\r\r\n\r\n\n"
    "                &cChoice:&0 ";
char           *MENU1 =
    "\r\r\n\r\n\r\n\r\n\r\n\r\n\n"
    "		/====================================\\\r\n"
    "		|    WELCOME TO THE LANDS OF MYST    |\r\n"
    "		|   =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-   |\r\n"
    "		|                                    |\r\n"
    "		|   (1) Enter the game.              |\r\n"
    "		|   (2) Enter description.           |\r\n"
    "		|   (3) Read the background story.   |\r\n"
    "		|   (4) Change password.             |\r\n"
    "		|   (5) Delete this character.       |\r\n"
    "               |				     |\r\n"
    "		|   (0) Exit.                        |\r\n"
    "               |   ------------------------------   |\r\n"
    "               |   (*) Advance on the Golden Path   |\r\n"
    "               |   ------------------------------   |\r\n"
    "       	\\====================================/\r\n"
    "\r\n"
    "                        Make your choice: ";
char           *greet1 =
    "                 ||_||_||_||         ||_||_||_||\r\n"
    "                  \\ _________ /           \\ _________ /    \r\n"
    "      \\-,_         |__|__|__||             ||__|__|__|        _,-/ \r\n"
    "   ,  \\ \\  ~-,     |_|__|__|_|_||_||_||_|_|__|__|_|     ,-~  /   , \r\n"
    "  _)\\   |    `\\    |__|__|__|               |__|__|__|   /'    |   /(_ \r\n"
    " /,  \\_,/  \\   `\\  |_|__|__|  LANDS OF MYST  |__|__|_| /'   /  \\,_/  .\\r\r\n"
    " `-,  ,\\    |    \\  \\ |__|                     |__| / /    |    /,  ,-'\r\n"
    " |G,/ / ,-, |     |  |__|Based on CircleMUD 3.0,|__| |     | ,-, \\ \\_3|\r\n"
    " _\\_,/_  \\ \\|-,   |  ||_ Created by Jeremy Elson _|| |   ,-|/ / ,_\\,_/_\r\n"
    "(_  ,__`  |    `\\ |  |__|                       |__| | /'    |   __,  _)\r\n"
    "  ~--._~-;'      \\|  ||_ Email all questions and _|| |/      `;-~_.--~\r\n"
    "     __/ /        |  |__|requests for new players__| |         \\ \\__  \r\n"
    "   '',-,'      ,-,   ||_tomcat@galeb.etf.bg.ac.yu_||   ,-,      `,-,``\r\n"
    "    ((|))     ((|))  |__|__|__|__|__|__|__|__|__|__|  ((|))     ((|)) \r\n"
    "     |||,-'~`-,|||_,-||__|__|__|__|__|__|__|__|__|_|-,_|||,-'~`-,|||  \r\n"
    "     |||,-'~'-,||| ,-|__|__|__|__|__|__|__|__|__|__|-, |||,-'~`-,|||  \r\n"
    "     |||       |||/  ||__|__|__|__|__|__|__|__|__|_|  \\|||       |||  \r\n"
    "     |||       ||| _/|__|__|__|__|__|__|__|__|__|__|\\_ |||       |||  \r\n"
    "     |||       ||||  May your travels be interesting  ||||       |||  \r\n"
    "    //|\\\\     //|\\|                                   |/|\\\\     //|\\\\ \r\n"
    "\r\n"
    "By what name you wish to be known: ";

char           *GREETINGS1 =
    "\r\n"
    "                           )   There is only one:  (\r\n"
    "                          /|\\       (     )       /|\\\r\n"
    "                         / | \\      \\\\_|_//      / | \\\r\n"
    "*                       /  |  \\     (/\\|/\\)     /  |  \\                      *\r\n"
    "|`.______________________________o___\\`|'/___o_____________________________.'|\r\n"
    "|                               '^`   \\|/   '^`                              |\r\n"
    "| W        WW   W    W WWWWW           V          W    W W   W  WWWW  WWWWW  |\r\n"
    "| W       W  W  WW   W W    W      WWWW  WWWW     WW  WW  W W  W        W    |\r\n"
    "| W      W    W W W  W W    W      W  W  W        W WW W   W    WWWW    W    |\r\n"
    "| W      WWWWWW W  W W W    W      W  W  WW       W    W   W        W   W    |\r\n"
    "| W      W    W W   WW W    W      WWWW  W        W    W   W   W    W   W    |\r\n"
    "| WWWWWW W    W W    W WWWWW         ,_.          W    W   W    WWWW    W    |\r\n"
    "|                                   /,-.\\                                    |\r\n"
    "| .________________________________//___\\\\_________________________________. |\r\n"
    "|'                    \\        /  ((     \\\\_// \\        /                   `|\r\n"
    "*   Implementor: Bog   \\/\\/\\/\\/    \\\\     `-'   \\/\\/\\/\\/                     *\r\n"
    "    Based on Circle 3.0            //\r\n"
    "                                   V\r\n"
    "\r\n"
    "\r\r\n\n"
    "What is thy name: ";

char           *GREETINGS_old =
    "\r\n\r\n"
    "              (O)\r\n"
    "              <M    LANDS OF MYST:\r\n"
    "   o          <M              The Saga Continues...\r\n"
    "  /| ......  /:M\\------------------------------------------------,,,,,,\r\n"
    "(0)[]XXXXXX[]I:K+}=====<{R}>================================------------>\r\n"
    "  \\| ''''''  \\:W/------------------------------------------------''''''\r\n"
    "   o          <W\r\n"
    "              <W    Based on Circle 3.00, Jeremy Elson\r\n"
    "              (O)       A derivative of DikuMUD (GAMMA 0.0)\r\n"
    "\r\n\r\n\r\n\r\n\r\n\r\n";

char *GREETINGS_tfe =
"\r\n\r\n"          
"-------------------------------------------------------------------------------\r\n"
"                             Thanks to\r\n"
"     Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.\r\n"
"    For many ideas and starting us down this twisting road of imagination.\r\n"
"-------------------------------------------------------------------------------\r\n"
"\r\n"                                                        
"      _                                       __     __  __                 \r\n"
"     / |                    _                / _|   |  \\/  |           _    \r\n"
"     | |     __ _ _ __   __| | ___      ___ | |_    | \\  / |_   _  ___| |_  \r\n"
"     | |    / _` | '_ \\ / _` |/ __|    / _ \\|  _|   | |\\/| | | | |/ __| __| \r\n"
"     | |___| (_| | | | | (_| |\\__ \\   | (_) | |     | |  | | |_| |\\__ \\ |_  \r\n"
"     \\_____|\\__,_|_| |_|\\__,_||___/    \\___/|_|     |_|  |_|\\__, ||___/\\__| \r\n"
"                                                             __/ |          \r\n"
"                                                            |___/           \r\n"
"                                                                            \r\n" 
"                          [1] Start a new character                         \r\n"        
"                          [2] See feature list                              \r\n"
"                          [3] Read policies                                 \r\n"
//"                          [4] See who is online                             \r\n"
"                                                                            \r\n"
"           --* Or enter the name of an already existing character. *--      \r\n"    
"                                                                            \r\n"    
"                        Choice: ";



char *GREETINGS=
"\r\n\r\n\r\n\x1B[1;32m"
"      _                                       __     __  __                 \r\n"
"     / |                    _                / _|   |  \\/  |           _    \r\n"
"     | |     __ _ _ __   __| | ___      ___ | |_    | \\  / |_   _  ___| |_  \r\n"
"     | |    / _` | '_ \\ / _` |/ __|    / _ \\|  _|   | |\\/| | | | |/ __| __| \r\n"
"     | |___| (_| | | | | (_| |\\__ \\   | (_) | |     | |  | | |_| |\\__ \\ |_  \r\n"
"     \\_____|\\__,_|_| |_|\\__,_||___/    \\___/|_|     |_|  |_|\\__, ||___/\\__| \r\n"
"                                                             __/ |          \r\n"
"                                                            |___/           \r\n"
"                                                                            \r\n\r\n"
"\x1B[0;35m                    --* \x1B[0;33m   148.251.247.173 4000   \x1B[0;35m*--\r\n"
"\x1B[0;35m                    --* \x1B[0;33mLOM engine v3.2, since 1996 \x1B[0;35m*--\r\n"
"\x1B[0;0m\r\n\r\n\r\n";

char *GREETINGS_nocolor=
"\r\n\r\n\r\n\r\n"
"      _                                       __     __  __                 \r\n"
"     / |                    _                / _|   |  \\/  |           _    \r\n"
"     | |     __ _ _ __   __| | ___      ___ | |_    | \\  / |_   _  ___| |_  \r\n"
"     | |    / _` | '_ \\ / _` |/ __|    / _ \\|  _|   | |\\/| | | | |/ __| __| \r\n"
"     | |___| (_| | | | | (_| |\\__ \\   | (_) | |     | |  | | |_| |\\__ \\ |_  \r\n"
"     \\_____|\\__,_|_| |_|\\__,_||___/    \\___/|_|     |_|  |_|\\__, ||___/\\__| \r\n"
"                                                             __/ |          \r\n"
"                                                            |___/           \r\n"
"                                                                            \r\n"
"                         --* 147.91.1.129 5000 *--\r\n"
"                    --* LOM engine v3.1, since 1996 *--\r\n"
"\r\n\r\n\r\n";



                       
char *GREETINGS_older=
    "\r\n"
    "                           )                    :  (\r\n"
    "                          /|\\       (     )       /|\\\r\n"
    "                         / | \\      \\\\_|_//      / | \\\r\n"
    "*                       /  |  \\     (/\\|/\\)     /  |  \\                      *\r\n"
    "|`.______________________________o___\\`|'/___o_____________________________.'|\r\n"
    "|                               '^`   \\|/   '^`                              |\r\n"
    "| W        WW   W    W WWWWW           V          W    W W   W  WWWW  WWWWW  |\r\n"
    "| W       W  W  WW   W W    W      WWWW  WWWW     WW  WW  W W  W        W    |\r\n"
    "| W      W    W W W  W W    W      W  W  W        W WW W   W    WWWW    W    |\r\n"
    "| W      WWWWWW W  W W W    W      W  W  WW       W    W   W        W   W    |\r\n"
    "| W      W    W W   WW W    W      WWWW  W        W    W   W   W    W   W    |\r\n"
    "| WWWWWW W    W W    W WWWWW         ,_.          W    W   W    WWWW    W    |\r\n"
    "|                                   /,-.\\                                    |\r\n"
    "| .________________________________//___\\\\_________________________________. |\r\n"
    "|'                    \\        /  ((     \\\\_// \\        /                   `|\r\n"
    "*                      \\/\\/\\/\\/    \\\\     `-'   \\/\\/\\/\\/                     *\r\n"
    "                                   //\r\n"
    "                                   V\r\n"
    "\r\n";

char           *GREETINGS_3=
    "\r\n\r\n\r\n"
    "              *****************************************\r\n"
    "              *          LOM: The Third Age           *\r\n"
    "              *            - BLOOD WARS -             *\r\n"
    "              *                                       *\r\n"
    "              *     Multi User Adventuring Domain     *\r\n"
    "              *                                       *\r\n"
    "              *                                       *\r\n"
    "              *  A derivative of DikuMUD (GAMMA 0.0)  *\r\n"
    "              *****************************************\r\n"
    "\r\n\r\n\r\n\r\n\r\n"
    "                      Welcome to the Homeland.\r\n"
    "\r\n\r\n\r\n";
char           *GREETINGS_1 =
    "\r\n\r\n"
    "              (O)\r\n"
    "              <M    BLOOD WARS:\r\n"
    "   o          <M              The First Age...\r\n"
    "  /| ......  /:M\\------------------------------------------------,,,,,,\r\n"
    "(0)[]XXXXXX[]I:K+}=====<{R}>================================------------>\r\n"
    "  \\| ''''''  \\:W/------------------------------------------------''''''\r\n"
    "   o          <W\r\n"
    "              <W\r\n"
    "              (O)\r\n"
    "\r\n\r\n\r\n\r\n\r\n\r\n";

char           *GREETINGS_old1 =
    "\r\n\r\n"
    "  Welcome to the ETF MUD SERVER!\r\n"
    " ================================\r\n"
    " located at lav.etf.bg.ac.yu 4000\r\n"
    " running LOM II mud engine v2.2.0\r\n"
    "\r\n\r\n"
    "   THIS IS ONLY A TEST VERSION!\r\n"
    " AFTER THE TESTING PERIOD EXPIRES\r\n"
    " PLAYER FILE WILL BE DELETED. \r\n"
    " USE 'BUG' AND 'IDEA' COMMANDS\r\n"
    " TO POINT OUT ANYTHING THAT CAN\r\n"
    " IMPROVE THE FINAL VERSION.\r\n\r\n"
    "\r\n\r\n";

//"By what name are you known? ";


char           *WELC_MESSG =
    "\r\n\r\n"
    "May your visit here be... Interesting."
    "\r\n\r\n";

char           *DEATH_MESSG =
    "\r\r\n\r\n\r\n\r\n\r\n\n"
    "                     ,_______________________________\r\n"
    "                    /                                \\\r\n"
    "                   /                                  \\\r\n"
    "                  /.             _ \\   _ _|    _ \\     \\\r\n"
    "                 /..            |   |    |    |   |     \\\r\n"
    "                |:..            __ <     |    ___/       |\r\n"
    "                |:..           _| \\_\\_)___|_)_|  _)      |\r\n"
    "                |:..                                     |\r\n"
    "                |:.%12s %-25s|\r\n"
    "                |:..                                     |\r\n"
    "                |:..                                     |\r\n"
    "                |:..                                     |\r\n"
    " \\ | /          |:..   \\ | /                             |       \\ | /\r\n"
    "__\\|/__________/|_______\\|/______________________________|\\_______\\|/______\r\n"
    "\r\r\n\r\n\r\n\n";

char           *START_MESSG =
    "\r\nWelcome to the Lands of Myst, and may your stay be enjoyable!\r\r\n\n";
char           *hum =
    "HUMANS\r\n"
    "\r\n"
    "Humans are the general race in the land, with no major penalties or\r\n"
    "benefits.  Humans can join any profession, and stand as the measuring\r\n"
    "base for the other races.\r\n"
    "\r\n"
    "Allowable Professions:  All\r\n"
    "Racial Abilites:        None\r\n"
    "Other Bonuses:          None\r\n";

char           *dwa =
    "DWARVES\r\n"
    "\r\n"
    "Dwarves are short and stout creatures, who prefer to be with their own.\r\n"
    "Used to living underground they can see quite well in the dark, and\r\n"
    "have healthy constitutions to help them survive in the harshest conditions.\r\n"
    "\r\n"
    "Allowable Professions:  Warrior, Cleric, Mage, Thief\r\n"
    "Racial Abilites:        Infravision, Detect Hidden\r\n"
    "Other Bonuses:          + con wis\r\n"
    "                        - dex cha\r\n";

char           *ogr =
    "OGRES\r\n"
    "\r\n"
    "Ogres, often thought of as large mindless brutes, willing to follow any\r\n"
    "leader anywhere for food, treasure, and a good fight, are the strongest\r\n"
    "of all races, and rightly so.  The main drawback of ogres is their innate\r\n"
    "easily hold their own against most anyone else.\r\n"
    "\r\n"
    "Allowable Professions:  Warrior, Cleric, Thief\r\n"
    "Racial Abilites:        None\r\n"
    "Other Bonuses:          + str con\r\n"
    "                        - int wis dex\r\n";

char           *gno =
    "GNOMES\r\n"
    "\r\n"
    "Gnomes, like dwarves enjoy being underground, but whereas Dwarves like to\r\n"
    "mine the deepearth for its minerals and jewels, Gnomes prefer to shape the\r\n"
    "gnomes like their jewels and minerals just as much as dwarves do.  Because\r\n"
    "of their small size, they are not as robust as most, but as a race they are\r\n"
    "much more agile than any other, including elves.\r\n"
    "\r\n"
    "Allowable Professions:  Thief, Mage, Cleric, Warrior\r\n"
    "Racial Abilites:        Infravision\r\n"
    "Other Bonuses:          + dex\r\n"
    "                        - con str\r\n";

char           *elf =
    "ELVES\r\n"
    "\r\n"
    "Elves are amongst the lithest of the races, and are often found in forests,\r\n"
    "where they seem to have close rapports with the animals which live in harmony\r\n"
    "with them.  Elves tend to place an emphasis on learning, and as such, their\r\n"
    "bodies are not as strong and robust as humans, but they are often much more\r\n"
    "intelligent and because of their great age, much more wise and patient.\r\n"
    "\r\n"
    "Allowable Professions:  Mage, Cleric, Druid, Thief, Warrior\r\n"
    "Racial Abilites:        Infravision, Scan +1\r\n"
    "Other Bonuses:          + int wis\r\n"
    "                        - str con\r\n";

char           *helf =
    "GOBLIN\r\n"
    "\r\n"
    "Halfelves are crossbreeds of humans and elves.  They have all the physical\r\n"
    "attributes of elves and have the bodily strength normally associated with\r\n"
    "humans.  Their stature is equivalent with that of humans.  Unfortunately,\r\n"
    "having elven characteristics, certain races may impart their hatred for elves\r\n"
    "upon halfelves.\r\n"
    "\r\n"
    "Allowable Professions:    Monk, Mage, Thief, Cleric, Warrior\r\n"
    "Racial Abilities:         Infravision\r\n"
    "Other Bonuses:            None\r\n";

char           *hob =
    "HALFLING\r\n"
    "\r\n"
    "Halflings are small funloving creatures with a strong sense of family.\r\n"
    "Halflings have small frames and are quite agile.  Because of their size, they\r\n"
    "gain certain advantages when fighting much larger foes but they are also picked\r\n"
    "upon by these large gruesome beasts too.\r\n"
    "\r\n"
    "Allowable Professions:    Thief, Warrior, Mage\r\n"
    "Racial Abilities:         Infravision, Detect Hidden\r\n"
    "Other Bonuses:            + dex\r\n"
    "                          - str\r\n";

char           *dro =
    "DROW\r\n"
    "\r\n"
    "Drows are evil elves that live underground mostly. That kind of hostile\r\n"
    "environment gave them better dexterity and higher intelligence. However,\r\n"
    "they aren't so popular among the surface dwelling races. Hence to their evil\r\n"
    "nature they are to be played as a deceptive characters.\r\n"
    "\r\n"
    "Allowable Professions:   Mage, Cleric, Thief, Warrior\r\n"
    "Racial Abilities:        Infravision, Detect hidden\r\n"
    "Other Bonuses:           + dex int\r\n"
    "                         - con cha\r\n";


char           *namepol =
    "\r\nUnacceptable character names will be deleted. Try to\r\n"
    "choose a medieval, fantasy type of name. If you are in\r\n"
    "doubt, type 'no' at the following prompt to get a few\r\n"
    "useful suggestions for a name.\r\n"
    "\r\nAre you sure you have chosen a proper name? ";

char *goodnames=
    "\r\nFew suggestions for you:\r\n"
    "Abida		Abimaer		Achan		Adam		Adar\r\n"
    "Adbeel		Adelard		Aegeleb		Aiah		Akan\r\n"
    "Aldor		Almodad		Alvan		Amalek		Amorites\r\n"
    "Amroth		Anah		Anamites	Anarion		Anborn\r\n"
    "Ancalogon	Angbor		Angelica	Angmar		Aphorties\r\n"
    "Appledore	Aragorn		Aram		Arathorn	Araw\r\n"
    "Arise		Arkites		Arod		Arphaxad	Arvad\r\n"
    "Arvedui		Arwen		Asfaloth	Ashkenaz	Ashur\r\n"
    "Athelas		Avith		Badillo		Baggins		Bain\r\n"
    "Bairt		Bal             Bal-Hanan	Baldor		Balin\r\n"
    "Barog		Bandobras	Bane		Banks		Barahir\r\n"
    "Baranor		Bard		Barliman	Bedad		Bedadar\r\n"
    "Beech		Beechbone	Bela		Beor		Beorn\r\n"
    "Beornings	Beren		Bergil		Bergond		Beruthiel\r\n"
    "Bifur		Bilbo		Bilhan		Bill		Birch\r\n"
    "Blafungel	Boffin		Bofur		Bolger		Bombad\r\n"
    "Bombadil	Bombur		Bome		Boromir		Bounders\r\n"
    "Bowman		Bozrah		Bracegirdle	Brand		Bree\r\n"
    "Breeker		Bregalad	Brego		Buchland	Butterbur\r\n"
    "Calcol		Caleb		Canaan		Captain		Carmi\r\n"
    "Casluhites	Cassandra	Celeborn	Celebrian	Celebrim\r\n"
    "Celes		Celos		Ceorl		Cerbain		Chaul\r\n"
    "Chidral		Chubb		Cirdan		Cirion		Citadel\r\n"
    "Corsairs	Cotton		Crush		Cuash		Cush\r\n"
    "Daeron		Dain		Damrod		Dark		DarkLord\r\n"
    "Darower         Darkness        David		Deagol		Dedan\r\n"
    "Delving		Demon		Demons		Denethor	Denhelm\r\n"
    "Deor		Deorwine	Deron		Derufin		Dervorin\r\n"
    "Dikiah		Dillo		Dior		Dol             Doldbuck\r\n"
    "Dora		Dori		Doubt		Dragon		Drim\r\n"
    "Drogo		Duilin		Duilthin	Duinhir		Dunadan\r\n"
    "Dunedain	Dunharrow	Dunhere		Dunlendings	Durin\r\n"
    "Dwalin		Dwimmerlaik	Dwimordene	Eagles		Earedil\r\n"
    "Earendil	Earnur		Earthborn	Eber		Ecthelion\r\n"
    "Edain		Edom		Elam		Elandor		Elanor\r\n"
    "Elbereth	Eldaah		Eldamar		Eldar		Eledil\r\n"
    "Elentari	Elentdari	Elessar		ElfStone	Elfhelm\r\n"
    "Eliab		Eliphaz		Elishah		Elladan		Elrohir\r\n"
    "Elrond		Elwing		Enem		Enemy		Ent\r\n"
    "Entmaiden	Eomer		Eomund		Eor             Eored\r\n"
    "Eorl		Eorlingas	Eothain		Eowyn		Epher\r\n"
    "Epitaph		Er		Ere             Erestor		Erk\r\n"
    "Erken		Erkenbrand	Esau		Eshban		Esmeralda\r\n"
    "Eve		Eveard		Evenstar	Fallohide	Fang\r\n"
    "Fangorn		Faramir		Farfoot		Fastred		Fatty\r\n"
    "Fayword		Feanor		Felarof		Fell		Fengel\r\n"
    "Ferny		Fimbrethil	Findegil	Finduilas	Finglas\r\n"
    "Finrod		Firefoot	Firstborn	Fladrif		Flammifer\r\n"
    "Floi		Folca		Folco		Folcwine	Fom\r\n"
    "Forgoil		Forlong		Forn		Foundin		Frar\r\n"
    "Frea		Frealaf		Freawine	Fredegar	FreeLord\r\n"
    "Frodo		From		Gaffer		Galad		Galadhrim\r\n"
    "Galadriel	Galdor		Galmod		Gamgee		Gamling\r\n"
    "Gandalf		Garulf		Gatam		Gether		Ghan\r\n"
    "GhanBuri        Gildor		Gilead		Gilgalad	Giltheoniel\r\n"
    "Gilthoniel	Gimli		Gir		Girgashites     Gith\r\n"
    "Gleowine	Glitter		Gloin		Glorfindel	Goatleaf\r\n"
    "Golasgil	Gold 		Goldberry	Goldwine	Gollum\r\n"
    "Gomer		Gondor		Goodbodies	Gorbadoc	Gorbag\r\n"
    "Gorhen		Gorhendad	Gothmog		Gram		GreatOne\r\n"
    "Greenleaf	Grey		GreyHost	Greyhame	Grima	\r\n"
    "Grimbeorn	Grimbold	Grip		Grishnakh	Grubbs\r\n"
    "Guthlaf		Gwaihir		Hador		Hadoram		Hal\r\n"
    "Halbarad        Haldir		Ham             Hama		Hanan\r\n"
    "Hanoh		Hara		Haradrim	Harding		Harfoot\r\n"
    "Hasufel		Havilah		Hearter		Hearth		Hearther\r\n"
    "Helm		Helmings	Hemdan		Herefara	Heru\r\n"
    "Herubrand	Hirgon		Hirluim		Hittites	Hivites\r\n"
    "Holdwine	Holman		Homam		Hori		Horn\r\n"
    "Hornblower	Hul             Huorn		Huorns		Hurin\r\n"
    "Husham		Iarwain		Imladris	Imrahil		Incanus\r\n"
    "Inglorion	Ingold		Ioreth		Iorlas		Iram\r\n"
    "Iri	    	Iron		Isaac		Isengarders	Isengrim\r\n"
    "Ishbak		Ishmael		Isidur		Ithran		Ithren\r\n"
    "Jair		Jalam		Japh		Japheth		Javan\r\n"
    "Jebusites	Jerah		Jesse		Jeth		Jeush\r\n"
    "Joktan		Jolly		Kedar		Kemo		Kenaz\r\n"
    "Keran		Keturah		Ketz		Kittim		Knakmos\r\n"
    "Kokshan		Korah		Lady		Lagduf		Lamedon\r\n"
    "Lament		Landroval	Lanter		Lassi		Lath\r\n"
    "Lathspell	Laurie		Lea             Leaflock        Lebennin\r\n"
    "Legolas		Lehabites	Leofa		Lindir		Lobelia\r\n"
    "Logain		Logan		Loni		Lore		Lorien\r\n"
    "Lossar		Lossarnach	Lotan		Lotho		Lud\r\n"
    "Ludites		Lufia		Lumpkin		Luthien		Mablung\r\n"
    "Mad		Madai		Magdiel		Maggot		Magog\r\n"
    "Makir		Malbeth		Mali		Manahath	Mardil\r\n"
    "Marigold	Mariner		Marvi		Mauhur		McGee\r\n"
    "Mearas		Medan		Melilot		Meneldil	Meneldor\r\n"
    "Meria		Meriadoc	Merry		Meruadic	Meshech\r\n"
    "Mesheh		Messenger	Mibsam		Midian		Mighty\r\n"
    "Milo		Minrod		Mishma		Mithran		Mithrandir\r\n"
    "Mizr		Mizraim		Morgoth		Morgul		Mourn\r\n"
    "Mugwort		Mumakil		Munak		Mundberg	Muzgash\r\n"
    "Myrin		Nahath		Nameless	Naphtuh		Nashon\r\n"
    "Nathan		Nazqul		Nebaioth	Necromancer	Neeker\r\n"
    "Nethanel	Nibs		Nimrod		Nimrodel	Noah\r\n"
    "Noakes		Nob 		Noldor		Nori		Numenor\r\n"
    "Nuncle		Obal		Odo	    	Ohtar		Oin\r\n"
    "Oldbuck		Oliphaunt	Olorin		Onodirm		Ophir\r\n"
    "Orald		Orc	    	Ori		Orofame		Orofarne\r\n"
    "Orome		Orophin		Otho		Ozem		Parthruestes\r\n"
    "Passim		Passin		Peleg		Pered		Peregrin\r\n"
    "Perian		Periannath	Perrin		Pimple		Pippin\r\n"
    "Praise		Predhil		Prime		Primula		Prince\r\n"
    "Pute		Quickbeem	Raamah		Radagast	Radbug\r\n"
    "Raddai		Ram		Renewer		Reuel		Rhyme\r\n"
    "RingWraith	Riphath		Rising		Rivendell	Rodanim\r\n"
    "Rohan		Roheryn		Rohirrim	Rory		Rose\r\n"
    "Rosie		Rumble		Rumil		Rushlight	Sabin\r\n"
    "Sabta		Sabtea		Sadowfax	Salmin		Sam\r\n"
    "Samwise		Sancho		Saradoc		Saruman		Sasrekah\r\n"
    "Sauron		Scatha		Seba		Seek		Seer\r\n"
    "Segub		Sezron		Shadow		Shagrat		Sharkey\r\n"
    "Sharku		Sheba		Shelah		Sheleph		Shelob\r\n"
    "Shem		Shimea		Shire		Shobal		Shriekers\r\n"
    "Shuah		Siah		Sidian		Sidon		Silvan\r\n"
    "Silver		Sinites		Sires		Sishan		Sishon\r\n"
    "Skinbark	Slinker		Smaug		Smeagol		Snaga\r\n"
    "SnowWhite	Snowmane	Southrons	Steward		Stinker\r\n"
    "Stoor		StormCrow	Strider		Stybba		Summons\r\n"
    "Surinen		Swerting	Targon		Tarshish	Tasarinan\r\n"
    "Telchar		Telcontar	Teman		Terror		Thain\r\n"
    "Tharkun		Thengel		Theoden		Theodred	Thingol\r\n"
    "Thistlewood	Thor		Thorin		Thorondor	Thorongil\r\n"
    "Thrain		Thran		Thranduil	Thror		Timna\r\n"
    "Tintalle	Tinuviel	Tiras		Tobold		Togarmah\r\n"
    "Took		Tor		Tower		Treebeard	Trongvor\r\n"
    "Tubal		Turin		Ufthak		Ugluk		Undomie\r\n"
    "Ungoliant	Urikhai		Uruks		Uz	    	Uzal\r\n"
    "Vala		Valandil	Valar		Varda		Variags\r\n"
    "Voron		Vorondil	Walda		Wanderng	Wandlimb\r\n"
    "Warg		Watcher		Wather		Weken		WhiteHand\r\n"
    "Whitfoot	Widfara		Wight		Wildman		Willie\r\n"
    "Willow		Winch		Wind		WinderWind	Windfola\r\n"
    "Winged		Winter		Witch		Wolf		Worm\r\n"
    "Wormtongue	Wos	    	Wose		Wraith		Youngin\r\n"
    "Zaavan		Zepho		Zerah		Zhammah		Zimran\r\n"
    "Zizzah		Zues\r\n";


char           *atrib =
    "\r\r\n\r\n\nNow your body is being created. You have to choose the main atrributes that\r\n"
    "will describe you. Those are:\r\n"
    "STR - measure of physical power, such as endurance, stamina as well as pure \r\n"
    "      muscle.\r\n"
    "DEX - measures your speed, agility, reflexes and anything else achived by\r\n"
    "      movement.\r\n"
    "CON - represents things like characters fitness and resistance. It determins\r\n"
    "      your max hitpoints.\r\n"
    "INT - your intelligence. Determing your mana gain per level as well as how\r\n"
    "      fast will you learn your spells and skills.\r\n"
    "WIS - combination of character's judgment, enlightnment, will power and\r\n"
    "      intuition. It majorly influences the amount of practices you gain.\r\n"
    "CHA - characters charisma. Roughly, it determines your power over other\r\n"
    "      creatures (like when charming and making peace).\r\n"
    "\r\n"
    "Generally classes like fighters should have higher STR and CON while spell\r\n"
    "casters should pay attention to their INT and WIS.\r\n";
/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/* Should the game automatically create a new wizlist/immlist every time
   someone immorts, or is promoted to a higher (or lower) god level? */
int             use_autowiz = YES;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
int             min_wizlist_lev = LVL_GOD;

int ident=YES;
