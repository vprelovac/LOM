#define __COMM_C__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>


#include "auction.h"
#include "structs.h"
#include "class.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "house.h"
#include "screen.h"
#include "teleport.h"
#include "arena.h"
#include "olc.h"
#include "chatlink.h"
#include "vt100c.h"
#include "ident.h"
#include "events.h"
#include "newquest.h"
#include "spells.h"

/* externs */
extern int      restrict1;
extern int      mini_mud;
extern int      no_rent_check;
extern FILE    *player_fl;
extern int      DFLT_PORT;
extern char    *DFLT_DIR;
extern int      MAX_PLAYERS;
extern int      MAX_DESCRIPTORS_AVAILABLE;


extern char *linewrap_buffer;
extern struct index_data *mob_index;
extern struct player_special_data dummy_mob;    /* In db.c */
extern struct title_type titles1[LVL_IMPL + 1];
extern char    *greet1;
extern struct room_data *world; /* In db.c */
extern int      top_of_world;   /* In db.c */
extern struct time_info_data time_info; /* In db.c */
extern char     help[];
extern char    *dirs[];
/* local globals */
struct descriptor_data *descriptor_list = NULL; /* master desc list */
struct txt_block *bufpool = 0;  /* pool of large output buffers */
extern struct index_data *obj_index;   /* index table for object file	 */
int             buf_largecount = 0;     /* # of large buffers which exist */
int             buf_overflows = 0;      /* # of overflows of output */
int             buf_switches = 0;       /* # of switches from small to large
                                         * buf */
int             circle_shutdown = 0;    /* clean shutdown */
int             circle_reboot = 0;      /* reboot the game after a shutdown */
int             no_specials = 0;/* Suppress ass. of special routines */
int             avail_descs = 0;/* max descriptors available */
int             tics = 0;       /* for extern checkpointing */
int             scheck = 0;     /* for syntax checking mode */
bool            MOBTrigger = TRUE;      /* For MOBProgs */
extern int      nameserver_is_slow;     /* see config.c */
extern int      auto_save;      /* see config.c */
extern int      autosave_time;  /* see config.c */
extern int total_ram;
struct timeval  null_time;      /* zero-valued time structure */
char bufza1[100];
char bufza[100];
extern int boothigh;
int port;
int mozeranddebug=0;
struct timeval curr_time;
extern time_t boot_time_real;
/* functions in this file */
void perform_rooms();
void check_idle_passwords(void);
int             get_from_q(struct txt_q * queue, char *dest, int *aliased);
void            init_game(int port);
void            signal_setup(void);
void            game_loop(int mother_desc);
int             init_socket(int port);
int             new_descriptor(int s);
int             get_avail_descs(void);
int             process_output(struct descriptor_data * t);
int             process_input(struct descriptor_data * t);
void            close_socket(struct descriptor_data * d);
struct timeval  timediff(struct timeval * a, struct timeval * b);
void            flush_queues(struct descriptor_data * d);
void            nonblock(int s);
int             perform_subst(struct descriptor_data * t, char *orig, char *subst);
int             perform_alias(struct descriptor_data * d, char *orig);
void            record_usage(void);
char           *make_prompt(struct descriptor_data * point);
void            UpdateScreen(struct descriptor_data *t);
void            InitScreen(struct char_data * ch);
extern int generate_autoquest();

/* extern fcnts */
char           *show_char_cond(int percent);
void            boot_db(void);
void            boot_world(void);
void            zone_update(void);
void            affect_update(void);    /* In spells.c */
void            point_update(void);     /* In limits.c */
void            mobile_activity(void);
void            string_add(struct descriptor_data * d, char *str);
void            perform_violence(void);
void            show_string(struct descriptor_data * d, char *input);
int             isbanned(char *hostname);
void            weather_and_time(int mode);
extern void     mprog_act_trigger(char *buf, struct char_data * mob, struct char_data * ch, struct obj_data * obj, void *vo);
void            auction_update(void);
void            start_arena();
extern int      in_arena;
void            do_game();
extern void     restore_all(char *name);
void            load_quad();
void proc_color(char *inbuf, int color_lvl);
void panic();
extern int players_online();
extern void save_mobkills();
extern void save_topdam();
extern void save_tophurt();

FILE *ff;
/* *********************************************************************
*  main game loop and related stuff                                    *
********************************************************************* */

int             main(int argc, char **argv)
{
    char            buf[512];
    int             pos = 1;
    char           *dir;

    port = DFLT_PORT;
    dir = DFLT_DIR;

   #if defined(ADV_MEMORY)
    init_memory();
   #endif



    ff=fopen("bran.dat","a");
    leech_list=NULL;
    /* Initialize dummy_mob variable. */
    memset(&dummy_mob, 0, sizeof(dummy_mob));
    /*//    CREATE(auction, struct auction_data, 1);*/

    while ((pos < argc) && (*(argv[pos]) == '-')) {
        switch (*(argv[pos] + 1)) {
        case 'd':
            if (*(argv[pos] + 2))
                dir = argv[pos] + 2;
            else if (++pos < argc)
                dir = argv[pos];
            else {
                log("Directory arg expected after option -d.");
                exit(1);
            }
            break;
        case 'm':
            mini_mud = 1;
            no_rent_check = 1;
            log("Running in minimized mode & with no rent check.");
            break;
        case 'c':
            scheck = 1;
            log("Syntax check mode enabled.");
            break;
        case 'q':
            no_rent_check = 1;
            log("Quick boot mode -- rent check supressed.");
            break;
        case 'r':
            restrict1 = 1;
            log("Restricting game -- no new players allowed.");
            break;
        case 's':
            no_specials = 1;
            log("Suppressing assignment of special routines.");
            break;
        default:
            sprintf(buf, "SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
            log(buf);
            break;
        }
        pos++;
    }

    if (pos < argc) {
        if (!isdigit((char) *argv[pos])) {
            fprintf(stderr, "Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n", argv[0]);
            exit(1);
        } else if ((port = atoi(argv[pos])) <= 1024) {
            fprintf(stderr, "Illegal port number.\n");
            exit(1);
        }
    }
    if (chdir(dir) < 0) {
        perror("Fatal error changing to data directory");
        exit(1);
    }
    sprintf(buf, "Using %s as data directory.", dir);
    log(buf);

    if (scheck) {
        boot_world();
        log("Done.");
        exit(0);
    } else {
        sprintf(buf, "Running game on port %d.", port);
        log(buf);
        init_game(port);
    }

    if (ff)
        fclose(ff);
    return 0;
}



/* Init sockets, run game, and cleanup sockets */
void            init_game(int port)
{
    int             mother_desc;
    void            my_srand(unsigned long initial_seed);


    log("Opening mother connection.");
    mother_desc = init_socket(port);
    avail_descs = get_avail_descs();     
    
    srand(time(0));
    my_srand(time(0));

    event_init();
    boot_db();
    
    
    

    //my_srand(my_rand());
    log("Signal trapping.");
    signal_setup();

    log("Entering game loop.");
    mozeranddebug=1;

    game_loop(mother_desc);

    log("Closing all sockets.");
    while (descriptor_list)
        close_socket(descriptor_list);

    close(mother_desc);
    fclose(player_fl);

    if (circle_reboot) {
        log("Rebooting.");
        exit(52);               /* what's so great about HHGTTG, anyhow? */
    }
    log("Normal termination of game.");
    if (ff)
        fclose(ff);
}


void write_mud_date_to_file(void)
{
    FILE *f;
    struct time_write date;

    f = fopen("etc/date_record", "wb");
    date.year = time_info.year;
    date.month = time_info.month;
    date.day   = time_info.day;
    fwrite(&date,sizeof(struct time_write),1,f);
    fflush(f);
    fclose(f);
}

/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
int             init_socket(int port)
{
    int             s,
    opt;
    struct sockaddr_in sa;

    /* Should the first argument to socket() be AF_INET or PF_INET?  I don't
     * know, take your pick.  PF_INET seems to be more widely adopted, and
     * Comer (_Internetworking with TCP/IP_) even makes a point to say that
     * people erroneously use AF_INET with socket() when they should be using
     * PF_INET.  However, the man pages of some systems indicate that AF_INET
     * is correct; some such as ConvexOS even say that you can use either
     * one. All implementations I've seen define AF_INET and PF_INET to be
     * the same number anyway, so ths point is (hopefully) moot. */

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Create socket");
        exit(1);
    }

#if defined(SO_SNDBUF)
    opt = LARGE_BUFSIZE + GARBAGE_SPACE;
    if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
        perror("setsockopt SNDBUF");
        exit(1);
    }
#endif

#if defined(SO_REUSEADDR)
    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        perror("setsockopt REUSEADDR");
        exit(1);
    }
#endif

#if defined(SO_REUSEPORT)
    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char *) &opt, sizeof(opt)) < 0) {
        perror("setsockopt REUSEPORT");
        exit(1);
    }
#endif

#if defined(SO_LINGER)
    {
        struct linger   ld;

        ld.l_onoff = 0;
        ld.l_linger = 0;
        if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
            perror("setsockopt LINGER");
            exit(1);
        }
    }
#endif

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *) & sa, sizeof(sa)) < 0) {
        perror("bind");
        close(s);
        exit(1);
    }
    nonblock(s);
    listen(s, 50);
    return s;
}


int             get_avail_descs(void)
{
    unsigned int             max_descs = 0;

	
    /*
     * First, we'll try using getrlimit/setrlimit.  This will probably work
     * on most systems.
     */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
    {
        struct rlimit   limit;

        getrlimit(RLIMIT_NOFILE, &limit);
        max_descs = UMIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
        limit.rlim_cur = max_descs;
        setrlimit(RLIMIT_NOFILE, &limit);
    }
#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
    max_descs = OPEN_MAX;       /* Uh oh.. rlimit didn't work, but we have
    * OPEN_MAX */
#else
    /* Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
    * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
    * in the UNIX Environment_). */
    errno = 0;
    if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
        if (errno == 0)
            max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
        else {
            perror("Error calling sysconf");
            exit(1);
        }
    }
#endif

    max_descs = UMIN((unsigned int) MAX_PLAYERS, (unsigned int) (max_descs - NUM_RESERVED_DESCS));

    if (max_descs <= 0) {
        log("Non-positive max player limit!");
        exit(1);
    }
    sprintf(buf, "Setting player limit to %d.", max_descs);
    log(buf);
    return max_descs;
}



/*
 * game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity().
 */

unsigned int   pulse = 0;
void            beware_lightning(void);


