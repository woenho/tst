# * ---------------------
# * UTF-8 한글확인 용
# * ---------------------
 
.SUFFIXES = .cpp .o
CXX=g++

# sha1.c 는 컴파일 하지 않고 opelssl 에 있는 함수를 사용한다.
# sha1.c, sha1.h 는 참조로 프로젝트에 추가만 한다

TARGET = amiproc
SRCS =  http.cpp \
		http_routes.cpp \
		websocket.cpp \
		ws_routes.cpp \
		processevents.cpp \
		amiaction.cpp \
		amiproc.cpp
OBJS = $(SRCS:.cpp=.o)

CFLAGS_RELEASE = -O2 -fPIC
CFLAGS_DEBUG = -O0 -g -ggdb3 -DDEBUG -fno-default-inline
CFLAGS  = -Wall -Wextra -Wshadow -Wformat-security -Winit-self -fpermissive
CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function
CFLAGS += -I../include
CFLAGS += -D_REENTRANT -D_PTHREADS

LFLAGS  = -lm -ldl -lpthread -lcrypto -L../lib 

ifeq (release, $(findstring release, $(MAKECMDGOALS))) #"make clean release"으로 clean 다음에 연이어 release가 나올 수가 있다.
	CFLAGS += $(CFLAGS_RELEASE)
	LFLAGS += -ltst -lAsyncThreadPool
else
	CFLAGS += $(CFLAGS_DEBUG)
ifeq (debugtrace, $(findstring debugtrace, $(MAKECMDGOALS))) #"make clean debugtrace"으로 clean 다음에 연이어 debugtrace 나올 수가 있다.
	CFLAGS += -DDEBUGTRACE
	LFLAGS += -ltst_t -lAsyncThreadPool_t
else
	LFLAGS += -ltst_d -lAsyncThreadPool_d
endif
endif

.PHONY: all clean debug release debugtrace

all: $(OBJS) $(TARGET)

$(TARGET):
	$(CXX) -o $@ $(OBJS) $(LFLAGS)

debug: all
release: all
debugtrace: all

%.o: %.cpp
	$(CXX) -o $@ $< -c $(CFLAGS)



clean:
	rm -f $(TARGET) *.o


