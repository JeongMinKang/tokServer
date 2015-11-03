CC = g++
INCLUDE = -I./ \
          -I/Users/zermin/work/library/cryptopp
LIBPATH = -L/Users/zermin/work/library/cryptopp
LDFLAGS = -lcryptopp
CCFLAGS = -std=c++11 -stdlib=libc++ -g -Wall $(INCLUDE)
SOURCES=$(wildcard *.cpp)
#OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS = $(patsubst %.cpp, obj/%.o, $(SOURCES))
TARGET=keyPairGenerator

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LIBPATH) -o $@ $(OBJECTS) $(LDFLAGS)

obj/%.o: %.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
