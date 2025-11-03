# Why Build Another Tokenizer?

While GitHub hosts a wealth of tokenizers, most suffer from two key limitations: they either support only specific languages or demand explicit language specification upfront.
My goal is to create a tokenizer that works seamlessly in uncertain language environments—particularly in mixed-language scenarios.

# sqlite-fts5-icu-tokenizer

The SQLite FTS5 extension provides International Components for Unicode (ICU) based tokenization for full-text search, support non-space-separated languages such as Chinese, Japanese. 

Furthermore, **it references the Unicode61 Tokenizer 'remove_diacritics=2' feature**, where by default, diacritics are removed from all Latin script characters. for example, "A", "a", "À", "à", "Â" and "â" are all considered to be equivalent.

The implementation fully complies with the FTS5 v2 API specifications and is written in C++.



## Prerequisites

- Download the SQLite source code from https://sqlite.org/2025/sqlite-src-3500400.zip
- Install ICU and SQLite3 development libraries. On Ubuntu, you can use:
  ```bash
  sudo apt-get install libicu-dev libsqlite3-dev 
  ```

# Building

```bash
 make

output: libicu.so
```

# Usage
Load the extension in SQLite

```
.load libicu
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize=icu)
```

Alternatively, 
```
make test
LD_LIBRARY_PATH=./ ./test icu
```


# TODO

- [ ] Option stopwords
- [ ] Use locale-specific tokenizers
