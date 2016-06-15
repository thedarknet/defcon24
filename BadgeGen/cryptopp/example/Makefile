CC=c++
CFLAGS=-I../.. -c -Wall
LDFLAGS=-static -lpthread
SOURCES=main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=genBadge

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ ./libcryptopp.a

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ 

clean:
	rm $(EXECUTABLE)
