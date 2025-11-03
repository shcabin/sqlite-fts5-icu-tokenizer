#include <assert.h>

#include <iostream>
#include <algorithm>
#include <unordered_set>

#include "unicode/brkiter.h"
#include "unicode/ubrk.h"
#include "unicode/utypes.h"
#include "sqlite3ext.h"

SQLITE_EXTENSION_INIT1

extern "C" {
typedef int (*XTokenizer)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd);

static int icuTokenizerCreate(void *pCtx, const char **azArg, int nArg, Fts5Tokenizer **ppTok);
static void icuTokenizerDelete(Fts5Tokenizer *pTok);
static int icuTokenizer(Fts5Tokenizer *pTok, void *pCtx, int flags, const char *pText, int nText, const char *pLocale,
                        int nLocale, XTokenizer xToken);

std::string unicode61(std::string in);


/*
** Return a pointer to the fts5_api pointer for database connection db.
** If an error occurs, return NULL and leave an error in the database
** handle (accessible using sqlite3_errcode()/errmsg()).
*/
fts5_api *fts5_api_from_db(sqlite3 *db) {
    fts5_api *pRet = 0;
    sqlite3_stmt *pStmt = 0;

    if (SQLITE_OK == sqlite3_prepare(db, "SELECT fts5(?1)", -1, &pStmt, 0)) {
        sqlite3_bind_pointer(pStmt, 1, (void *) &pRet, "fts5_api_ptr", NULL);
        sqlite3_step(pStmt);
    }
    sqlite3_finalize(pStmt);
    return pRet;
}

int sqlite3_icu_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);
    fts5_api *fts5api;

    fts5api = fts5_api_from_db(db);
    if (fts5api == NULL) {
        return SQLITE_ERROR;
    }
    assert(fts5api->iVersion >= 3);
    fts5_tokenizer_v2 tokenizer = {2, icuTokenizerCreate, icuTokenizerDelete, icuTokenizer};

    rc = fts5api->xCreateTokenizer_v2(fts5api, "icu", (void *) fts5api, &tokenizer, NULL);
    return rc;
}


typedef struct {
    icu::BreakIterator *pIter;
    UErrorCode status;
    std::unordered_set<std::string> stopwords;
} IcuTokenizer;


static int icuTokenizerCreate(void *pCtx, const char **azArg, int nArg, Fts5Tokenizer **ppTok) {
    UErrorCode status = U_ZERO_ERROR;
    icu::BreakIterator *pIter = icu::BreakIterator::createWordInstance(icu::Locale::getDefault(), status);
    if (U_FAILURE(status)) {
        return SQLITE_ERROR;
    }
    IcuTokenizer *p = (IcuTokenizer *) sqlite3_malloc(sizeof(IcuTokenizer));
    if (!p) return SQLITE_NOMEM;
    p->pIter = pIter;
    p->stopwords = std::unordered_set<std::string>{
        // Chinese
        "的", "是", "了", "与", "即", "这", "和", "就",  "而", "及", "与", "或", "上", "也", "很", "到",  "要",
        // English
        "a", "an", "are", "as", "at", "be", "by", "for", "from", "how", "i", "in", "is", "it", "of", "on", "or", "that",
        "the", "this", "to", "was", "what", "when", "where", "who", "with",
    };
    *ppTok = (Fts5Tokenizer *) p;
    return SQLITE_OK;
}

static void icuTokenizerDelete(Fts5Tokenizer *pTokenizer) {
    if (pTokenizer) {
        IcuTokenizer *p = (IcuTokenizer *) pTokenizer;
        delete p->pIter;
        sqlite3_free(p);
    }
}

static int icuTokenizer(Fts5Tokenizer *pTokenizer, void *pCtx, int flags, const char *pText, int nText,
                        const char *pLocale, int nLocale, XTokenizer xToken) {
    IcuTokenizer *p = reinterpret_cast<IcuTokenizer *>(pTokenizer);
    if (!p || !pText || nText <= 0)
        return SQLITE_OK;
    auto pIter = p->pIter;
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(icu::StringPiece(pText, nText));
    pIter->setText(ustr);

    int32_t start = pIter->first();
    int32_t end = pIter->next();
    int32_t pos_end = start;
    while (end != icu::BreakIterator::DONE) {
        // Unicode to UTF-8
        std::string token;
        ustr.tempSubString(start, end - start).toUTF8String(token);

        int32_t pos_start = pos_end;
        pos_end = pos_start + token.length();

        int32_t token_status = pIter->getRuleStatus();
        bool is_word = (token_status != UBRK_WORD_NONE);
        // c++20: bool is_stopword = p->stopwords.contains(token);
        bool not_stopword = (p->stopwords.find(token) == p->stopwords.end());
        if (is_word && not_stopword) {
            auto result = unicode61(token);
            xToken(pCtx, 0, result.c_str(), result.size(), pos_start, pos_end);
        } else if ((!is_word) && (token == "@")) {
            xToken(pCtx, 0, token.c_str(), token.size(), pos_start, pos_end);
        }

        start = end;
        end = pIter->next();
    }
    return SQLITE_OK;
}
}