void            game_loop(int mother_desc)
{
    fd_set          input_set,
    output_set,
    exc_set;
    struct timeval  last_time,
                now,
                timespent,
                timeout,
                opt_time;
    char            comm[MAX_INPUT_LENGTH];
    struct descriptor_data *d,
                *next_d;
    int             mins_since_crashsave = 0,
                                           maxdesc,
                                           aliased;

    /* initialize various time values */
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    opt_time.tv_usec = OPT_USEC;
    opt_time.tv_sec = 0;
    gettimeofday(&last_time, (struct timezone *) 0);

    /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho. The.. */
while (circle_shutdown != 1) {

        /* Sleep if we don't have any connections */
        if (descriptor_list == NULL) {
            log("No connections.  Going to sleep.");
            FD_ZERO(&input_set);
            FD_SET(mother_desc, &input_set);
            if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
                if (errno == EINTR)
                    log("Waking up to process signal.");
                else
                    perror("Select coma");
            } else
                log("New connection.  Waking up.");
            gettimeofday(&last_time, (struct timezone *) 0);
        }
        /* Set up the input, output, and exception sets for select(). */
        FD_ZERO(&input_set);
        FD_ZERO(&output_set);
        FD_ZERO(&exc_set);
        FD_SET(mother_desc, &input_set);
        maxdesc = mother_desc;
        for (d = descriptor_list; d; d = d->next) {
            if (d->descriptor > maxdesc)
                maxdesc = d->descriptor;
            FD_SET(d->descriptor, &input_set);
            FD_SET(d->descriptor, &output_set);
            FD_SET(d->descriptor, &exc_set);
        }

        /* At this point, the original Diku code set up a signal mask to
         * avoid block all signals from being delivered.  I believe this was
         * done in order to prevent the MUD from dying with an "interrupted
         * system call" error in the event that a signal be received while
         * the MUD is dormant. However, I think it is easier to check for an
         * EINTR error return from this select() call rather than to block
         * and unblock signals. */
        do {
            errno = 0;          /* clear error condition */

            pulse++;

            if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
                weather_and_time(1);
            event_process();


            /* figure out for how long we have to sleep */
            gettimeofday(&now, (struct timezone *) 0);
            timespent = timediff(&now, &last_time);
            timeout = timediff(&opt_time, &timespent);

            /* sleep until the next 0.1 second mark */
            if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
                if (errno != EINTR) {
                    perror("Select sleep");
                    exit(1);
                }
        } while (errno);

        /* record the time for the next pass */
        gettimeofday(&last_time, (struct timezone *) 0);

        /* poll (without blocking) for new input, output, and exceptions */
        if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
            perror("Select poll");
            return;
        }
        /* New connection waiting for us? */
        if (FD_ISSET(mother_desc, &input_set))
            new_descriptor(mother_desc);

        /* kick out the freaky folks in the exception set */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            if (FD_ISSET(d->descriptor, &exc_set) || d->close_me) {
                FD_CLR(d->descriptor, &input_set);
                FD_CLR(d->descriptor, &output_set);
                close_socket(d);
            }
        }

        /* process descriptors with input pending */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            if (FD_ISSET(d->descriptor, &input_set))
                if (process_input(d) < 0)
                    close_socket(d);
        }

        /* process descriptors with ident pending */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            if (waiting_for_ident(d))
                ident_check(d, pulse);
        }


        /* process commands we just read from process_input */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;

            if ((--(d->wait) <= 0) && get_from_q(&d->input, comm, &aliased) && !waiting_for_ident(d)) {
                if (d->character) {
                    d->character->char_specials.timer = 0;
                    if (!d->connected && GET_WAS_IN(d->character) != NOWHERE) {
                        if (d->character->in_room != NOWHERE)
                            char_from_room(d->character);
                        char_to_room(d->character, GET_WAS_IN(d->character));
                        GET_WAS_IN(d->character) = NOWHERE;
                        act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
                    }
                }
                d->wait = 1;
                d->has_prompt = 0;

                /* reversed these top 2 if checks so that you can use the
                 * page_string */
                /* function in the editor */
                if (d->showstr_point)   /* reading something w/ pager     */
                    show_string(d, comm);
                else if (d->str)/* writing boards, mail, etc.     */
                    string_add(d, comm);
                else if (d->connected != CON_PLAYING)   /* in menus, etc.	 */
                    nanny(d, comm);
                else {          /* else: we're playing normally */
                    if (aliased)/* to prevent recursive aliases */
                        d->has_prompt = 1;
                    else {
                        if (perform_alias(d, comm))     /* run it through
                                                             * aliasing system */
                            get_from_q(&d->input, comm, &aliased);
                    }
                    command_interpreter(d->character, comm);    /* send it to
                                                                 * interpreter */
                }
            }
        }

        if (circle_shutdown > 1) {
            switch (circle_shutdown) {
            case (3600 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL SHUTDOWN IN 1 HOUR\r\n");
                break;
            case (1800 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 30 MINUTES\r\n");
                break;
            case (1200 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 20 MINUTES\r\n");
                break;
            case (600 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 10 MINUTES\r\n");
                break;
            case (300 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 5 MINUTES\r\n");
                break;
            case (180 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 3 MINUTES\r\n");
                break;
            case (60 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 1 MINUTE.\r\n");
                send_to_all("We suggest you go somewhere safe and 'save'...\r\n");
                break;
            case (30 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 30 SECONDS\r\n");
                break;
            case (10 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 10 SECONDS\r\n");
                break;
            case (9 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 9 SECONDS\r\n");
                break;
            case (8 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 8 SECONDS\r\n");
                break;
            case (7 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 7 SECONDS\r\n");
                break;
            case (6 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 6 SECONDS\r\n");
                break;
            case (5 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 5 SECONDS\r\n");
                break;
            case (4 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 4 SECONDS\r\n");
                break;
            case (3 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 3 SECONDS\r\n");
                break;
            case (2 RL_SEC):
                            send_to_all("\007\007Land of Myst WILL REBOOT IN 2 SECONDS\r\n");

                break;
            case (1 RL_SEC):
                            send_to_all("\007\007SHUTTING DOWN\007\007\r\n");
                send_to_all("Lands of Myst will reboot as soon as possible.\r\n");
                save_mobkills();
                save_topdam();
                save_tophurt();
                Crash_save_all();
                House_save_all();
                break;
            }
            circle_shutdown--;
        }
        /* send queued output out to the operating system (ultimately to
         * user) */
for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            /*//      if (FD_ISSET(d->descriptor, &output_set) && *(d->output))*/
            if (*(d->output) && FD_ISSET(d->descriptor, &output_set)) {
                /* Output for this player is ready */

                if (d && process_output(d) < 0)
                    close_socket(d);
                else
                    d->has_prompt = 1;
            }
        }

        /* Print prompts for other descriptors who had no other output */
        for (d = descriptor_list; d; d = d->next) {
            if (!d->has_prompt) {
                if (d->connected == CON_PLAYING && GET_TERM(d->character) == VT100) {
                    UpdateScreen(d);
                    /*                   {
                                           int             door;
                                            *bufza = '\0';*buf2='\0';
                                            for (door = 0; door < NUM_OF_DIRS; door++)
                                                if (EXIT(d->character, door) && EXIT(d->character, door)->to_room != NOWHERE && !IS_SET(EXIT(d->character, door)->exit_info, EX_CLOSED))
                                                    sprintf(bufza, "%s%c", bufza, UPPER(*dirs[door]));
                                            if (GET_POS(d->character) <= POS_SLEEPING)
                                                sprintf(bufza1, "??");
                                            else
                                                sprintf(bufza1, "%s", *bufza ? bufza : "None!");
                                           strcat(buf2,">  ");

                                        write_to_descriptor(d->descriptor, bufza1);
                                        }*/
                    write_to_descriptor(d->descriptor, "> ");
                } else
                    write_to_descriptor(d->descriptor, make_prompt(d));
                d->has_prompt = 1;
            }
            /*            else if (d->wait>1 && !((pulse+1)%PASSES_PER_SEC)) {
                            if (d->connected == CON_PLAYING && GET_TERM(d->character) == VT100) {
                                write_to_descriptor(d->descriptor, "\r\n(*) > ");
                            } else
                                write_to_descriptor(d->descriptor, make_prompt(d));
                            d->has_prompt = 1;
                        }*/
        }

        /* kick out folks in the CON_CLOSE state */
        for (d = descriptor_list; d; d = next_d) {
            next_d = d->next;
            if (STATE(d) == CON_CLOSE)
                close_socket(d);
        }

        /* handle heartbeat stuff */
        /* Note: pulse now changes every 0.10 seconds  */
        
        
          /*
        	zone_update();		           
        	mobile_activity();      
        	perform_violence();     
        	affect_update();        
        	point_update();         
        	zone_update();		     
        mobile_activity();    
        perform_violence();   
        affect_update();      
        point_update();       
                  zone_update();
          mobile_activity();    
          perform_violence();   
          affect_update();      
          point_update();       
        	zone_update();		           
        	mobile_activity();      
        	perform_violence();     
        	affect_update();        
        	point_update();         
        	zone_update();		     
        mobile_activity();    
        perform_violence();   
        affect_update();      
        point_update();       
                  zone_update();
          mobile_activity();    
          perform_violence();   
          affect_update();      
          point_update();       
        
        */
        mob_act_update ();
        obj_act_update ();
        room_act_update();

        if (!(pulse % PULSE_ZONE))
            zone_update();
        if (!(pulse % (60 * PASSES_PER_SEC)))		/* 60 seconds */
            check_idle_passwords();
        if (!(pulse % PULSE_MOBILE))
            mobile_activity();
        if (!(pulse % PULSE_VIOLENCE))
            perform_violence();
        /* Do Arena related stuff */
        if (in_arena == ARENA_START)
            if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
                start_arena();
        if (in_arena == ARENA_RUNNING)
            if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
                do_game();
        if (!(pulse % PULSE_PLAYERS))
            perform_players(0);
        /*    if (!(pulse % PULSE_TELEPORT))
        //      TeleportPulseStuff(pulse);*/
        if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC / 4))) {
            point_update();
            perform_rooms();
            beware_lightning();
            generate_zonekill_ratio();
        }
        if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) { /* 1 minutes */
            affect_update();
            quest_update();
            newquest_update();
            update_blood();

        }

        if (!(pulse % (150 * PASSES_PER_SEC)))      /* 3 minute */
        {
            mins_since_crashsave = 0;
            Crash_save_all();
            House_save_all();
            fflush(player_fl);
        }
        if (!(pulse % (1800 * PASSES_PER_SEC))) /* 30 minutes */
        {
            record_usage();
            if (!number(0, 47))
                load_quad();
            else if (!number(0, 15))
                generate_autoquest();    
        }
        if (!(pulse % (PULSE_AUCTION)))
            auction_update();
        if (!(pulse % (512 * PASSES_PER_SEC)))
            write_who_file();
        if (!(pulse % (10000 * PASSES_PER_SEC))) /* 90 minutes */
        {
            save_mobkills();
            save_topdam();
            save_tophurt();
            restore_all("the mercy of Gods");
            write_mud_date_to_file();
        }

        clean_char_queue();
        clean_obj_queue();

        if (pulse >= (INT_MAX)) {      // mnogo
            pulse = 0;
        }
        tics++;                 /* tics since last checkpoint signal */
    }
}



/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
 *  new code to calculate time differences, which works on systems
 *  for which tv_usec is unsigned (and thus comparisons for something
 *  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
 */

/*
 * code to return the time difference between a and b (a-b).
 * always returns a nonnegative value (floors at 0).
 */
struct timeval  timediff(struct timeval * a, struct timeval * b)
{
    struct timeval  rslt;

    if (a->tv_sec < b->tv_sec)
        return null_time;
    else if (a->tv_sec == b->tv_sec) {
        if (a->tv_usec < b->tv_usec)
            return null_time;
        else {
            rslt.tv_sec = 0;
            rslt.tv_usec = a->tv_usec - b->tv_usec;
            return rslt;
        }
    } else {                    /* a->tv_sec > b->tv_sec */
        rslt.tv_sec = a->tv_sec - b->tv_sec;
        if (a->tv_usec < b->tv_usec) {
            rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
            rslt.tv_sec--;
        } else
            rslt.tv_usec = a->tv_usec - b->tv_usec;
        return rslt;
    }
}


void            record_usage(void)
{
    int             sockets_connected = 0,
                                        sockets_playing = 0;

    struct descriptor_data *d;
    char            buf[256];

    for (d = descriptor_list; d; d = d->next) {
        sockets_connected++;
        if (!d->connected)
            sockets_playing++;
    }

    sprintf(buf, "USAGE: %3d players (%3d high)  Allocated RAM: %.3f MB",
            sockets_playing, boothigh, (float ) total_ram/(1024*1024));
    log(buf);
    /*    {
            struct rusage   ru;

            getrusage(0, &ru);
            sprintf(buf, "rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
                    ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
            log(buf);
        }*/
}




char           *make_prompt(struct descriptor_data * d)
{

    int             percent, door, skill, qend, flag;
    struct char_data *tank;
    struct follow_type *f;
    static char     prompt[256];
    struct char_data *ch, *pom=NULL;
    const char     *str;
    const char     *i;
    char           *point;
    char            buf[MAX_STRING_LENGTH];
    char            buf2[MAX_STRING_LENGTH], pelp[100];
    int clvl=0, movemin=1000000, hitmin=1000000;



    if (!d) return NULL;
    /* reversed these top 2 if checks so that page_string() would work in */
    /* the editor */
    if (d->showstr_point) {
        strcpy(prompt, "&0*** Press return to continue, q to quit ***");
    } else if (d->str)
        strcpy(prompt, "] ");
    else if (!d->connected && d->character->in_room>-1) {

        *prompt = '\0';
        /*        if (d->wait>1)
                    strcpy(prompt,"\r\n(*) ");
        */

        {

            if(d->character)
                clvl=COLOR_LEV(d->character);
            point = buf;
            ch = d->character;
            str = FIGHTING(ch)? ch->player_specials->saved.prompt_combat:ch->player_specials->saved.prompt;

            while (*str != '\0') {
                if (*str != '%') {
                    *point++ = *str++;
                    continue;
                }
                ++str;
                switch (*str) {
                default:
                    i = " ";
                    break;
                case 'f':
                    if (AFF_FLAGGED(ch, AFF_SANCTUARY) || AFF_FLAGGED(ch, AFF_HIDE) ||
                            AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_SILVER) || AFF3_FLAGGED(ch, AFF3_QUAD) || AFF2_FLAGGED(ch, AFF2_PRISM) || AFF2_FLAGGED(ch, AFF2_ULTRA))
                    {
                        strcpy(buf2, "(");
                        //if (AFF3_FLAGGED(ch, AFF3_QUAD))
                        //strcat(buf2, "Q");
                        //if (AFF_FLAGGED(ch, AFF_SANCTUARY))
                        //                      	strcat(buf2, "S");
                        if (AFF_FLAGGED(ch, AFF_SNEAK))
                            strcat(buf2, "S");
                        //if (AFF2_FLAGGED(ch, AFF2_PRISM))
                        //strcat(buf2, "P");
                        //if (AFF2_FLAGGED(ch, AFF2_ULTRA))
                        //                      	strcat(buf2, "U");
                        //                    if (AFF_FLAGGED(ch, AFF_SILVER))
                        //                  	strcat(buf2, "A");
                        if (AFF_FLAGGED(ch, AFF_HIDE))
                            strcat(buf2, "H");
                        if (AFF_FLAGGED(ch, AFF_INVISIBLE))
                            strcat(buf2, "I");
                        strcat(buf2, ")");
                    }
                    else
                        *buf2='\0';
                    i=buf2;
                    break;
                case 'a':
                    str++;
                    if (*str != '\'')
                        return "Illegal usage of %a code - see 'help prompt' >";
                    for (qend = 1; *(str + qend) && (*(str + qend) != '\''); qend++);
                    if (*(str+qend) != '\'')
                        return "Illegal usage of %a code - see 'help prompt' >";
                    *buf2=0;
                    //strncpy(buf2, (str + 1), qend-1);
                    strcpy(buf2, str+1);
                    *(buf2+qend-1)='\0';
                    if ((skill = find_skill_num(buf2)) <= 0)
                    {
                        sprintf(prompt, "Unrecognized skill in %%a code (%s) >", buf2);
                        return  prompt;
                    }
                    if (affected_by_spell(ch, skill))
                        sprintf(buf2, "%c", UPPER(*spells[skill]));
                    else
                        *buf2=0;
                    str+=qend;
                    i=buf2;
                    break;
                case 'h':
                    sprintf(buf2, "%d", (int) GET_HIT(ch));
                    i = buf2;
                    break;
                case 'H':
                    sprintf(buf2, "%d", (int) GET_MAX_HIT(ch));
                    i = buf2;
                    break;
                case 'm':
                    sprintf(buf2, "%d", GET_MANA(ch));
                    i = buf2;
                    break;
                case 'M':
                    sprintf(buf2, "%d", GET_MAX_MANA(ch));
                    i = buf2;
                    break;
                case 'v':
                    sprintf(buf2, "%d", GET_MOVE(ch));
                    i = buf2;
                    break;
                case 'V':
                    sprintf(buf2, "%d", GET_MAX_MOVE(ch));
                    i = buf2;
                    break;
                case 'l':
                    sprintf(buf2, "%d", ch->mana_leech);
                    i = buf2;
                    break;
                case 'x':
                    if (GET_LEVEL(d->character) < (LVL_IMMORT - 1) && !IS_MOB(d->character))
                        sprintf(buf2,
                                "%d",
                                //                                total_exp(GET_LEVEL(d->character)) - GET_EXP(d->character));
                                LEVELEXP(d->character) - GET_EXP(d->character));
                    i = buf2;
                    break;
                case 'X':
                    if (GET_LEVEL(d->character) < (LVL_IMMORT - 1) && !IS_MOB(d->character))
                        sprintf(buf2, "%d", GET_EXP(ch));
                    i = buf2;
                    break;
                case 'w':
                    sprintf(buf2, "%d", (float) 0.2*(ch->desc->wait-1)>0?(int) ((float) 0.2*(ch->desc->wait-1)+0.5):0);
                    i = buf2;
                    break;
                case 'g':
                    sprintf(buf2, "%d", GET_GOLD(ch));
                    i = buf2;
                    break;
                case '^':
                    sprintf(buf2, "\r\n");
                    i = buf2;
                    break;
                case 'n':
                    sprintf(buf2, "%s", GET_NAME(ch));
                    i=buf2;
                    break;
                case 't':
                    *buf2='\0';
                    if (FIGHTING(d->character)) {

                        tank = FIGHTING(FIGHTING(d->character));


                        if (tank && is_same_group(d->character, tank)) {

                            if (GET_MAX_HIT(tank))
                                percent = (int) 100 *GET_HIT(tank) / GET_MAX_HIT(tank);
                            else
                                percent = -1;
                            sprintf(buf2, "%s:%s", GET_NAME(tank), show_char_cond(percent));
                        }
                    }
                    i = buf2;
                    break;

                case 'b':

                    tank=(ch->master ? ch->master : ch);
                    movemin=GET_MOVE(tank);
                    pom=tank;
                    for (f = tank->followers; f; f = f->next)
                        if (!IS_NPC(f->follower) && GET_MOVE(f->follower)<movemin)
                        {
                            movemin=GET_MOVE(f->follower);
                            pom=f->follower;
                        }
                    if (pom && is_same_group(pom, ch))
                        sprintf(buf2, "%s:%d", GET_NAME(pom), movemin);
                    else
                        sprintf(buf2, "%s:%d", GET_NAME(ch), GET_MOVE(ch));


                    i=buf2;
                    break;
                case 'j':



                    tank=(ch->master ? ch->master : ch);
                    pom=tank;
                    hitmin=GET_HIT(tank);
                    for (f = tank->followers; f; f = f->next)
                        if (!IS_NPC(f->follower) && GET_HIT(f->follower)<hitmin)
                        {
                            hitmin=GET_HIT(f->follower);
                            pom=f->follower;
                        }

                    if (pom && is_same_group(pom, ch))
                        sprintf(buf2, "%s:%d", GET_NAME(pom), hitmin);
                    else
                        sprintf(buf2, "%s:%d", GET_NAME(ch), GET_HIT(ch));
                    i=buf2;
                    break;
                case 'k':


                    tank=(ch->master ? ch->master : ch);
                    hitmin=100*GET_HIT(tank)/GET_MAX_HIT(tank);
                    pom=tank;
                    for (f = tank->followers; f; f = f->next)
                        if (!IS_NPC(f->follower) && ((100*GET_HIT(f->follower)/GET_MAX_HIT(f->follower))<hitmin))
                        {
                            hitmin=100*GET_HIT(f->follower)/GET_MAX_HIT(f->follower);
                            pom=f->follower;
                        }

                    if (pom && is_same_group(pom, ch))
                        sprintf(buf2, "%s:%d%%", GET_NAME(pom), hitmin);
                    else
                        sprintf(buf2, "%s:%d", GET_NAME(ch), 100*GET_HIT(ch)/GET_MAX_HIT(ch));
                    i=buf2;
                    break;
                    /*case 'a':
                        if (!FIGHTING(d->character)) {
                            int             door;
                            
                            *bufza = '\0';
                            for (door = 0; door < NUM_OF_DIRS; door++)
                                if (EXIT(d->character, door) && EXIT(d->character, door)->to_room != NOWHERE && !IS_SET(EXIT(d->character, door)->exit_info, EX_CLOSED))
                                    sprintf(bufza, "%s%c", bufza, UPPER(*dirs[door]));
                            if (GET_POS(d->character) <= POS_SLEEPING)
                                sprintf(buf2, "??");
                            else
                                sprintf(buf2, "%s", *bufza ? bufza : "None!");
                        } else {
                            percent = (int) 100 *GET_HIT(FIGHTING(d->character)) / GET_MAX_HIT(FIGHTING(d->character));

                            sprintf(buf2, "[%s]", show_char_cond(percent));
                        }
                        i = buf2;
                        break;
                      */
                case 'e':


                    flag=0;
                    *bufza = '\0';
                    for (door = 0; door < NUM_OF_DIRS; door++)
                        if (EXIT(d->character, door) && EXIT(d->character, door)->to_room != NOWHERE && !IS_SET(EXIT(d->character, door)->exit_info, EX_CLOSED) && !IS_SET(EXIT(d->character, door)->exit_info, EX_HIDDEN)) {
                            if (door<NORTHEAST)
                            {
                                flag=1;
                                sprintf(bufza, "%s%c", bufza, UPPER(*dirs[door]));
                            }
                            else
                            {
                                switch(door)
                                {
                                case NORTHEAST:
                                    sprintf(bufza, "%s%s%s", bufza,flag?" ":"","NE");flag=1;break;
                                case NORTHWEST:
                                    sprintf(bufza, "%s%s%s", bufza,flag?" ":"","NW");flag=1;break;
                                case SOUTHEAST:
                                    sprintf(bufza, "%s%s%s", bufza,flag?" ":"","SE");flag=1;break;
                                case SOUTHWEST:
                                    sprintf(bufza, "%s%s%s", bufza,flag?" ":"","SW");flag=1;break;
                                default:
                                    sprintf(bufza, "%s%s%s", bufza,flag?" ":"","??");flag=1;break;
                                }
                            }
                        }
                    
                    if (GET_POS(d->character) == POS_SLEEPING)
                        sprintf(buf2, "%2d",15-((pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC / 4))/PASSES_PER_SEC ));
                    else if (GET_POS(d->character) < POS_SLEEPING || IS_AFFECTED(d->character, AFF_BLIND))
                        sprintf(buf2, "??");                     
                    else
                        sprintf(buf2, "%s", *bufza ? bufza : "None!");
                    i = buf2;

                    break;
                case 'd':
                    *buf2=0;
                    if (FIGHTING(ch))
                    {
                        percent = (int) 100 *GET_HIT(FIGHTING(d->character)) / GET_MAX_HIT(FIGHTING(d->character));
                        sprintf(buf2, "%s", show_char_cond(percent));
                    }
                    i=buf2;
                    break;
                case 'o':
                    *buf2=0;
                    if (FIGHTING(ch))
                    {
                        percent = (int) 100 *GET_HIT(FIGHTING(d->character)) / GET_MAX_HIT(FIGHTING(d->character));
                        sprintf(buf2, "%d%%", MAX(0, percent));
                    }
                    i=buf2;
                    break;
                case 'O':
                    *buf2=0;
                    if (FIGHTING(ch))
                    {
                        sprintf(buf2, "%s", GET_NAME(FIGHTING(ch)));
                    }
                    i=buf2;
                    break;
                case '%':
                    sprintf(buf2, "%%");
                    i = buf2;
                    break;
                }
                ++str;
                while ((*point = *i) != '\0')
                    ++point, ++i;
            }
            *point = '\0';
            strcpy(prompt, buf);
        }

    } else
        *prompt = '\0';
    strcat(prompt,"&0");
    proc_color(prompt,clvl);
    return prompt;
}


void            write_to_q(char *txt, struct txt_q * queue, int aliased)
{
    struct txt_block *new;

    CREATE(new, struct txt_block, 1);
    CREATE(new->text, char, strlen(txt) + 1);
    strcpy(new->text, txt);
    new->aliased = aliased;

    /* queue empty? */
    if (!queue->head) {
        new->next = NULL;
        queue->head = queue->tail = new;
    } else {
        queue->tail->next = new;
        queue->tail = new;
        new->next = NULL;
    }
}



int             get_from_q(struct txt_q * queue, char *dest, int *aliased)
{
    struct txt_block *tmp;

    /* queue empty? */
    if (!queue->head)
        return 0;

    tmp = queue->head;
    strcpy(dest, queue->head->text);
    *aliased = queue->head->aliased;
    queue->head = queue->head->next;

    DISPOSE(tmp->text);
    DISPOSE(tmp);

    return 1;
}



/* Empty the queues before closing connection */
void            flush_queues(struct descriptor_data * d)
{
    int             dummy;

    if (d->large_outbuf) {
        d->large_outbuf->next = bufpool;
        bufpool = d->large_outbuf;
    }
    while (get_from_q(&d->input, buf2, &dummy));
}

#undef TWOX_DEBUG
#define TWOX_MAX	99      /* Max value of (x*) digit.	 */
#define TWOX_LEN	2       /* How many digits TWOX_MAX is.	 */

void            write_to_output(const char *txt, struct descriptor_data * t)
{
    int             size;

    size = strlen(txt);

    /* if we're in the overflow state already, ignore this new output */
    if (t->bufptr < 0 || !txt || !*txt)
        return;


    /* if we have enough space, just write to buffer and that's it! */
    if (t->bufspace >= size) {
        strcpy(t->output + t->bufptr, txt);
        t->bufspace -= size;
        t->bufptr += size;
    } else {                    /* otherwise, try switching to a lrg buffer */
        if (size + t->bufptr > LARGE_BUFSIZE - 1) {
            /* we're already using large buffer, or even the large buffer
             * isn't big enough -- switch to overflow state */
            t->bufptr = -1;
            buf_overflows++;
            return;
        }
        buf_switches++;

        /* if the pool has a buffer in it, grab it */
        if (bufpool != NULL) {
            t->large_outbuf = bufpool;
            bufpool = bufpool->next;
        } else {                /* else create a new one */
            CREATE(t->large_outbuf, struct txt_block, 1);
            CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
            buf_largecount++;
        }

        strcpy(t->large_outbuf->text, t->output);       /* copy to big buffer */
        t->output = t->large_outbuf->text;      /* make big buffer primary */
        strcat(t->output, txt); /* now add new text */


        /* set the pointer for the next write */
        t->bufptr = strlen(t->output);

        /* calculate how much space is left in the buffer */
        t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;

    }
}





/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */


int             new_descriptor(int s)
{
    int             desc,
    sockets_connected = 0;
    unsigned long   addr;
    socklen_t              i;
    static int      last_desc = 0;      /* last descriptor number */
    struct descriptor_data *newd;
    struct sockaddr_in peer;
    struct hostent *from;
    extern char    *GREETINGS;
    char titlescreen[LARGE_BUFSIZE];
    time_t current_time;
    /* accept the new connection */
    i = sizeof(peer);
    if ((desc = accept(s, (struct sockaddr *) & peer, &i)) < 0) {
        perror("accept");
        return -1;
    }
    /* keep it from blocking */
    nonblock(desc);

    /* make sure we have room for it */
    for (newd = descriptor_list; newd; newd = newd->next) {
        sockets_connected++;
        if (newd->ident_sock != -1)
            sockets_connected++;
    }

    if (sockets_connected >= avail_descs) {
        write_to_descriptor(desc, "Sorry, Lands of Myst is full right now... try again later!\r\n");
        close(desc);
        return 0;
    }
    /* create a new descriptor */
    CREATE(newd, struct descriptor_data, 1);
    memset((char *) newd, 0, sizeof(struct descriptor_data));

    /* find the sitename */


    //!!! odkomentovati ovo



    if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
                                       sizeof(peer.sin_addr), AF_INET))) {
        if (!nameserver_is_slow)
            perror("gethostbyaddr");
        addr = ntohl(peer.sin_addr.s_addr);
        sprintf(newd->host, "%d.%d.%d.%d", (int) ((addr & 0xFF000000) >> 24),
                (int) ((addr & 0x00FF0000) >> 16), (int) ((addr & 0x0000FF00) >> 8),
                (int) ((addr & 0x000000FF)));
    } else {
        strncpy(newd->host, from->h_name, HOST_LENGTH);
        *(newd->host + HOST_LENGTH) = '\0';
    }

    /* determine if the site is banned */
    if (isbanned(newd->host) == BAN_ALL) {
        close(desc);
        sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
        mudlog(buf2, CMP, LVL_GOD, TRUE);
        DISPOSE(newd);
        return 0;
    }





#if 0
    /* Log new connections - probably unnecessary, but you may want it */
    sprintf(buf2, "New connection from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_GOD, FALSE);
#endif

    /* initialize descriptor data */
    newd->descriptor = desc;
    newd->connected = CON_GET_NAME;
    newd->wait = 1;
    newd->idle_tics = 0;
    newd->peer_port = peer.sin_port;
    newd->output = newd->small_outbuf;
    newd->bufspace = SMALL_BUFSIZE - 1;
    newd->login_time = time(0);
    *newd->output = '\0';
    newd->bufptr = 0;
    newd->has_prompt = 1;       /* prompt is part of greetings */
    CREATE(newd->history, char *, HISTORY_SIZE);

    if (++last_desc == 1000)
        last_desc = 1;
    newd->desc_num = last_desc;

    /* prepend to list */
    newd->next = descriptor_list;
    descriptor_list = newd;
    //    SEND_TO_Q("\r\n\r\nPlease wait", newd);

    gettimeofday( &curr_time, NULL );
    current_time = curr_time.tv_sec;
    sprintf_minutes(buf1, current_time-boot_time_real);
    sprintf( buf, "\n\r%d players on.\n\rSystem started %s ago.\n\rGetting site info ...\r\n", players_online( ), buf1 );
    SEND_TO_Q(buf, newd);
    process_output(newd);
    //strcpy(titlescreen, GREETINGS); /* copy fixed-sized string to largerbuffer */
    //proc_color(titlescreen, C_CMP); /* C_CMP should be 3 */

    //SEND_TO_Q(titlescreen, newd);
    ident_start(newd, peer.sin_addr.s_addr);


    //    if (number(1, 10) < 9)
    //    SEND_TO_Q(GREETINGS, newd);
    //    else
    //      SEND_TO_Q(greet1, newd);

    return 0;
}


void check_idle_passwords(void)
{
    struct descriptor_data *d, *next_d;

    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME)
            continue;
        if (!d->idle_tics) {
            d->idle_tics++;
            continue;
        } else {
            echo_on(d);
            SEND_TO_Q("\r\nTimed out... goodbye.\r\n", d);
            STATE(d) = CON_CLOSE;
        }
    }
}


int             process_output(struct descriptor_data * t)
{
    static char     i[MAX_SOCK_BUF];
    static int      result;
    int clvl=0;
    if(t->character) clvl=COLOR_LEV(t->character);

    /* we may need this \r\n for later -- see below */
    strcpy(i, "\r\n");

    /* now, append the 'real' output */
    strcpy(i + 2, t->output);

    /* if we're in the overflow state, notify the user */
    if (t->bufptr < 0)
        strcat(i, "**OVERFLOW**\0");

    /* add the extra CRLF if the person isn't in compact mode */
    if (!t->connected && t->character && !PRF_FLAGGED(t->character, PRF_COMPACT))
        strcat(i + 2, "\r\n");

    if (t->connected == CON_PLAYING && GET_TERM(t->character) == VT100) {
        proc_color(i,clvl);
        if (t->has_prompt)          /* && !t->connected) */
            result = write_to_descriptor(t->descriptor, i);
        else
            result = write_to_descriptor(t->descriptor, i + 2);
        UpdateScreen(t);

        /*                    {
                               int             door;
                                *bufza = '\0';
                                for (door = 0; door < NUM_OF_DIRS; door++)
                                    if (EXIT(t->character, door) && EXIT(t->character, door)->to_room != NOWHERE && !IS_SET(EXIT(t->character, door)->exit_info, EX_CLOSED))
                                        sprintf(bufza, "%s%c", bufza, UPPER(*dirs[door]));
                                if (GET_POS(t->character) <= POS_SLEEPING)
                                    sprintf(bufza1, "??");
                                else
                                    sprintf(bufza1, "%s", *bufza ? bufza : "None!");
                                strcat(bufza1,"> ");
                            write_to_descriptor(t->descriptor, bufza1);
                            }
        */
        write_to_descriptor(t->descriptor, "> ");
    }
    else
    {
        /* add a prompt */
        strncat(i + 2, make_prompt(t), MAX_PROMPT_LENGTH);


        /* now, send the output.  If this is an 'interruption', use the prepended
         * CRLF, otherwise send the straight output sans CRLF. */
        proc_color(i,clvl);
        if (t->has_prompt && !t->connected)          /* && !t->connected) */
            result = write_to_descriptor(t->descriptor, i);
        else
            result = write_to_descriptor(t->descriptor, i + 2);
    }
    /* handle snooping: prepend "% " and send to snooper */
    if (t->snoop_by) {
        SEND_TO_Q("% ", t->snoop_by);
        SEND_TO_Q(t->output, t->snoop_by);
        SEND_TO_Q("%%", t->snoop_by);
    }
    /* if we were using a large buffer, put the large buffer on the buffer
     * pool and switch back to the small one */
    if (t->large_outbuf) {
        if (number(0, 2)) {     /* Keep it. */
            t->large_outbuf->next = bufpool;
            bufpool = t->large_outbuf;
        } else {                /* Free it. */
            DISPOSE(t->large_outbuf);
            buf_largecount--;
        }

        t->large_outbuf = NULL;
        t->output = t->small_outbuf;
    }
    /* reset total bufspace back to that of a small buffer */
    t->bufspace = SMALL_BUFSIZE - 1;
    t->bufptr = 0;
    *(t->output) = '\0';

    return result;
}



int             write_to_descriptor(int desc, char *txt)
{
    int             total,
    bytes_written;

    total = strlen(txt);

    do {
        if ((bytes_written = write(desc, txt, total)) < 0) {
#ifdef EWOULDBLOCK
            if (errno == EWOULDBLOCK)
                errno = EAGAIN;
#endif
            if (errno != EAGAIN)
                log("process_output: socket write would block, about to close");
            else
                perror("Write to socket");
            return -1;
        } else {
            txt += bytes_written;
            total -= bytes_written;
        }
    } while (total > 0);

    return 0;
}

int flush_input_queue(struct descriptor_data *d)
{
    int a;
    if ((d->input).head)
    {
        while (get_from_q(&d->input, buf, &a));
        send_to_char("&w[ *** Aborting all pending input *** ]&0\r\n", d->character);
    }
    write_to_q("\r\n", &d->input, 0);
    
    return 0;
}



/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int             process_input(struct descriptor_data * t)
{
    int             buf_length,
    bytes_read,
    space_left,
    failed_subst;
    char           *ptr,
    *read_point,
    *write_point,
    *nl_pos = NULL;
    char            tmp[MAX_INPUT_LENGTH + 8];

    /* first, find the point where we left off reading data */
    buf_length = strlen(t->inbuf);
    read_point = t->inbuf + buf_length;
    space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

    do {
        if (space_left <= 0) {
            log("process_input: about to close connection: input overflow");
            return -1;
        }
        if ((bytes_read = read(t->descriptor, read_point, space_left)) < 0) {

#ifdef EWOULDBLOCK
            if (errno == EWOULDBLOCK)
                errno = EAGAIN;
#endif
            if (errno != EAGAIN && errno != EINTR) {
                perror("process_input: about to lose connection");
                return -1;      /* some error condition was encountered on
                                 * read */
            } else
                return 0;       /* the read would have blocked: just means no
                             * data there */
        } else if (bytes_read == 0) {
            log("EOF on socket read (connection broken by peer)");
            return -1;
        }
        /* at this point, we know we got some data from the read */

        *(read_point + bytes_read) = '\0';      /* terminate the string */

        /* search for a newline in the data we just read */
        for (ptr = read_point; *ptr && !nl_pos; ptr++)
            if (ISNEWL(*ptr))
                nl_pos = ptr;

        read_point += bytes_read;
        space_left -= bytes_read;

        /*
         * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
         * causing the MUD to hang when it encounters input not terminated by a
         * newline.  This was causing hangs at the Password: prompt, for example.
         * I attempt to compensate by always returning after the _first_ read, instead
         * of looping forever until a read returns -1.  This simulates non-blocking
         * I/O because the result is we never call read unless we know from select()
         * that data is ready (process_input is only called if select indicates that
         * this descriptor is in the read set).  JE 2/23/95.
         */
        /*#if !defined(POSIX_NONBLOCK_BROKEN)
          } while (nl_pos == NULL);
        #else*/
    } while (0);

    if (nl_pos == NULL)
        return 0;
    /*// #endif*/

    /* okay, at this point we have at least one newline in the string; now we
     * can copy the formatted data to a new array for further processing. */

    read_point = t->inbuf;

    while (nl_pos != NULL) {
        write_point = tmp;
        space_left = MAX_INPUT_LENGTH - 1;

        for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++) {
            if (*ptr == '\b') { /* handle backspacing */
                if (write_point > tmp) {
                    if (*(--write_point) == '$') {
                        write_point--;
                        space_left += 2;
                    } else
                        space_left++;
                }
            } else if (isascii(*ptr) && isprint(*ptr)) {
                if ((*(write_point++) = *ptr) == '$') { /* copy one character */
                    *(write_point++) = '$';     /* if it's a $, double it */
                    space_left -= 2;
                } else
                    space_left--;
            }
        }

        *write_point = '\0';

        if ((space_left <= 0) && (ptr < nl_pos)) {
            char            buffer[MAX_INPUT_LENGTH + 64];

            sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
            if (write_to_descriptor(t->descriptor, buffer) < 0)
                return -1;
        }
        if (t->snoop_by) {
            SEND_TO_Q("% ", t->snoop_by);
            SEND_TO_Q(tmp, t->snoop_by);
            SEND_TO_Q("\r\n", t->snoop_by);
        }
        failed_subst = 0;
        if ((*tmp == '!' || *tmp == ']') && !(*(tmp + 1)))       /* Redo last command. */
            strcpy(tmp, t->last_input);
        else if ((*tmp== '=') && !(*(tmp+1)))
        {
            flush_input_queue(t);
            failed_subst=1;
        }
        else if ((*tmp == '!' || *tmp == ']') && *(tmp + 1)) {
            /*//	    if (!isdigit(*(tmp + 1))) {*/
            char            tmp_switch[MAX_INPUT_LENGTH + 64];
            /* This would be a good spot for snprintf(). */
            sprintf(tmp_switch, "%s%s", t->last_input, tmp + 1);
            strcpy(tmp, tmp_switch);
            /*	    } else {
                            char *commandln = (tmp + 1);
                            int starting_pos = t->history_pos,
                                cnt = (t->history_pos == 0 ? HISTORY_SIZE - 1 : t->history_pos - 1);

                            skip_spaces(&commandln);
                            for (; cnt != starting_pos; cnt--) {
                                if (t->history[cnt] && is_abbrev(commandln, t->history[cnt])) {
                                    strcpy(tmp, t->history[cnt]);
                                    log(tmp);
                                    break;
                                }
                                if (cnt == 0)	
                                    cnt = HISTORY_SIZE;
                            }

                        }*/



        } else if (*tmp == '^') {
            if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
                strcpy(t->last_input, tmp);

        } else {
            strcpy(t->last_input, tmp);
            /*	    if (t->history[t->history_pos])
                            DISPOSE(t->history[t->history_pos]);	
                        t->history[t->history_pos] = str_dup(tmp);	
                        if (++t->history_pos >= HISTORY_SIZE)	
                            t->history_pos = 0;*/
        }


        if (!failed_subst)
            write_to_q(tmp, &t->input, 0);

        /* find the end of this line */
        while (ISNEWL(*nl_pos))
            nl_pos++;

        /* see if there's another newline in the input buffer */
        read_point = ptr = nl_pos;
        for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
            if (ISNEWL(*ptr))
                nl_pos = ptr;
    }

    /* now move the rest of the buffer up to the beginning for the next pass */
    write_point = t->inbuf;
    while (*read_point)
        *(write_point++) = *(read_point++);
    *write_point = '\0';

    return 1;
}



/*
 * perform substitution for the '^..^' csh-esque syntax
 * orig is the orig string (i.e. the one being modified.
 * subst contains the substition string, i.e. "^telm^tell"
 */
int             perform_subst(struct descriptor_data * t, char *orig, char *subst)
{
    char            new[MAX_INPUT_LENGTH + 5];

    char           *first,
    *second,
    *strpos;

    /* first is the position of the beginning of the first string (the one to
     * be replaced */
    first = subst + 1;

    /* now find the second '^' */
    if (!(second = strchr(first, '^'))) {
        SEND_TO_Q("Invalid substitution.\r\n", t);
        return 1;
    }
    /* terminate "first" at the position of the '^' and make 'second' point
     * to the beginning of the second string */
    *(second++) = '\0';

    /* now, see if the contents of the first string appear in the original */
    if (!(strpos = strstr(orig, first))) {
        SEND_TO_Q("Invalid substitution.\r\n", t);
        return 1;
    }
    /* now, we construct the new string for output. */

    /* first, everything in the original, up to the string to be replaced */
    strncpy(new, orig, (strpos - orig));
    new[(strpos - orig)] = '\0';

    /* now, the replacement string */
    strncat(new, second, (MAX_INPUT_LENGTH - strlen(new) - 1));

    /* now, if there's anything left in the original after the string to
     * replaced, copy that too. */
    if (((strpos - orig) + strlen(first)) < strlen(orig))
        strncat(new, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(new) - 1));

    /* terminate the string in case of an overflow from strncat */
    new[MAX_INPUT_LENGTH - 1] = '\0';
    strcpy(subst, new);

    return 0;
}



void            close_socket(struct descriptor_data * d)
{
    struct descriptor_data *temp;
    char            buf[128];
    long            target_idnum = -1;

    close(d->descriptor);
    flush_queues(d);

    if (d->ident_sock != -1)
        close(d->ident_sock);

    /* Forget snooping */
    if (d->snooping)
        d->snooping->snoop_by = NULL;

    if (d->snoop_by) {
        SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
        d->snoop_by->snooping = NULL;
    }


    /* . Kill any OLC stuff . */
    switch (d->connected) {
    case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
        cleanup_olc(d, CLEANUP_ALL);
    default:
        break;
    }

    if (d->character) {
        target_idnum = GET_IDNUM(d->character);
        if (d->character==auction.seller)
        {            
            obj_to_char(auction.obj, d->character);                     
            AUC_OUT("Auction canceled, seller left the game.");
            auction_reset();
        }
        if (d->character==auction.bidder)
        {
            auction.bidder=NULL;
            auction.ticks=AUC_BID;
        }

        add_llog_entry(d->character,LAST_DISCONNECT);

        if (!IS_NPC(d->character) && PLR_FLAGGED(d->character, PLR_MAILING) && d->str) {
            if (*(d->str))
                DISPOSE(*(d->str));
            DISPOSE(d->str);
        }

        /* added to free up temporary editing constructs */
        /*    if (AFF2_FLAGGED(d->character,AFF2_WRATH) && WRATHOBJ(d->character))
        //      {extract_obj(WRATHOBJ(d->character));WRATHOBJ(d->character)=NULL;}*/
        //REMOVE_BIT(AFF_FLAGS(d->character), AFF_VISITING);
        if (d->connected == CON_PLAYING) {
            if (d->character->in_room>1)
                save_char(d->character, d->character->in_room);
            act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
            sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
            mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            d->character->desc = NULL;
        } else {
            sprintf(buf, "Losing player: %s.",
                    GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
            mudlog(buf, CMP, LVL_IMMORT, TRUE);
            free_char(d->character);
        }
    } else
        mudlog("Losing descriptor without char.", CMP, LVL_IMMORT, TRUE);

    /* JE 2/22/95 -- part of my enending quest to make switch stable */
    if (d->original && d->original->desc)
        d->original->desc = NULL;

    REMOVE_FROM_LIST(d, descriptor_list, next);
    /* Clear the command history. */
    if (d->history) {
        int             cnt;
        for (cnt = 0; cnt < HISTORY_SIZE; cnt++)
            if (d->history[cnt])
                DISPOSE(d->history[cnt]);
        DISPOSE(d->history);
    }
    if (d->showstr_head)
        DISPOSE(d->showstr_head);

    DISPOSE(d);

    /* kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping. */
    /*  if (target_idnum >= 0) {
        for (temp = descriptor_list; temp; temp = next_d) {
          next_d = temp->next;
          if (temp->character && GET_IDNUM(temp->character) == target_idnum)
            close_socket(temp);
        }
      }*/
}


/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */
#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void            nonblock(int s)
{
    int             flags;

    flags = fcntl(s, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(s, F_SETFL, flags) < 0) {
        perror("Fatal error executing nonblock (comm.c)");
        exit(1);
    }
}




/* ******************************************************************
*  signal-handling functions (formerly signals.c)                   *
****************************************************************** */


void            checkpointing()
{
    if (!tics) {
        log("SYSERR: CHECKPOINT shutdown: tics not updated");
        panic();
        abort();
    } else
        tics = 0;
}

void            shuttingdown()
{             
    FILE *fl;
    circle_shutdown = circle_reboot = (1801 RL_SEC);
    send_to_all("&W[Info]:  Auto-Reboot sequence initiated.&0\r\n\r\n");
    
     if (!(fl = fopen(LAST_FILE, "rb"))) {
        if (errno != ENOENT) {  /* if it fails but NOT because of no file */
            sprintf(buf1, "SYSERR: deleting LAST file %s (1)", LAST_FILE);
            perror(buf1);
        }
        return;
    }
    fclose(fl);

    if (unlink(LAST_FILE) < 0) {
        if (errno != ENOENT) {  /* if it fails, NOT because of no file */
            sprintf(buf1, "SYSERR: deleting LAS file %s (2)", LAST_FILE);
            perror(buf1);
        }
    }
}

void            reread_wizlists()
{
    void            reboot_wizlists(void);

    mudlog("Signal received - rereading wizlists.", CMP, LVL_IMMORT, TRUE);
    reboot_wizlists();
}


void            unrestrict_game()
{
    extern struct ban_list_element *ban_list;
    extern int      num_invalid;

    mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
           BRF, LVL_IMMORT, TRUE);
    ban_list = NULL;
    restrict1 = 0;
    num_invalid = 0;
}


void            hupsig()
{
    log("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
    Crash_save_all();
    save_mobkills();
    save_topdam();
    save_tophurt();
    write_mud_date_to_file();
    exit(0);                    /* perhaps something more elegant should
                                 * substituted */
}


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#if defined(NeXT) || defined(sun386)
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc        *my_signal(int signo, sigfunc * func)
{
    struct sigaction act,
                oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;       /* SunOS */
#endif

    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;

    return oact.sa_handler;
}

#endif          /* NeXT */
extern char last_command[10000];
int global_save=0;
void panic()
{
    FILE *f;
    struct descriptor_data *i;
    time_t          ct;
    char           *tmstr;
    f=fopen("panic.txt", "a");
    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    if (f){
        fprintf(f, "%-15.15s :: %s\n", tmstr + 4,last_command);
        fclose(f);
    }
    logs("CRASH ERROR: %s", last_command);
    global_save=1;
    Crash_save_all();
    save_mobkills();
    save_topdam();
    save_tophurt();
    for (i = descriptor_list; i; i = i->next)
        if (!i->connected)
            write_to_descriptor(i->descriptor, "\r\n\r\n===============================================================================\r\nLands of Myst detected an error that would normally crash the MUD.\r\nYour character will now be saved and the MUD will reboot automatically.\r\n===============================================================================\r\n\r\n");
    write_mud_date_to_file();  
    
   

  #ifdef AFRO      
  
    system( "tail panic.txt | /usr/lib/sendmail -t tomcat@galeb.etf.bg.ac.yu" );
  #endif
  
    sleep(1);

    if (ff) fclose(ff);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGKILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);

//    exit(1);

}

void            signal_setup(void)
{
    struct itimerval itime;
    struct timeval  interval;

    /* user signal 1: reread wizlists.  Used by autowiz system. */
    my_signal(SIGUSR1, (sigfunc *) reread_wizlists);

    /* user signal 2: unrestrict game.  Used for emergencies if you lock
     * yourself out of the MUD somehow.  (Duh...) */
    my_signal(SIGUSR2, (sigfunc *) unrestrict_game);

    /* set up the deadlock-protection so that the MUD aborts itself if it
     * gets caught in an infinite loop for more than 2 minutes */
    interval.tv_sec = 15;
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, NULL);
    my_signal(SIGVTALRM ,(sigfunc *) checkpointing);
    /* just to be on the safe side: */
    my_signal(SIGHUP, (sigfunc *) hupsig);
    my_signal(SIGINT, (sigfunc *) hupsig);
    my_signal(SIGTERM, (sigfunc *) hupsig);
    my_signal(SIGPIPE, SIG_IGN);
    my_signal(SIGALRM, (sigfunc *) shuttingdown);
    signal(SIGSEGV, panic);
    signal(SIGBUS, panic);
    signal(SIGABRT, panic);
    signal(SIGKILL, panic);
    signal(SIGFPE, panic);

}



/* ****************************************************************
*       Public routines for system-to-player-communication        *
*******************************************************************/
struct char_data *is_playing(char *vict_name)
{
    extern struct descriptor_data *descriptor_list;
    struct descriptor_data *i,
                *next_i;

for (i = descriptor_list; i; i = next_i) {
        next_i = i->next;
        if (i->connected == CON_PLAYING && !strcmp(i->character->player.name, CAP(vict_name)))
            return i->character;
    }
    return NULL;
}






void            send_to_char(char *messg, struct char_data * ch)
{
    if (ch->desc && messg)
        SEND_TO_Q(messg, ch->desc);
}


void            send_to_all(char *messg)
{
    struct descriptor_data *i;

    if (messg)
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected)
                SEND_TO_Q(messg, i);
}


