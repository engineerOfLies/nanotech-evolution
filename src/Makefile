############################################################################
#
# The Linux-GCC Makefile
#
##############################################################################

#
# Object files.
#

OBJ = $(patsubst %.c,%.o,$(wildcard *.c))

#
# Compiler stuff -- adjust to your system.
#

# Linux
PROJECT = nvp
CC      = gcc
#CC	= clang
SDL_CFLAGS = `sdl-config --cflags`
SDL_LDFLAGS = `sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_net -lz -lm
LFLAGS = -g -o ../$(PROJECT)
CFLAGS = -g -Wall -pedantic -std=gnu99 -fgnu89-inline -Wno-unknown-pragmas 
# -ffast-math for relase version

DOXYGEN = doxygen

#
# Targets
#

$(PROJECT): $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) $(SDL_LDFLAGS)
docs:
	$(DOXYGEN) ../docs/Doxygen
  
depend:
	@-rm makefile.dep
	@echo Creating dependencies.
	@for i in *.c; do $(CC) $(INC) -MM $$i; done > makefile.dep
	@echo Done.

clean:
	rm *.o ../$(PROJECT)

count:
	wc *.c *.h makefile

#
# Dependencies.
#

include makefile.dep

#
# The default rule.
#

.c.o:
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $<

