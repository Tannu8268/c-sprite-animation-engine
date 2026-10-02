CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

LIBANIMATE_DIR ?= libanimate
LIBANIMATE_HEADER := $(LIBANIMATE_DIR)/include/animate/animate.h
LIBANIMATE_ARCHIVE := $(LIBANIMATE_DIR)/lib/libanimate.a

ENGINE_TARGET := test_simple
SHAPES_TARGET := check_demo
CLIENT_TARGET := animate_client
SERVER_TARGET := animate_server

.PHONY: all engine demos client server run run-shapes check-libanimate clean

all: demos client

engine: animate.o

demos: $(ENGINE_TARGET) $(SHAPES_TARGET)

client: $(CLIENT_TARGET)

server: $(SERVER_TARGET)

$(ENGINE_TARGET): main_simple.o animate.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SHAPES_TARGET): check.o animate.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(CLIENT_TARGET): animate_client.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(SERVER_TARGET): animate_server.c check-libanimate
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(LIBANIMATE_DIR)/include $< \
		$(LDFLAGS) -L$(LIBANIMATE_DIR)/lib -lanimate $(LDLIBS) -o $@

animate.o: animate.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

main_simple.o: main_simple.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

check.o: check.c animate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

animate_client.o: animate_client.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

check-libanimate:
	@test -f "$(LIBANIMATE_HEADER)" || { \
		echo "ERROR: missing $(LIBANIMATE_HEADER)" >&2; \
		echo "Extract the supplied libanimate archive or set LIBANIMATE_DIR." >&2; \
		exit 1; \
	}
	@test -f "$(LIBANIMATE_ARCHIVE)" || { \
		echo "ERROR: missing $(LIBANIMATE_ARCHIVE)" >&2; \
		echo "Extract the supplied libanimate archive or set LIBANIMATE_DIR." >&2; \
		exit 1; \
	}

run: $(ENGINE_TARGET)
	./$(ENGINE_TARGET)

run-shapes: $(SHAPES_TARGET)
	./$(SHAPES_TARGET)

clean:
	rm -f animate.o main_simple.o check.o animate_client.o
	rm -f $(ENGINE_TARGET) $(SHAPES_TARGET) $(CLIENT_TARGET) $(SERVER_TARGET)
	rm -f simple.dat
	rm -f Doxyfile PointerProAnimateRefman.pdf
	rm -rf latex html