void            send_to_outdoor(char *messg)
{
    struct descriptor_data *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next)
        if (!i->connected && i->character && AWAKE(i->character) &&
                OUTSIDE(i->character))
            SEND_TO_Q(messg, i);
}

void            send_to_channel(char *messg, int channel)
{
    struct descriptor_data *i;
    char            buf[256];

    if (!messg || !*messg)
        return;

    sprintf(buf, "Channel %d: %s", channel, messg);

    for (i = descriptor_list; i; i = i->next)
        if (!i->connected && i->character && AWAKE(i->character) &&
                IS_SET(GET_CHANNEL(i->character), 1 << (channel - 1)))
            SEND_TO_Q(buf, i);
}

void            send_to_room(char *messg, int room)
{
    struct char_data *i;

    if (messg)
        for (i = world[room].people; i; i = i->next_in_room)

            if (i->desc && AWAKE(i))
                SEND_TO_Q(messg, i->desc);
    for (i = world[room].listeners; i; i = i->next_listener)
        if (i->desc && AWAKE(i)) {
            sprintf(buf, "(listen) %s\r\n", messg);
            SEND_TO_Q(buf, i->desc);
        }
    /*    if (ROOM_FLAGGED(room, ROOM_BROADCAST)) {
            if (world[room].broad->channel > 0);
            send_to_channel(messg, world[room].broad->channel);
            if (real_room(world[room].broad->targ1) > 0)
                send_to_room(messg, real_room(world[room].broad->targ1));
            if (real_room(world[room].broad->targ2) > 0)
                send_to_room(messg, real_room(world[room].broad->targ2));
        }*/  

}

