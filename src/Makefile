##############################################################################
#
# The Linux-GCC Makefile
#
##############################################################################

#
# Object files.
#

OBJ = game.o graphics.o font.o entity.o audio.o particle.o area.o monster.o hud.o skills.o combat.o events.o net.o items.o
#
# Compiler stuff -- adjust to your system.
#

# Linux

CC     = gcc
SDL_CFLAGS = `sdl-config --cflags`
SDL_LDFLAGS = `sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_net
LFLAGS = -g -o ../nvp
CFLAGS = -g -Wall -pedantic -O3

#
# Targets.
#

navo: $(OBJ)
	gcc $(OBJ) $(LFLAGS) $(SDL_LDFLAGS)
  
depend:
	@-rm makefile.dep
	@echo Creating dependencies.
	@for i in *.c; do $(CC) $(INC) -MM $$i;
	@echo Done.

clean:
	rm *.o ../nvp

count:
	wc *.c *.h makefile

#
# Dependencies.
#

net.o: net.c net.h hud.h combat.h game.h
events.o: events.c events.h monster.h particle.h
items.o: items.c items.h monster.h
combat.o: combat.c combat.h entity.h hud.h monster.h skills.h particle.h net.h
skills.o: skills.c skills.h monster.h hud.h events.h area.h
hud.o: hud.c hud.h monster.h entity.h area.h net.h
monster.o:monster.c monster.h entity.h area.h hud.h game.h
area.o: area.c area.h graphics.h monster.h area.h hud.h combat.h events.h skills.h
particle.o: particle.c particle.h entity.h graphics.h
entity.o: entity.c entity.h graphics.h audio.h
audio.o: audio.c audio.h
font.o: font.c graphics.h
graphics.o: graphics.c graphics.h
game.o: game.c game.h graphics.h entity.h particle.h game.h area.h monster.h hud.h skills.h combat.h events.h items.h net.h

#
# The default rule.
#

.c.o:
	gcc $(CFLAGS) $(SDL_CFLAGS) -c $<

