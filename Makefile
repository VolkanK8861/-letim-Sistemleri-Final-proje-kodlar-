CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)
CONTIKI = ../..
MODULES += os/storage/cfs
include $(CONTIKI)/Makefile.include
