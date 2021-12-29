# * ---------------------
# * UTF-8 한글확인 용
# * ---------------------
 
#.SUFFIXES = .cpp .o
CXX=g++

LIBTST = libtst.a
SRCS = tst.cpp 
OBJS = $(SRCS:.cpp=.o)

CFLAGS_RELEASE = -O2 -fPIC
CFLAGS_DEBUG = -O0 -g -ggdb3 -DDEBUG -fno-default-inline
CFLAGS  = -Wall -Wextra -Wshadow -Wformat-security -Winit-self -fpermissive
CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function
CFLAGS += -D_REENTRANT -D_PTHREADS
ifeq (release, $(findstring release, $(MAKECMDGOALS))) #"make clean release"으로 clean 다음에 연이어 release가 나올 수가 있다.
	CFLAGS += $(CFLAGS_RELEASE)
else
	CFLAGS += $(CFLAGS_DEBUG)
	LIBTST = libtst_d.a
ifeq (debugtrace, $(findstring debugtrace, $(MAKECMDGOALS))) #"make clean debugtrace"으로 clean 다음에 연이어 debugtrace 나올 수가 있다.
	CFLAGS += -DDEBUGTRACE
endif
endif


LFLAGS  = -lm -ldl -lpthread

.PHONY: all clean debug release debugtrace cversion

all:  cversion $(LIBTST)
debug: all
release: all
debugtrace: all

%.o: %.cpp
	$(CXX) -o $@ $< -c $(CFLAGS)

$(LIBTST): $(OBJS)
	ar cq $(LIBTST) $(OBJS)
	mkdir -p ../lib
	cp $(LIBTST) ../lib
	mkdir -p ../include
	cp tst.h ../include

cversion:
	echo `$(CXX) -v`

clean:
	rm -f $(LIBTST) example *.o

