DEPMODE=gcc
EXTRA_CFLAGS=-Wno-deprecated -O3 -fstrict-aliasing -ffast-math -fforce-mem -finline-functions -funroll-all-loops -fomit-frame-pointer  -mcpu=pentium4 -msse -mfpmath=sse -malign-double -msse2 -mfpmath=sse
CUDD=/usr/local/cudd-2.3.1
CUDD_LIB=-L$(CUDD) -L$(CUDD)/lib -lobj -lcudd -lmtr -lst -lutil -lepd -lm
CUDD_CFLAGS=-I$(CUDD) -I$(CUDD)/include
MTL=/usr/local/mtl-2.1.2-21/
MTL_LIB=
MTL_CFLAGS=-I$(MTL)
FLEX_LIB=/usr//lib/libfl.a
READLINE_LIB=-lcurses -lreadline
