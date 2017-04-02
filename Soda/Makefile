cxxflags := $(CPPFLAGS) -I. $(CXXFLAGS) -std=c++14
ldflags := $(LDFLAGS)

sources := \
	DotGenerator.cpp \
	Main.cpp \
	NameMangler.cpp \
	Options.cpp \
	Parser.cpp \
	ScopeBuilder.cpp \
	SourceFile.cpp \
	SymbolResolver.cpp \
	SymbolTable.cpp \
	Tokenizer.cpp \
	Visitor.cpp

objects := $(sources:.cpp=.o)
depends := $(sources:.cpp=.d)

all: soda

clean:
	$(RM) *.o test.dot test.svg soda

soda: $(objects)
	$(CXX) $(strip $(cxxflags) -o $@ $(objects) $(ldflags))

.cpp.o:
	$(CXX) $(strip $(cxxflags) -c -MMD -o $@ $<)

-include $(depends)

test: soda test.svg

test.svg: test.dot
	dot -Tsvg -o $@ test.dot

test.dot: soda test.soda
	./soda test.soda > $@

.PHONY: all clean
