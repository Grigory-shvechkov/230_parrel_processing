IDIR = inc
SDIR = src
ODIR = obj
DEPS = $(wildcard $(IDIR)/*.h)

# Object files
OBJS = $(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(wildcard $(SDIR)/*.c)) $(ODIR)/util.o

APPBIN = scheduler

CC = gcc
ifeq ($(DEPS),)
CFLAGS = -Wall -Wextra -g -pthread -std=c11
else
CFLAGS = -I$(IDIR) -Wall -Wextra -g -pthread -std=c11
endif
LIBS = -lm

all: $(APPBIN)

$(ODIR):
	mkdir -p $(ODIR)

# Compile src/*.c
$(ODIR)/%.o: $(SDIR)/%.c $(ODIR) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile inc/util.c
$(ODIR)/util.o: $(IDIR)/util.c $(IDIR)/util.h | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link all object files
$(APPBIN): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

submission:
ifeq ($(DEPS),)
	zip -r submission src
else
	zip -r submission src inc
endif
	@if [ -f README.txt ]; then zip submission README.txt; fi

.PHONY : clean
clean:
	rm -f $(APPBIN)
	rm -f *.zip
	rm -fr $(ODIR)
