OutPutOpt     = -o 

CXX           = g++
CXXFLAGS      = -Wall -Wno-unused-local-typedefs -Wno-deprecated-declarations
LD            = g++
LDFLAGS       = -O #-pg 

CXXFLAGS     += -g #-pg 
LIBS          = -LC:\Users\Rolf\Desktop\boost_1_60_0\bin.v2\libs\system\build\gcc-mingw-6.3.0\release\link-static\threading-multi -lboost_system-mgw63-mt-1_60

GL_LIBS       = 
INCLUDES      = -I"C:\Users\Rolf\Desktop\boost_1_60_0"

OBJECTS       = Parser.o Object.o DateParser.o Tester.o 

#------------------------------------------------------------------------------
# Clean suffixes so I can overwrite default cc -> o rule. 
.SUFFIXES: 
.PHONY: clean 
all:	parser

# Make object files depend on corresponding .cc and .hh files. 
%.o:	%.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

parser:		$(OBJECTS)
		ar rcs libParser.lib $(OBJECTS) 

tester:		$(OBJECTS)
		$(LD) $(CXXFLAGS) $(GL_LIBS) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt) $@
		@echo "$@ done"

clean:
		del *.o 
		del libParser.lib 

###