char           *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);


/* higher-level communication: the act() function */
char           *perform_act(char *orig, struct char_data * ch, struct obj_data * obj,
                            void *vict_obj, struct char_data * to)
{
    register char  *i,
    *buf;
    static char     lbuf[MAX_STRING_LENGTH];

    buf = lbuf;
    i = NULL;

    for (;;) {
        if (*orig == '$') {
            switch (*(++orig)) {
            case 'n':
                i = PERS(ch, to);
                break;
            case 'N':
                CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
                break;
            case 'm':
                i = HMHR(ch);
                break;
            case 'M':
                CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
                break;
            case 's':
                i = HSHR(ch);
                break;
            case 'S':
                CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
                break;
            case 'e':
                i = HSSH(ch);
                break;
            case 'E':
                CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
                break;
            case 'o':
                CHECK_NULL(obj, OBJN(obj, to));
                break;
            case 'O':
                CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
                break;
            case 'p':
                CHECK_NULL(obj, OBJS(obj, to));
                break;
            case 'P':
                CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
                break;
            case 'a':
                CHECK_NULL(obj, SANA(obj));
                break;
            case 'A':
                CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
                break;
            case 'T':
                CHECK_NULL(vict_obj, (char *) vict_obj);
                break;
            case 'F':
                CHECK_NULL(vict_obj, fname((char *) vict_obj));
                break;
            case '$':
                i = "$";
                break;
            default:
                log("SYSERR: Illegal $-code to act():");
                strcpy(buf1, "SYSERR: ");
                strcat(buf1, orig);
                log(buf1);
                break;
            }
            while ((*buf = *(i++)))
                buf++;
            orig++;
        } else if (!(*(buf++) = *(orig++)))
            break;
    }

    *(--buf) = '\r';
    *(++buf) = '\n';
    *(++buf) = '\0';
    if (to->desc)
        SEND_TO_Q(super_silent?linewrap(CAP(lbuf), IS_NPC(to) ? 79 : GET_LINEWRAP(to)):lbuf, to->desc);
    //SEND_TO_Q(CAP(lbuf), to->desc);
    if (MOBTrigger && IS_NPC(to) && (mob_index[to->nr].progtypes & ACT_PROG))
        mprog_act_trigger(lbuf, to, ch, obj, vict_obj);
    return &lbuf[0];
}


