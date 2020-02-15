CXXFLAGS=-std=c++11 -g -Wall

OBJS=latexwiki.o format_document.o scan_document.o nodes.o input.o utility.o \
		errors.o make_indexes.o
TARGET=latexwiki

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

clean:
	$(RM) *.o $(TARGET)

.PHONY: clean
