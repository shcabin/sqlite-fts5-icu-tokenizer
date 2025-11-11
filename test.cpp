#include "sqlite3.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>

using namespace std;
using Clock = std::chrono::system_clock;
using ms = std::chrono::duration<double, std::milli>;

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        cout << azColName[i] << ": " << argv[i] << endl;
    }
    if (argc == 0) {
        cout << "### empty" << endl;
    }
    return 0;
}


void handle_rc(sqlite3 *db, int rc, const string msg, char *zErrMsg = nullptr) {
    if (rc != SQLITE_OK) {
        cout << "#sqlite3 rc: " << rc << ", error: " << sqlite3_errmsg(db) << ", msg:" << msg << endl;

        fprintf(stderr, "Error loading extension: %s\n", zErrMsg ? zErrMsg : "unknown error");
        sqlite3_free(zErrMsg);
        exit(rc);
    }
}

void full_query(sqlite3 *db);

int main(int argc, char *argv[]) {
    string tokenizer = "icu";
    if (argc > 1) {
        tokenizer = argv[1];
    }
    string lib = "lib";
    lib += tokenizer;
    cout << "Using tokenizer: " << tokenizer << " from:" << lib << endl;

    sqlite3 *db;
    int rc = sqlite3_open(":memory:", &db);
    handle_rc(db, rc, "open");
    auto before = Clock::now();
    // load extensions
    rc = sqlite3_enable_load_extension(db, 1);
    handle_rc(db, rc, "enable load extension");

    char *zErrMsg = NULL;
    rc = sqlite3_load_extension(db, lib.c_str(), NULL, &zErrMsg);
    handle_rc(db, rc, "load extension", zErrMsg);

    ms load_extension = Clock::now() - before;
    std::cout << load_extension.count() << "ms to load extension" << std::endl;

    string sql = "CREATE VIRTUAL TABLE if not exists t1 USING fts5(x, tokenize =\"" + tokenizer + "\")";
    cout << "#create table sql:" << sql << endl;

    rc = sqlite3_exec(db, sql.c_str(), callback, nullptr, &zErrMsg);
    handle_rc(db, rc, "exec create");

    full_query(db);
}

void full_query(sqlite3 *db) {
    char *zErrMsg = NULL;
    int rc;
    string sql;
    sql = R"V0G0N(insert into t1(x) values ('v Ｐ @ English special&'),('hi@gmail.com 1234 Number!'),
        ('J''aime aller au café.'),('邓紫棋(G.E.M):美丽的泡沫 虽然一剎花火'),('インターネットサービス')
        )V0G0N";
    cout << "sql:" << sql << endl;
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec insert", zErrMsg);

    // case 1: support non-space-separated languages
    sql = "select highlight(t1, 0, '[', ']') as matched1 from t1 where x match '泡沫'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 1");
    sql = "select highlight(t1, 0, '[', ']') as matched1 from t1 where x match 'サービス'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 1");

    // case 2: match diacritics
    sql = "select highlight(t1, 0, '[', ']') as matched2 from t1 where x match 'cafe'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 2");

    // case 3: match special chars @
    sql = "select highlight(t1, 0, '[', ']') as matched3 from t1 where x match '\"@\"'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 3");

    sql = "select highlight(t1, 0, '[', ']') as matched4 from t1 where x match 'special'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 4");

    // case: match number
    sql = "select highlight(t1, 0, '[', ']') as matched5 from t1 where x match '1234'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 5");
    // mismatch
    // sql = "select highlight(t1, 0, '[', ']') as matched5 from t1 where x match '123'";

    sql = "select highlight(t1, 0, '[', ']') as matched6 from t1 where x match 'number'";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    handle_rc(db, rc, "exec select 6");
}
