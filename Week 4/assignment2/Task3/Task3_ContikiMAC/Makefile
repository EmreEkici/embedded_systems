CONTIKI_PROJECT = source
all: $(CONTIKI_PROJECT)

CONTIKI = ../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
TARGET_LIBFILES += -lm

CONTIKI_WITH_RIME = 1

include $(CONTIKI)/Makefile.include
