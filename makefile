SRC=$(wildcard src/*.cpp)
OBJ=$(patsubst src/%.cpp, bin/%.o, $(SRC))
EXE=partViewer.exe

CC=g++
CFLAGS=-Wall -g -O3
LDFLAGS= -lglew32 -lfreeglut -lopengl32 -lglu32 -lAntTweakBar
RM=del /q

SHELL=cmd.exe

vpath %.o bin/

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all
all: $(EXE)
	@echo Done

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	
.PHONY: clean
clean:
	-$(RM) bin\*
	@echo Clean Done!