char     actbuf[MAX_STRING_LENGTH];
char           *act_string(char *orig, struct char_data * ch, struct obj_data * obj,
                           void *vict_obj, struct char_data * to)
{
    register char  *i,
    *buf;


    buf = actbuf;
    i = NULL;

    for (;;) {
        if (*orig == '$') {
            switch (*(++orig)) {
            case 'n':
                i = PERS(ch, to);
                break;
            case 'N':
                CHECK_NULL(vict_obj, PERS((struct char_data *) vict_obj, to));
                break;
            case 'm':
                i = HMHR(ch);
                break;
            case 'M':
                CHECK_NULL(vict_obj, HMHR((struct char_data *) vict_obj));
                break;
            case 's':
                i = HSHR(ch);
                break;
            case 'S':
                CHECK_NULL(vict_obj, HSHR((struct char_data *) vict_obj));
                break;
            case 'e':
                i = HSSH(ch);
                break;
            case 'E':
                CHECK_NULL(vict_obj, HSSH((struct char_data *) vict_obj));
                break;
            case 'o':
                CHECK_NULL(obj, OBJN(obj, to));
                break;
            case 'O':
                CHECK_NULL(vict_obj, OBJN((struct obj_data *) vict_obj, to));
                break;
            case 'p':
                CHECK_NULL(obj, OBJS(obj, to));
                break;
            case 'P':
                CHECK_NULL(vict_obj, OBJS((struct obj_data *) vict_obj, to));
                break;
            case 'a':
                CHECK_NULL(obj, SANA(obj));
                break;
            case 'A':
                CHECK_NULL(vict_obj, SANA((struct obj_data *) vict_obj));
                break;
            case 'T':
                CHECK_NULL(vict_obj, (char *) vict_obj);
                break;
            case 'F':
                CHECK_NULL(vict_obj, fname((char *) vict_obj));
                break;
            case '$':
                i = "$";
                break;
            default:
                log("SYSERR: Illegal $-code to act():");
                strcpy(buf1, "SYSERR: ");
                strcat(buf1, orig);
                log(buf1);
                break;
            }
            while ((*buf = *(i++)))
                buf++;
            orig++;
        } else if (!(*(buf++) = *(orig++)))
            break;
    }

    *(--buf) = '\r';
    *(++buf) = '\n';
    *(++buf) = '\0';
    actbuf[0]=UPPER(actbuf[0]);
    return &actbuf[0];
}



