TARGET=$(shell basename `pwd`)
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:%.cpp=%.o)

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
		$(CXX) -g -o lex $(LDFLAGS) $(OBJECTS) $(LOADLIBES) $(LDLIBS)

clean:
		$(RM) $(OBJECTS) $(TARGET)

.PHONY: all clean