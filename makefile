SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
EXE=main.exe

CC=g++
CFLAGS=-Wall -g -O3
LDFLAGS= -lglew32 -lfreeglut -lopengl32 -lglu32
RM=del

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: all
all: $(EXE)
	@echo Done

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	
.PHONY: clean
clean:
	-$(RM) $(OBJ)
	@echo Clean Done!
