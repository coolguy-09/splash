CC = gcc
CFLAGS = -L. -I./include/ -Os
LD = $(CC)
LDFLAGS = -L. -I./include/ -Os

LIBS = -lm

SRC = src/libsplash.c
OBJ = src/libsplash.o

PROGSRC = src/splash.c
PROGOBJ = src/splash.o

all: libsplash.so splash

src/libsplash.o: src/libsplash.c
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) -fPIC -c $< -o $@ $(LIBS)

libsplash.so: $(OBJ)
	@echo "  LD      $@.1.0.1"
	@$(LD) $(LDFLAGS) -shared -o $@.1.0.1 $^ $(LIBS)
	@echo "  LN      $@.1.0.1 -> $@.1"
	@ln -sf $@.1.0 $@.1
	@echo "  LN      $@.1 -> $@"
	@ln -sf $@.1 $@

src/splash.o: src/splash.c
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

splash: $(PROGOBJ)
	@echo "  LD      $@"
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBS) -lsplash

clean:
	@echo "  RM      src/libsplash.o libsplash.so libsplash.so.1 libsplash.so.1.0.1 src/splash.o splash"
	@rm -f src/libsplash.o libsplash.so libsplash.so.1 libsplash.so.1.0.1 src/splash.o splash
