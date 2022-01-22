MAKEFLAGS := --jobs=16
MAKEFLAGS += --output-sync=target

# FLAGS = -std=c++2a -stdlib=libc++ -Wall -Wextra -g -fsanitize=address
# FLAGS = -std=c++2a -Wall -Wextra -g -I/usr/local/include -stdlib=libc++ -fsanitize=address -fsanitize=undefined
# consider adding -Wfloat-equal
FLAGS = -std=c++2a -Wall -Wextra -Wpedantic -Wuninitialized -Wshadow -Wmost -g -I/usr/local/include
LIBS = -lglfw -lglew 
FRAMEWORKS = -Ivendor/ -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon

SRC_DIR := ./diver
OBJ_DIR := ./output/diver
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
DEPENDS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.d,$(SRC_FILES))

EXE_DIR := $(OBJ_DIR)
EXE := $(OBJ_DIR)/diver.exe

LIBGEN = ar
CCC = clang++
MFLAGS = -MMD -MP 

all: diver

# change a bunch of vars to 
# support windows 
windows: CCC=x86_64-w64-mingw32-g++
windows: LIBGEN=x86_64-w64-mingw32-ar
windows: FLAGS=-std=c++2a -I/usr/local/include
windows: FRAMEWORKS=-Ivendor/
windows: EXE=$(OBJ_DIR)/diver.windows.exe
windows: MFLAGS=
windows: diver

# end windows

engine: 
	$(MAKE) -C vendor/supermarket-engine 

diver: engine $(OBJ_FILES)
	$(CCC) $(FLAGS) $(LIBS) $(FRAMEWORKS) -o $(EXE) $(SRC_FILES) ./vendor/supermarket-engine/output/libengine.a
	DEBUG=123 ./$(EXE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	@mkdir -p $(@D)
	$(CCC) $(FLAGS) $(MFLAGS) -c $< -o $@ 

clean:
	$(RM) $(OBJ_FILES) $(DEPENDS) 

trace:
	rm -f diver.exe.trace
	../apitracedyld/build/apitrace trace -o ./output/diver.trace --api gl $(EXE)

view:
	../apitrace/build/qapitrace ./output/diver.trace

.PHONY: all clean