/* #define SENDOK(ch) ((ch)->desc && (AWAKE(ch) || sleep) && \
                    !PLR_FLAGGED((ch), PLR_WRITING) && \
                    !PLR_FLAGGED((ch), PLR_EDITING))
*/
#define SENDOK(ch) ((AWAKE(ch) || sleep) && \
		    !PLR_FLAGGED((ch), PLR_WRITING) && \
		    !PLR_FLAGGED((ch), PLR_EDITING))

void            act(char *str, int hide_invisible, struct char_data * ch,
                    struct obj_data * obj, void *vict_obj, int type)
{
    struct char_data *to = NULL;
    int             sleep,
    chat_check;
    struct descriptor_data *i; /* add this line */

    if (ch && IS_SUPERMOB(ch) && type!=TO_CHAR && super_silent)
        return;

    if (!str || !*str) {
        MOBTrigger = TRUE;
        return;
    }
    /* Warning: the following TO_SLEEP code is a hack.
     * 
     * I wanted to be able to tell act to deliver a message regardless of sleep
     * without adding an additional argument.  TO_SLEEP is 128 (a single bit
     * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
     * command.  It's not legal to combine TO_x's with each other otherwise.
     * TO_SLEEP only works because its value "happens to be" a single bit; do
     * not change it to something else.  In short, it is a hack. */

