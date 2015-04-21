# Makefile for CNotes

CC = gcc
LD = gcc

CURRENT_DIR = $(shell echo %~dp0)
INC = -I.
CFLAGS = -w -std=gnu99 -g
RESINC = 
LIBDIR = -Llib
LIBDIR_WIN =
LIBDIR_NIX =
LIB = -lssl -lcrypto -lws2_32 -lgdi32
LIB_WIN =
LIB_NIX =
LDFLAGS =
OBJDIR = obj
OUTDIR = bin
OUT_WIN = chatbot.exe
OUT_NIX = chatbot
SRC = $(wildcard *.c)
DEP =

ifdef SYSTEMROOT
	LIB += $(LIB_WIN)
	LIBDIR += $(LIBDIR_WIN)
	OUT = $(OUT_WIN)
else
	LIB += $(LIB_NIX)
	LIB += $(LIBDIR_NIX)
	OUT = $(OUT_NIX)
endif

all: chatbot

clean:
	rm -rf $(OBJDIR)
	rm -rf $(OUTDIR)

before:
ifdef SYSTEMROOT
	-@ if not exists "$(OUTDIR)" ( mkdir $(OUTDIR) )
	-@ if not exists "$(OBJDIR)" ( mkdir $(OBJDIR) )
else
	mkdir -p $(OUTDIR)
	mkdir -p $(OBJDIR)
endif

chatbot: before out after

after:

$(SRC): before
	echo $@
	$(CC) -o $(OBJDIR)/$(notdir $(@:.c=.o)) -c $@ $(LIBDIR) $(LIB) $(CFLAGS) $(INC) 

out: before $(DEP) $(SRC)
	$(LD) -o $(OUTDIR)/$(OUT) $(OBJDIR)/* $(LIBDIR) $(LIB)

.PHONY: before after clean out check chatbot all $(SRC) run

