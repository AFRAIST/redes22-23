TOPDIR ?= $(CURDIR)

TARGET	 = client
BUILD	 = build
SOURCES  = src
INCLUDES := include

#-----Compilers-----

CC	= gcc

CXX = g++

#------------------

#-----Get Flags---------

CFLAGS = $(INCLUDE) -march=native -Wall -Wextra -g -O2 -pedantic -ffunction-sections -fdata-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -std=gnu++20 -fno-exceptions

LIBS := -pthread -lm

DEPENDENCY_FLAGS = -MMD -MP -MF $(DEPSDIR)/$*.d

#----------------------

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Phony $(BUILD) so that the fact that it still executes even if it exists.
.PHONY: $(BUILD) all clean

export TOPDIR	:=	$(CURDIR)

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(OUTPUT).zip	:	$(OUTPUT)
	zip -jr $@ $(SOURCES)

clean:
	@echo Cleaning all build...
	@rm -rf $(BUILD) $(TARGET)
	@rm -rf $(BUILD) $(TARGET).zip

#-------------------------------------------------------------------------------
else
.PHONY: all

DEPENDS	:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all	:	$(OUTPUT)

$(OUTPUT)	:	$(OFILES)
	$(LD) $(OFILES) $(LIBS) -o $@

%.o: %.c
	$(CC) $(DEPENDENCY_FLAGS) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(DEPENDENCY_FLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

