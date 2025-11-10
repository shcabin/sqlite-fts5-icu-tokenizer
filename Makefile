all : libicu test

libicu: fts5icu.cpp unicode61.cpp fts5_unicode2.c
	g++  -fPIC  fts5icu.cpp  unicode61.cpp fts5_unicode2.c \
	-licui18n -licuuc -licudata -I sqlite-src-3500400 \
	-shared -o libicu.so -O2

test: test.cpp
	g++ test.cpp sqlite-src-3500400/libsqlite3.a \
	-Wl,-Bstatic -licui18n -licuuc -licudata -Wl,-Bdynamic \
	-I sqlite-src-3500400 \
	-o test -g -O0 -lm

clean :
	rm -rf libicu.so test

