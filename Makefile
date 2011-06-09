## Makefile for src/mod/python.mod

srcdir  = .
objects = api.o bridge.o pymod.o

# needed for building on OSX
PYTHON_INCLUDE = /System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6
PYTHON_LIB     = /usr/lib
REAL_LD        = /usr/bin/ld

doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../; make

static: ../python.o

modules: ../../../python.$(MOD_EXT)

../python.o: $(objects)
	$(REAL_LD) -o ../python.o -r $?

../../../python.$(MOD_EXT): ../python.o
	$(LD) -L$(PYTHON_LIB) -o ../../../python.$(MOD_EXT) ../python.o -lpython $(XLIBS) $(MODULE_XLIBS)
	$(STRIP) ../../../python.$(MOD_EXT)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(PYTHON_INCLUDE) -DMAKING_MODS -c $<

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM *.c > .depend

clean:
	@rm -fv .depend *.o *.$(MOD_EXT)