    /* check if TO_SLEEP is there, and remove it if it is. */
    if ((sleep = (type & TO_SLEEP)))
        type &= ~TO_SLEEP;

    if ((chat_check = (type & CHAT_CHECK)))
        type &= ~CHAT_CHECK;


    if (type == TO_CHAR) {
        if (ch && SENDOK(ch))
            perform_act(str, ch, obj, vict_obj, ch);
        MOBTrigger = TRUE;
        return;
    }
    if (type == TO_VICT) {
        if ((to = (struct char_data *) vict_obj) && SENDOK(to)) {
            char           *text = perform_act(str, ch, obj, vict_obj, to);
            if (chat_check)
                chatperform(to, ch, text);
        }
        MOBTrigger = TRUE;
        return;
    }


    /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

    /* add from here to GMOTE_END */
    if (type == TO_GMOTE) {
        for (i = descriptor_list; i; i = i->next) {
            if (!i->connected && i->character &&
                    !PRF_FLAGGED(i->character, PRF_NOGOSS) &&
                    !PLR_FLAGGED(i->character, PLR_WRITING) &&
                    !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

                //send_to_char(CCYEL(i->character, C_NRM), i->character);
                perform_act(str, ch, obj, vict_obj, i->character);
                // send_to_char(CCNRM(i->character, C_NRM), i->character);
            }
        }
        return;
    }
    /* GMOTE_END */

    if (ch && ch->in_room != NOWHERE)
        to = world[ch->in_room].people;
    else if (obj && obj->in_room != NOWHERE && !PURGED(obj))
        to = world[obj->in_room].people;
    else {
        char actbuf[1000];
        sprintf(actbuf, "SYSERR: no valid target to act(%s, ...) !", str);
        log(actbuf);
        return;
    }

    if ( MOBTrigger && type !=TO_CHAR && type !=TO_VICT  && to )
    {
        OBJ_DATA *to_obj;
        char *txt;

        txt = act_string(str, ch, obj, vict_obj, to);
        if ( HAS_ROOM_PROG(&world[to->in_room], ACT_PROG) )
            rprog_act_trigger(txt, &world[to->in_room], ch, obj, vict_obj);
        for ( to_obj = world[to->in_room].contents; to_obj;
                to_obj = to_obj->next_content )
            if ( HAS_OBJ_PROG(to_obj, ACT_PROG) )
                oprog_act_trigger(txt, to_obj, ch, obj, vict_obj);
    }


    for (; to; to = to->next_in_room)
        if (SENDOK(to) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
                (to != ch) && (type == TO_ROOM || (to != (struct char_data *) vict_obj)))
            perform_act(str, ch, obj, vict_obj, to);

    if (ch && ch->in_room>0)
        for (to = world[ch->in_room].listeners; to; to = to->next_listener)
            if (to->desc) {
                if (SENDOK(to) && !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
                        (to != ch) && (type == TO_ROOM || (to != (struct char_data *) vict_obj)))
                {
                    /* handle snooping: prepend "% " and send to snooper */
                    SEND_TO_Q("(listen) ", to->desc);
                    perform_act(str, ch, obj, vict_obj, to);
                    improve_skill(to, SKILL_EAVESDROP, 1);
                }
            }

    MOBTrigger = TRUE;
}

char            affbuf[50],
usbuf[255],
usbuf1[255],
usbuf2[255];
int             size,
percent,
percent1;
struct char_data *tank;

