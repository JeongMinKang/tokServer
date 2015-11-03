CC = g++
INCLUDE = -I./ \
          -I/Users/zermin/work/library/boost/include \
          -I/Users/zermin/work/library/mysql/include \
          -I/Users/zermin/work/library/cryptopp \
          -I../../tokLib
LIBPATH = -L/Users/zermin/work/library/boost/lib \
          -L/Users/zermin/work/library/mysql/lib \
          -L/Users/zermin/work/library/cryptopp \
          -L../../tokLib
LDFLAGS = -lboost_system -lmysqlclient -lcryptopp -ltokLib
CCFLAGS = -std=c++11 -stdlib=libc++ -g -Wall $(INCLUDE) -DJSON_IS_AMALGAMATION
SOURCES=$(wildcard *.cpp)
#OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS = $(patsubst %.cpp, obj/%.o, $(SOURCES))
TARGET=authServer

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LIBPATH) -o $@ $(OBJECTS) $(LDFLAGS)

obj/%.o: %.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
