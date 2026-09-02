CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lSDL2 -lSDL2_ttf
TARGET = game_2048
SRC = game_2048_sdl.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
