GROUP_NUMBER	= 28

#-----Compilers-----

export CC	= gcc

export CXX	= g++

export LD	= $(CXX)

#------------------

#-----Get Flags---------

export CFLAGS = -march=native -Wall -Wextra -g -O2 -pedantic \
				-ffunction-sections -fdata-sections -Wl,--gc-sections \
				-fno-strict-aliasing
export CXXFLAGS = $(CFLAGS) -std=gnu++20 -fno-exceptions -fno-rtti

#----------------------


#----Other------------
export OUTDIR = $(CURDIR)
CLIENT_DIR	= player_src
CLIENT_MK 	= $(CLIENT_DIR)/client.mk
SERVER_DIR	= GS_src
SERVER_MK	= $(SERVER_DIR)/server.mk
#---------------------

#----Programs---------
.PHONY: all clean client server fenix

all: client server

client:
	$(MAKE) all -C $(CLIENT_DIR) -f $(CURDIR)/$(CLIENT_MK)

server:
	$(MAKE) all -C $(SERVER_DIR) -f $(CURDIR)/$(SERVER_MK)

clean:
	@echo Cleaning all build...
	$(MAKE) clean -C $(CLIENT_DIR) -f $(CURDIR)/$(CLIENT_MK)
	$(MAKE) clean -C $(SERVER_DIR) -f $(CURDIR)/$(SERVER_MK)

fenix: clean
	@zip -r proj_$(GROUP_NUMBER).zip * 

#---------------------

