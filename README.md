# Why Build Another Tokenizer?

While GitHub hosts a wealth of tokenizers, most suffer from two key limitations: they either support only specific languages or demand explicit language specification upfront.
My goal is to create a tokenizer that works seamlessly in uncertain language environments—particularly in mixed-language scenarios.

# sqlite-fts5-icu-tokenizer

The SQLite FTS5 extension provides International Components for Unicode (ICU) based tokenization for full-text search, support non-space-separated languages such as Chinese, Japanese. 

Furthermore, **it references the Unicode61 Tokenizer 'remove_diacritics=2' feature**, where by default, diacritics are removed from all Latin script characters. for example, "A", "a", "À", "à", "Â" and "â" are all considered to be equivalent.

The implementation fully complies with the FTS5 v2 API specifications and is written in C++.



## Prerequisites

1. Option 1

-  Install ICU and SQLite3 development libraries. On Ubuntu, you can use:
```bash
sudo apt-get install libicu-dev libsqlite3-dev 
```

2. Option 2: build from source

- Download ICU source code from https://github.com/unicode-org/icu/releases , and build ICU.
```bash
tar -zxvf icu-release-78.1.tar.gz
cd icu-release-78.1/icu4c/source
./runConfigureICU Linux --enable-static=no --enable-shared=yes
make -j4
```

- Download the SQLite source code from https://sqlite.org/2025/sqlite-src-3500400.zip , and build SQLite with fts5 support.

```bash
unzip sqlite-src-3500400.zip
cd sqlite-src-3500400
CPPFLAGS="-I ../icu-release-78.1/icu4c/source/common/ -I ../icu-release-78.1/icu4c/source/i18n/" LDFLAGS="-L ../icu-release-78.1/icu4c/source/lib" ./configure --enable-fts5 --with-icu-ldflags="-licui18n -licuuc -licudata"
make
```


# Building

```bash
make 

output: libicu.so
```

# Usage
Load the extension in SQLite

## SQLite CLI
```
.load libicu
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize=icu)
```


## Demo Test

```
make test
LD_LIBRARY_PATH=./ ./test icu
```


# TODO

- [ ] Option stopwords
- [ ] Use locale-specific tokenizers