void            UpdateScreen(struct descriptor_data * t)
{struct char_data *ch=t->character;
    size = GET_TERM_SIZE(ch);

    if (size <= 4)
        return;

    sprintf(usbuf, VT_CURSAVE);
    write_to_descriptor(t->descriptor, usbuf);

    // sprintf(usbuf, "%s%s%s%s%s%s%s%s%s%s%s%s", VT_CURSPOS, "           ", VT_CURSPOS, "%4d (%4d)", VT_CURSPOS, "           ", VT_CURSPOS, "%4d (%4d)", VT_CURSPOS, "           ", VT_CURSPOS, "%4d (%4d)");
    // sprintf(usbuf1, usbuf, size - 3, 7, size - 3, 7, GET_HIT(ch), GET_MAX_HIT(ch), size - 2, 7, size - 2, 7, GET_MANA(ch), GET_MAX_MANA(ch), size - 1, 7, size - 1, 7, GET_MOVE(ch), GET_MAX_MOVE(ch));
    sprintf(usbuf, "%s%s%s%s%s%s", VT_CURSPOS, "%4d (%4d)",  VT_CURSPOS, "%4d (%4d)", VT_CURSPOS, "%4d (%4d)");
    sprintf(usbuf1, usbuf,  size - 3, 7, GET_HIT(ch), GET_MAX_HIT(ch), size - 2, 7, GET_MANA(ch), GET_MAX_MANA(ch),  size-1, 7, GET_MOVE(ch), GET_MAX_MOVE(ch));
    if (strcmp(t->p1,usbuf1))
    {
        strcpy(t->p1,usbuf1);
        write_to_descriptor(t->descriptor, usbuf1);
    }


    //    sprintf(usbuf, "%s%s%s%s%s%s%s%s%s%s%s%s",VT_CURSPOS, "              ", VT_CURSPOS, "%s", VT_CURSPOS, "              ", VT_CURSPOS, "%s", VT_CURSPOS, "              ", VT_CURSPOS, "%s");
    sprintf(usbuf, "%s%s%s%s%s%s", VT_CURSPOS, "%-15s",  VT_CURSPOS, "%-15s",  VT_CURSPOS, "%-15s");
    if (!FIGHTING(ch))
    {
        *affbuf='\0';
        if (AFF_FLAGGED(ch, AFF_SANCTUARY) || AFF_FLAGGED(ch, AFF_HIDE) ||
                AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_SILVER) || AFF3_FLAGGED(ch, AFF3_QUAD) || AFF2_FLAGGED(ch, AFF2_PRISM) || AFF2_FLAGGED(ch, AFF2_ULTRA))
        {
            if (AFF3_FLAGGED(ch, AFF3_QUAD))
                strcat(affbuf, "Q");
            if (AFF_FLAGGED(ch, AFF_SANCTUARY))
                strcat(affbuf, "S");
            if (AFF2_FLAGGED(ch, AFF2_PRISM))
                strcat(affbuf, "P");
            if (AFF2_FLAGGED(ch, AFF2_ULTRA))
                strcat(affbuf, "U");
            if (AFF_FLAGGED(ch, AFF_SILVER))
                strcat(affbuf, "A");
            if (AFF_FLAGGED(ch, AFF_HIDE))
                strcat(affbuf, "H");
            if (AFF_FLAGGED(ch, AFF_INVISIBLE))
                strcat(affbuf, "I");
        }

        sprintf(usbuf1, usbuf,  size - 3, 33, affbuf, size - 2, 33, "-", size - 1, 33, "-");
    }
    else {
        if (GET_MAX_HIT(ch))
            percent = (int) 100 *GET_HIT(FIGHTING(ch)) / GET_MAX_HIT(FIGHTING(ch));
        else
            percent = -1;

        tank = FIGHTING(FIGHTING(ch));
        if (tank && is_same_group(ch, tank)) {
            if (GET_MAX_HIT(tank))
                percent1 = (int) 100 *GET_HIT(tank) / GET_MAX_HIT(tank);
            else
                percent1 = -1;
        }
        *affbuf='\0';
        if (AFF_FLAGGED(ch, AFF_SANCTUARY) || AFF_FLAGGED(ch, AFF_HIDE) ||
                AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_SILVER) || AFF3_FLAGGED(ch, AFF3_QUAD) || AFF2_FLAGGED(ch, AFF2_PRISM) || AFF2_FLAGGED(ch, AFF2_ULTRA))
        {
            if (AFF3_FLAGGED(ch, AFF3_QUAD))
                strcat(affbuf, "Q");
            if (AFF_FLAGGED(ch, AFF_SANCTUARY))
                strcat(affbuf, "S");
            if (AFF2_FLAGGED(ch, AFF2_PRISM))
                strcat(affbuf, "P");
            if (AFF2_FLAGGED(ch, AFF2_ULTRA))
                strcat(affbuf, "U");
            if (AFF_FLAGGED(ch, AFF_SILVER))
                strcat(affbuf, "A");
            if (AFF_FLAGGED(ch, AFF_HIDE))
                strcat(affbuf, "H");
            if (AFF_FLAGGED(ch, AFF_INVISIBLE))
                strcat(affbuf, "I");
        }

        sprintf(usbuf1, usbuf,  size - 3, 33,affbuf, size - 2, 33, show_char_cond(percent),  size - 1, 33, show_char_cond(percent1));
    }
    if (strcmp(t->p2,usbuf1))
    {
        proc_color(usbuf1, COLOR_LEV(t->character));
        write_to_descriptor(t->descriptor, usbuf1);
        strcpy(t->p2,usbuf1);
    }

    //    sprintf(usbuf, "%s%s%s%s%s%s%s%s%s%s%s%s", VT_CURSPOS, "            ", VT_CURSPOS, "%d", VT_CURSPOS, "            ", VT_CURSPOS, "%d", VT_CURSPOS, "            ", VT_CURSPOS, "%s");
    sprintf(usbuf, "%s%s%s%s%s%s",  VT_CURSPOS, "%-15d",  VT_CURSPOS, "%-15d",  VT_CURSPOS, "%-15s");
    //sprintf(usbuf1, usbuf,  size - 3, 66, total_exp(GET_LEVEL(ch)) - GET_EXP(ch),  size - 2, 66, GET_GOLD(ch), size - 1, 66,  align_types[get_alignment_type(GET_ALIGNMENT(ch))]);
    sprintf(usbuf1, usbuf,  size - 3, 66, LEVELEXP(ch) - GET_EXP(ch),  size - 2, 66, GET_GOLD(ch), size - 1, 66,  align_types[get_alignment_type(GET_ALIGNMENT(ch))]);
    if (strcmp(t->p3,usbuf1))
    {
        write_to_descriptor(t->descriptor, usbuf1);
        strcpy(t->p3,usbuf1);
    }


    sprintf(usbuf, VT_CURREST);
    write_to_descriptor(t->descriptor, usbuf);

}


void            InitScreen(struct char_data * ch)
{
    char            buf[255],
    buf2[255];
    int             size,
    percent;
    struct char_data *tank;

    size = GET_TERM_SIZE(ch);
    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, VT_MARGSET, 0, size - 5);
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 4, 1);
    send_to_char(buf, ch);
    sprintf(buf, "-=============================================================================-");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 3, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Hit : ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 3, 24);
    send_to_char(buf, ch);
    sprintf(buf, "Affects: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 3, 52);
    send_to_char(buf, ch);
    sprintf(buf, "Exp to level:  ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 2, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Mana: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 2, 60);
    send_to_char(buf, ch);
    sprintf(buf, "Gold: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 2, 23);
    send_to_char(buf, ch);
    sprintf(buf, "Opponent: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 1, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Move: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 1, 25);
    send_to_char(buf, ch);
    sprintf(buf, "Tanker: ");
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 1, 55);
    send_to_char(buf, ch);
    sprintf(buf, "Alignment: ");
    send_to_char(buf, ch);

    /*
     ch->last.mana = GET_MANA(ch);
     ch->last.mmana = GET_MAX_MANA(ch);
     ch->last.hit = GET_HIT(ch);
     ch->last.mhit = GET_MAX_HIT(ch);
     ch->last.move = GET_MOVE(ch);
     ch->last.mmove = GET_MAX_MOVE(ch);
     ch->last.exp = GET_EXP(ch);
     ch->last.gold = GET_GOLD(ch);
     */
    /* Update all of the info parts */
    sprintf(buf, VT_CURSPOS, size - 3, 7);
    send_to_char(buf, ch);
    sprintf(buf, "%d (%d)", (int) GET_HIT(ch), (int) GET_MAX_HIT(ch));
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 2, 7);
    send_to_char(buf, ch);
    sprintf(buf, "%d (%d)", (int) GET_MANA(ch), (int) GET_MAX_MANA(ch));
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, size - 1, 7);
    send_to_char(buf, ch);
    sprintf(buf, "%d (%d)",(int)  GET_MOVE(ch), (int) GET_MAX_MOVE(ch));
    send_to_char(buf, ch);
    *buf2='\0';
    if (AFF_FLAGGED(ch, AFF_SANCTUARY) || AFF_FLAGGED(ch, AFF_HIDE) ||
            AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_SILVER) || AFF3_FLAGGED(ch, AFF3_QUAD) || AFF2_FLAGGED(ch, AFF2_PRISM) || AFF2_FLAGGED(ch, AFF2_ULTRA))
    {
        if (AFF3_FLAGGED(ch, AFF3_QUAD))
            strcat(buf2, "Q");
        if (AFF_FLAGGED(ch, AFF_SANCTUARY))
            strcat(buf2, "S");
        if (AFF2_FLAGGED(ch, AFF2_PRISM))
            strcat(buf2, "P");
        if (AFF2_FLAGGED(ch, AFF2_ULTRA))
            strcat(buf2, "U");
        if (AFF_FLAGGED(ch, AFF_SILVER))
            strcat(buf2, "A");
        if (AFF_FLAGGED(ch, AFF_HIDE))
            strcat(buf2, "H");
        if (AFF_FLAGGED(ch, AFF_INVISIBLE))
            strcat(buf2, "I");
    }


    sprintf(buf, VT_CURSPOS, size - 3, 33);
    send_to_char(buf, ch);
    sprintf(buf, "%s", buf2);
    send_to_char(buf, ch);

    if (!FIGHTING(ch))
        strcpy(buf2, "none");
    else {
        if (GET_MAX_HIT(ch))
            percent = (int) 100 *GET_HIT(FIGHTING(ch)) / GET_MAX_HIT(FIGHTING(ch));
        else
            percent = -1;
        sprintf(buf2, "%s", show_char_cond(percent));
    }

    sprintf(buf, VT_CURSPOS, size - 2, 33);
    send_to_char(buf, ch);
    sprintf(buf, "%s", buf2);
    send_to_char(buf, ch);

    sprintf(buf2, "none");
    if (FIGHTING(ch)) {
        tank = FIGHTING(FIGHTING(ch));
        if (tank && /* (tank != d->character) && */ is_same_group(ch, tank)) {
            if (GET_MAX_HIT(tank))
                percent = (int) 100 *GET_HIT(tank) / GET_MAX_HIT(tank);
            else
                percent = -1;
            sprintf(buf2, "%s", show_char_cond(percent));
        }
    }
    sprintf(buf, VT_CURSPOS, size - 1, 33);
    send_to_char(buf, ch);
    sprintf(buf, "%s", buf2);
    send_to_char(buf, ch);


    sprintf(buf, VT_CURSPOS, size - 3, 66);
    send_to_char(buf, ch);
    //sprintf(buf, "%d", total_exp(GET_LEVEL(ch)) - GET_EXP(ch));
    sprintf(buf, "%d", LEVELEXP(ch) - GET_EXP(ch));
    send_to_char(buf, ch);

    sprintf(buf, VT_CURSPOS, size - 2, 66);
    send_to_char(buf, ch);
    sprintf(buf, "%d", GET_GOLD(ch));
    send_to_char(buf, ch);

    /*sprintf(buf, VT_CURSPOS, size - 1, 66);
    send_to_char(buf, ch);
    sprintf(buf, "%d", align_types[get_alignment_type(GET_ALIGNMENT(ch))]);
    send_to_char(buf, ch);
    */
    sprintf(buf, VT_CURSPOS, 0, 0);
    send_to_char(buf, ch);

}
