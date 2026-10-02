CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

TARGET := test_simple
SHAPES_TARGET := check_demo

.PHONY: all demos run run-shapes clean

all: $(TARGET)

demos: $(TARGET) $(SHAPES_TARGET)

$(TARGET): main_simple.o animate.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SHAPES_TARGET): check.o animate.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

animate.o: animate.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c animate.c -o $@

main_simple.o: main_simple.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c main_simple.c -o $@

check.o: check.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c check.c -o $@

run: $(TARGET)
	./$(TARGET)

run-shapes: $(SHAPES_TARGET)
	./$(SHAPES_TARGET)

clean:
	rm -f animate.o main_simple.o check.o
	rm -f $(TARGET) $(SHAPES_TARGET)
	rm -f simple.dat
	rm -f Doxyfile PointerProAnimateRefman.pdf
	rm -rf latex html
