all :   libicu   test

libicu: fts5icu.cpp  unicode61.cpp fts5_unicode2.c
	g++  -fPIC  fts5icu.cpp  unicode61.cpp fts5_unicode2.c \
	-licui18n -licuuc -licudata \
	-shared  -o libicu.so  -O2

test:  test.cpp
	g++   test.cpp  \
	 -licui18n -licuuc -licudata -lsqlite3 \
	-o test -g -O0 -lm

clean :
	rm -rf libicu.so test

