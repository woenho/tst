# * ---------------------
# * UTF-8 한글확인 용
# * ---------------------
 
.SUFFIXES = .cpp .o
CXX=g++

TARGET = echoserver
SRCS = echoserver.cpp 
OBJS = $(SRCS:.cpp=.o)

CFLAGS_RELEASE = -O2 -fPIC
CFLAGS_DEBUG = -O0 -g -ggdb3 -DDEBUG -fno-default-inline
CFLAGS  = -Wall -Wextra -Wshadow -Wformat-security -Winit-self -fpermissive
CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function
CFLAGS += -I../include
CFLAGS += -D_REENTRANT -D_PTHREADS

LFLAGS  = -lm -ldl -lpthread -L../lib 

ifeq (release, $(findstring release, $(MAKECMDGOALS))) #"make clean release"으로 clean 다음에 연이어 release가 나올 수가 있다.
	CFLAGS += $(CFLAGS_RELEASE)
	LFLAGS += -ltst
else
	CFLAGS += $(CFLAGS_DEBUG)
	LFLAGS += -ltst_d
ifeq (debugtrace, $(findstring debugtrace, $(MAKECMDGOALS))) #"make clean debugtrace"으로 clean 다음에 연이어 debugtrace 나올 수가 있다.
	CFLAGS += -DDEBUGTRACE
endif
endif

.PHONY: all clean debug release debugtrace cversion

all: cversion $(TARGET)

debug: all
release: all
debugtrace: all

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)  $(LFLAGS)


%.o: %.cpp
	$(CXX) -o $@ $< -c $(CFLAGS)

cversion:
	echo `$(CXX) -v`

clean:
	rm -f $(TARGET) *.o

