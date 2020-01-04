TARGET=bench

CC=gcc
CXX=g++
SRC_DIRS=./

#SRCS := $(shell find $(SRC_DIRS) -name "*.cpp" -or -name "*.cc" -or -name "*.c" -or -name "*.s")
#OBJS := $(addsuffix .o,$(basename $(SRCS)))

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %cc, %o, $(SRCS))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INC_FLAGS_BENCH=-I./msgpack-c/include -I./ -I/usr/local/include/google/protobuf/

INC_FLAGS_XBASE=-I/mnt/centos-share/top/xchain_github/src/xtopcom/xbase -I/mnt/centos-share/top/xchain_github/src/xtopcom/xbase/uv/ 


CFLAGS=
CPPFLAGS=-std=c++11

LDFLAGS=
LDLIBS=-L /usr/local/lib/ -Wl,-Bstatic -lprotobuf -Wl,-Bdynamic -pthread
LDLIBS_XBASE=-L /mnt/centos-share/top/xchain_github/src/xtopcom/xbase/libs/Linux/debug -Wl,-Bstatic -lxbase -luv   -lintel_aes64 -lmbedcrypto -lmbedtls -lmbedx509  -Wl,-Bdynamic -lgcc -lstdc++ -lpthread -ldl -std=c++11

%.o:%.cc
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INC_FLAGS_BENCH) $(INC_FLAGS_XBASE) -c $<

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(CPPFLAGS) -o $@  $(LDLIBS) $(LDLIBS_XBASE)

.PHONY: clean

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
