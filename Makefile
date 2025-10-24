CXX = g++
CXXFLAGS = -g -Wall -std=c++17

TARGET = translator.exe

SOURCES = main.cpp scanner.cpp parser.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
