CC = g++
INCLUDE =   -I./ \
            -I/Users/zermin/work/library/boost/include \
            -I/Users/zermin/work/library/mysql/include \
            -I/Users/zermin/work/library/cryptopp
LDFLAGS = 
CCFLAGS = -std=c++11 -stdlib=libc++ -g -Wall $(INCLUDE) -DJSON_IS_AMALGAMATION
SOURCES=$(wildcard *.cpp)
#OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS = $(patsubst %.cpp, obj/%.o, $(SOURCES))
TARGET=libtokLib.a

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	ar rc $@ $(OBJECTS)

obj/%.o: %.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
