TARGET = $(subst _src, ,$(notdir $(CURDIR)))
BUILD	 = build
SOURCES  = src
INCLUDES = $(OUTDIR)/include

#-----Get Flags---------

LIBS := -lm

DEPENDENCY_FLAGS = -MMD -MP -MF $(DEPSDIR)/$*.d

#----------------------

#-------Build----------

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Phony $(BUILD) so that the fact that it still executes even if it exists.
.PHONY: $(BUILD) all clean

export OUTPUT	:=	$(OUTDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES		:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES_COMMON		:=	$(foreach dir,$(OUTDIR)/common_src/build,$(wildcard $(dir)/*.o))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(dir))

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/client.mk

clean:
	@rm -rf $(BUILD) $(OUTPUT)

else
.PHONY: all

DEPENDS	:=	$(OFILES:.o=.d)

all	:	$(OUTPUT)

$(OUTPUT)	:	$(OFILES)
	$(LD) $(OFILES) $(OFILES_COMMON) $(LIBS) -o $@

%.o: %.c
	$(CC) $(DEPENDENCY_FLAGS) $(INCLUDE) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(DEPENDENCY_FLAGS) $(INCLUDE) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
#----------------------

