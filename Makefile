CC=gcc
CFLAGS=-Wall -O3 -g -std=c99 -pedantic
LDFLAGS=

ifeq ($(PREFIX),)
  PREFIX=/usr/local
endif

TARGETS=pytt.o lookup3.o
LIB_TARGET=libpytt.a
TEST_TARGETS=hash_test collision_test typed_test

all: $(LIB_TARGET) $(TEST_TARGETS) pytt.pc

$(LIB_TARGET): $(TARGETS)
	ar -cru $(LIB_TARGET) $(TARGETS)
	ranlib $(LIB_TARGET)

hash_test: hash_test.c $(LIB_TARGET)
collision_test: collision_test.c $(LIB_TARGET)
typed_test: typed_test.c $(LIB_TARGET)

%:%.c libpytt.a
	$(CC) $(CFLAGS) $< -L. -lpytt $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

pytt.o: pytt.c pytt.h lookup3.h
lookup3.o: CFLAGS+=-Wno-unused-variable
lookup3.o: lookup3.c lookup3.h

pytt.pc:

clean:
	rm -f $(TARGETS) pytt.pc

distclean: clean
	rm -f $(LIB_TARGET) $(TEST_TARGETS)

install: $(LIB_TARGET)
	install -m 644 $(LIB_TARGET) $(PREFIX)/lib/
	install -m 644 pytt.h $(PREFIX)/include/
	install -m 644 pytt++.h $(PREFIX)/include/
	install -m 644 pytt.pc $(PREFIX)/lib/pkgconfig/
