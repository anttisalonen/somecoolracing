CXX      ?= g++
AR       ?= ar
CXXFLAGS ?= -O2 -g3 -Werror
CXXFLAGS += -std=c++11 -Wall

CXXFLAGS += $(shell sdl-config --cflags)
LDFLAGS  += $(shell sdl-config --libs) \
		  -lSDL_image -lSDL_ttf -lGL -lGLEW

CXXFLAGS += -Isrc
BINDIR       = bin

# Common lib

COMMONSRCDIR = src/common
COMMONLIB = $(COMMONSRCDIR)/libcommon.a
COMMONSRCS = $(shell (find $(COMMONSRCDIR) \( -name '*.cpp' -o -name '*.h' \)))


# Main binary

MAINBINARYBINNAME = somecoolracing
MAINBINARYBIN     = $(BINDIR)/$(MAINBINARYBINNAME)
MAINBINARYSRCDIR = src
MAINBINARYSRCFILES = abyss/Particle.cpp abyss/RigidBody.cpp \
		     scr/GameDriver.cpp scr/Game.cpp scr/main.cpp

MAINBINARYSRCS = $(addprefix $(MAINBINARYSRCDIR)/, $(MAINBINARYSRCFILES))
MAINBINARYOBJS = $(MAINBINARYSRCS:.cpp=.o)
MAINBINARYDEPS = $(MAINBINARYSRCS:.cpp=.dep)



.PHONY: clean all

all: $(MAINBINARYBIN)

$(BINDIR):
	mkdir -p $@

$(COMMONLIB): $(COMMONSRCS)
	make -C $(COMMONSRCDIR)

$(MAINBINARYBIN): $(COMMONLIB) $(MAINBINARYOBJS) $(BINDIR)
	$(CXX) $(LDFLAGS) $(MAINBINARYOBJS) $(COMMONLIB) -o $@


%.dep: %.cpp
	@rm -f $@
	@$(CXX) -MM $(CXXFLAGS) $< > $@.P
	@sed 's,\($(notdir $*)\)\.o[ :]*,$(dir $*)\1.o $@ : ,g' < $@.P > $@
	@rm -f $@.P

clean:
	find src/ -name '*.o' -exec rm -rf {} +
	find src/ -name '*.dep' -exec rm -rf {} +
	find src/ -name '*.a' -exec rm -rf {} +
	rm -rf $(MAINBINARYBIN)
	rmdir $(BINDIR)

-include $(MAINBINARYDEPS)

