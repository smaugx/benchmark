TARGET=bench

CC=gcc
CXX=g++
SRC_DIRS=./

#SRCS := $(shell find $(SRC_DIRS) -name "*.cpp" -or -name "*.cc" -or -name "*.c" -or -name "*.s")
#OBJS := $(addsuffix .o,$(basename $(SRCS)))

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %cc, %o, $(SRCS))
DEPS := $(OBJS:.o=.d)

#INC_DIRS := $(shell find $(SRC_DIRS) -type d)
#INC_FLAGS := $(addprefix -I,$(INC_DIRS))

INC_DIRS=-I./
INC_DIRS_THIRD=-I./third_party -I./third_party/include

CFLAGS=
CPPFLAGS=-std=c++11

LDFLAGS=

LDLIBS_THIRD=-L ./third_party/lib -lprotobuf -lmsgpackc -lgflags
LDLIBS=-lgcc -lstdc++ -lpthread -ldl

%.o:%.cc
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INC_DIRS) $(INC_DIRS_THIRD) -c $<

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(CPPFLAGS) -o $@  $(LDLIBS_THIRD) $(LDLIBS)

.PHONY: clean

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
