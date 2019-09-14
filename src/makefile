
SHELL    = /bin/sh
INCLUDES = -I.
BINDIR   = ../bin


# for AIX
# CC = gcc
# for IRIX
CC = gcc

CFLAGS   = -O3  -Wno-missing-noreturn -Wno-implicit-function-declaration -Wno-pointer-bool-conversion -Wno-format -Wno-parentheses -Wno-unused-value -Wno-format-security -Wno-incompatible-library-redeclaration -Wno-return-type -Wno-constant-logical-operand -Wno-absolute-value -Wno-unsequenced -Wno-comment -Wno-implicit-int  -Wno-memset-transposed-args -Wno-tautological-constant-out-of-range-compare -Wno-pointer-sign -Wno-empty-body -Wno-non-literal-null-conversion -Wno-trigraphs -Wno-int-to-pointer-cast -Wno-array-bounds
#CFLAGS   = -ggdb3
#CFLAGS   = -DADV_MEMORY

# for AIX
#CFLAGS = -o2 -Wall
# for IR
# CFLAGS =  -O2 -OPT:fold_arith_limit=3395



# LIBS     = -lsocket -lnsl

# SVR4 or Solaris
# LIBS = -lsocket -lnsl
# IRIX
 LIBS = 


DEFS     =


# flags for profiling (see hacker.doc for more information)
#
# (just add this to the implicit rule below if you want profiling)



# implicit rule for building object files

#%.o : %.c
#	$(CC) -c $(CFLAGS) $(DEFS) $(INCLUDES) $< 


##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

HDRS = 	boards.h				\
	comm.h					\
	db.h					\
	handler.h				\
	house.h					\
	interpreter.h				\
	mail.h					\
	olc.h					\
	screen.h				\
	shop.h					\
	spells.h				\
	structs.h				\
	utils.h \
	arena.h \
	akey.hpp allkey.hpp chatmain.hpp eliza.hpp chatlink.h \
	clan.h \
	ident.h \
	ego.h \
	events.h queue.h \
	newquest.h \


SRCS =  act.comm.c				\
	act.informative.c			\
	act.movement.c				\
	act.obj.c				\
	act.offensive.c				\
	act.other.c				\
	act.social.c				\
	act.wizard.c				\
	ban.c					\
	boards.c				\
	class.c					\
	comm.c					\
	config.c				\
	constants.c				\
	db.c					\
	fight.c					\
	graph.c					\
	handler.c				\
	house.c					\
	interpreter.c				\
	limits.c				\
	magic.c					\
	mail.c					\
	mobact.c				\
	modify.c				\
	objsave.c				\
	olc.c					\
	random.c				\
	shop.c					\
	spec_assign.c				\
	spec_procs.c				\
	spell_parser.c				\
	spells.c				\
	utils.c					\
	weather.c				\
	alias.c					\
	mobcmd.c				\
	mobprog.c				\
	oedit.c					\
	redit.c					\
	zedit.c					\
        medit.c					\
        sedit.c					\
	quest.c					\
	teleport.c		\
	castle.c \
	arena.c \
	akey.C allkey.C chatmain.C eliza.C \
	clan.c \
	log.c \
        color.c \
        ident.c \
        ego.c \
        events.c queue.c \
        eventprocs.c \
        memory.c \
        newquest.c \
        trap.c        \
        ackmapper.c \
        soundex.c


OBJS = 	comm.o 					\
	act.comm.o 				\
	act.informative.o 			\
	act.movement.o 				\
	act.obj.o				\
	act.offensive.o 			\
	act.other.o 				\
	act.social.o 				\
	act.wizard.o 				\
	ban.o 					\
	boards.o				\
	class.o 				\
	config.o 				\
	constants.o 				\
	db.o 					\
	fight.o 				\
	graph.o 				\
	handler.o				\
	house.o 				\
	interpreter.o 				\
	limits.o 				\
	magic.o 				\
	mail.o 					\
	mobact.o 				\
	modify.o				\
	objsave.o 				\
	olc.o 					\
	random.o 				\
	shop.o 					\
	spec_assign.o 				\
	spec_procs.o				\
	spell_parser.o 				\
	spells.o 				\
	utils.o 				\
	weather.o				\
	alias.o					\
	mobcmd.o				\
	mobprog.o				\
	oedit.o					\
	redit.o					\
	zedit.o					\
	medit.o					\
	sedit.o					\
	quest.o					\
	teleport.o		\
	castle.o \
	arena.o \
	clan.o \
	log.o \
    	color.o \
    	ident.o \
    	ego.o \
    	events.o queue.o \
    	eventprocs.o \
    	memory.o \
    	newquest.o     	\
    	trap.o	\
    	ackmapper.o \
    	soundex.o


all : .accepted $(BINDIR)/circle


# silly license document that must be viewed at least once before building

.accepted:
	


.PHONY:	utils
utils : .accepted $(BINDIR)/autowiz $(BINDIR)/delobjs $(BINDIR)/listrent $(BINDIR)/mudpasswd $(BINDIR)/purgeplay $(BINDIR)/showplay $(BINDIR)/sign $(BINDIR)/split $(BINDIR)/showmail


$(BINDIR)/circle : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(BINDIR)/a.exe


clean :
	rm -f *.o util/*.o

TAGS  : $(SRCS) $(HDRS)
	etags -T $(HDRS) $(SRCS)

# set this to the GNU C++ include directory (irrelevent?)
DEPENDINCLUDES = -I/opt/gnu/lib/g++-include

depend   : $(SRCS)
	makedepend -DMAKINGDEPEND $(DEFS) $(DEPENDINCLUDES) $(INCLUDES) $(HDRS) $(SRCS)


# DO NOT DELETE THIS LINE -- make depend depends on it.
