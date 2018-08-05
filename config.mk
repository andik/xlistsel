# Where is your X11 installed?
X11DIR=/usr/X11R6

# make install target
PREFIX=/usr/local

# will be called upon 'make install' 	like $(INSTALL) src dest 
INSTALLSCRIPT=install -o root -g bin -m 755
INSTALLDATA=install -o root -g bin -m 644

# Compiler and Linker Flags
CINCL   = -I$(X11DIR)/include -I./vendor -I./generated
CFLAGS  = -pedantic -O2 -g -D_POSIX_C_SOURCE=200809L $(CINCL)
LDFLAGS = -L$(X11DIR)/lib -lX11 -lm

