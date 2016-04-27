# DikuMUD makefile 
CC = gcc 
CFLAGS = -g3 -O2 -pipe -fno-builtin-log
LIBS= -lcrypt

HEADERS = comm.h db.h handler.h interpreter.h limits.h maildef.h \
	os.h spells.h structs.h utils.h prototypes.h 
	
CFILES= comm.c act.comm.c act.informative.c act.movement.c act.obj1.c \
	act.obj2.c act.offensive.c act.other.c act.social.c act.wizard.c \
	handler.c db.c interpreter.c utility.c spec_assign.c shop.c \
	limits.c mobact.c fight.c modify.c weather.c spells1.c spells2.c \
	spell_parser.c reception.c constants.c spec_procs.c signals.c \
	board.c mar_fiz_maz.c magic.c changes.c 
# .o versions of above
OFILES= $(CFILES:.c=.o)

OTHERSTUFF= mail.c os.c

UTILITIES= insert_any.c repairgo.c list.c syntax_checker.c \
	sign.c update.c delplay.c

# documentation
DOCS= actions.doc defs.doc license.doc running.doc time.doc combat.doc \
	do_mail look.doc shops.doc values.doc comm.doc handler.doc macro.doc \
	skills.doc database.doc interpreter.doc newstruct.doc spell_info.doc \
	dbsup.doc levels.doc readme spells.doc
PDOCS= $(patsubst %,doc/%,$(DOCS))	

# data - zones, help, mobs, objects, rooms, etc.
DATA= actions help_table news readme tinyworld.wld board.messages info \
	pcobjs.obj tinyworld.mob tinyworld.zon credits messages players \
	tinyworld.obj wizlist help motd poses tinyworld.shp
PDATA= $(patsubst %,lib/%,$(DATA))	

# Files in the standard distribution
DISTFILES= $(CFILES) $(HEADERS) $(PDOCS) $(PDATA) $(UTILITIES) \
	$(OTHERSTUFF) nightrun opstart readme INSTALL_NOTES \
	makefile makefile.bor makefile.dgm makefile.vc makefile.lcc 
PDIST= $(patsubst %,diku-alfa/%,$(DISTFILES))
RELEASE=dist

TARGETS= dmserver list delplay insert_any repairgo \
	syntax_checker update sign
OTARGETS=  list.o delplay.o insert_any.o repairgo.o syntax_checker.o \
	update.o sign.o	

all: $(TARGETS)

dmserver : $(OFILES)
	$(CC) $(CFLAGS) -o dmserver $(OFILES) $(LIBS)

list : list.o
	$(CC) $(CFLAGS) -o list list.o
	
delplay : delplay.o
	$(CC) $(CFLAGS) -o delplay delplay.o

insert_any : insert_any.o
	$(CC) $(CFLAGS) -o insert_any insert_any.o

repairgo : repairgo.o
	$(CC) $(CFLAGS) -o repairgo repairgo.o

syntax_checker : syntax_checker.o
	$(CC) $(CFLAGS) -o syntax_checker syntax_checker.o

update : update.o
	$(CC) $(CFLAGS) -o update update.o

sign : sign.o
	$(CC) $(CFLAGS) -o sign sign.o

clean:
	-rm -f *.d $(OFILES) $(TARGETS) $(OTARGETS) 

dist: 
	ln -s ./ diku-alfa
	tar czvf diku-alfa-$(RELEASE).tar.gz $(PDIST) 
	rm diku-alfa

# pull in dependency info for *existing* .o files
OBJDEPENDS := $(OFILES) delplay.o list.o 
-include $(OBJDEPENDS:.o=.d)
	
# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    append directory to object target. (gcc bug?)
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o 
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
