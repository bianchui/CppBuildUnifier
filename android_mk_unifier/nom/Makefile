CXXFLAGS := -std=c++11 -g

LIB     := libnomnom.a

SOURCES := $(wildcard src/*.cpp src/builtin/*.cpp)

HEADERS := $(wildcard src/*.hpp)
APPSSRC := $(wildcard apps/*.cpp)

APPS    := $(APPSSRC:apps/%.cpp=bin/%)

all : $(APPS)

OBJECTS := $(SOURCES:src/%.cpp=.obj/%.o)

bin:
	mkdir bin

.obj/$(LIB): $(OBJECTS)
	ar rcs $@  $(OBJECTS)


bin/%:apps/%.cpp .obj/$(LIB) | bin
	$(CXX) $(CXXFLAGS) -Wl,--whole-archive  $^ -Wl,--no-whole-archive -ldl -lunwind -rdynamic -o $@

.obj:
	mkdir .obj
	mkdir .obj/boost
	mkdir -p .obj/builtin/

.obj/%.o:src/%.cpp $(HEADERS) | .obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf .obj
	rm -rf bin


test: bin/lexer bin/parser bin/semantics bin/builtin
	bin/lexer
	bin/parser
	bin/semantics
	bin/builtin
