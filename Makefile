# Makefile с поддержкой кросс-компиляции для Windows

# --- Переменные ---
TARGET_LINUX = translator.exe
TARGET_WINDOWS = translator_win.exe

SOURCES = main.cpp scanner.cpp parser.cpp semantic.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Общие флаги компиляции
COMMON_CXXFLAGS = -g -Wall -std=c++17

# --- Правила ---

# Правило по умолчанию: нативная сборка для Linux
all: linux

# Правило для нативной сборки
linux: CXX = g++
linux: CXXFLAGS = $(COMMON_CXXFLAGS)
linux: $(TARGET_LINUX)

# Правило для кросс-компиляции под Windows
windows: $(TARGET_WINDOWS)

# Set the compiler AFTER declaring the rule
windows: CXX = x86_64-w64-mingw32-g++
windows: CXXFLAGS = $(COMMON_CXXFLAGS)
windows: LDFLAGS = -static -static-libgcc -static-libstdc++

# --- Общие правила сборки ---

# Правило для линковки (создания исполняемого файла)
$(TARGET_LINUX): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

$(TARGET_WINDOWS): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

# Правило для компиляции .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Правило для очистки
clean:
	rm -f $(TARGET_LINUX) $(TARGET_WINDOWS) *.o
