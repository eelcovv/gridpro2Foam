CFLAGS = -g
LDFLAGS = -g
LINKOPTS =
LOCAL_LIBRARIES = 

all:: gridpro2FOAM

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

util.o:	util.c util.h
	$(CC) -c $(CFLAGS) util.c

gridpro2FOAM.o:	gridpro2FOAM.cpp util.h
	$(CXX) -c $(CFLAGS) gridpro2FOAM.cpp

OBJECTS =   Patch.o Node.o Block.o gridpro2FOAM.o  util.o Triplet.o Face.o RTechTimer.o PeriodicStats.o


gridpro2FOAM:: $(OBJECTS) $(DEPS) 
	rm -f $@
	$(CXX) -o $@ $(LINKOPTS) $(OBJECTS) \
          $(DEPS) $(LOCAL_LIBRARIES) $(LDFLAGS) $(SYSLIBS) $(SYSLAST_LIBRARIES)

clean::
	rm -f *.o *.a gridpro2FOAM


tar::
	tar cvfz GridPro2FOAM_5.7.tgz Makefile *.h *.hpp *.cpp *.c compare README

install::
	cp -v gridpro2FOAM $(FOAM_SITE_APPBIN)
