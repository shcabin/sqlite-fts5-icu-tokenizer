
all: libicu test

libicu: fts5icu.cpp  unicode61.cpp fts5_unicode2.c
	g++  -fPIC  fts5icu.cpp  unicode61.cpp fts5_unicode2.c \
	-licui18n -licuuc -licudata \
	-L icu-release-78.1/icu4c/source/lib/  \
	-I sqlite-src-3500400 -I icu-release-78.1/icu4c/source/common/ -I icu-release-78.1/icu4c/source/i18n/ \
	-shared  -o libicu.so  -O2

test: test.cpp
	g++  test.cpp \
	-I sqlite-src-3500400 -L sqlite-src-3500400 -lsqlite3  \
	-L icu-release-78.1/icu4c/source/lib/ -licui18n -licuuc -licudata \
  	-o test -g -O0 -lm
	# run: LD_LIBRARY_PATH=./:icu-release-78.1/icu4c/source/lib/   ./test

clean :
	rm -rf libicu.so test
