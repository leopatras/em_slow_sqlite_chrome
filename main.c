#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <emscripten.h>
#include "sqlite3.h"

EM_JS(char*,addPersistentDirsInt,(),{
  const dirname="/persist";
  try {
    FS.mkdir(dirname);
  } catch(err) {
    console.error("mkdir %o failed:%o",dirname,err);
    return;
  }
  try {
    FS.mount(IDBFS, { autoPersist: true }, dirname);
    console.log("did mount IDBFS to "+dirname);
  } catch (err) {
    console.error("mount(IDBFS,{ autoPersist: true },%o) error:%o",dirname,err);
  }
});


static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    NotUsed = 0;
    /*
       int i;
       for (i = 0; i < argc; i++) {
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
       }
       printf("\n");
     */
    return 0;
}

int main(int argc, char **argv)
{
    sqlite3 *db;
    char *err = 0;
    int rc,i,loop;
    struct timeval stop, start;
    addPersistentDirsInt();
    for (loop = 1; loop <= 20; loop++) {
        gettimeofday(&start, NULL);
        remove("/persist/test.db");
        rc = sqlite3_open("/persist/test.db", &db);
        if (rc == SQLITE_OK) {
            rc = sqlite3_exec(db,
                              "CREATE TABLE test (id INTEGER NOT NULL, text VARCHAR(100))",
                              callback, 0, &err);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error CREATE: %s\n", err);
            }
            for (i = 1; i < 100; i++) {
                char buf[500];
                sprintf(buf, "INSERT INTO test VALUES (%d, 'text%d')", i,
                        i);
                rc = sqlite3_exec(db, buf, callback, 0, &err);
                if (rc != SQLITE_OK) {
                    fprintf(stderr, "SQL error INSERT: %s\n", err);
                }
            }
            rc = sqlite3_exec(db, "SELECT * FROM test", callback, 0, &err);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error SELECT: %s\n", err);
            }
            rc = sqlite3_exec(db, "DELETE FROM  test", callback, 0, &err);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error DELETE: %s\n", err);
            }
            rc = sqlite3_exec(db, "DROP TABLE test", callback, 0, &err);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error DROP: %s\n", err);
            }
            sqlite3_close(db);
            gettimeofday(&stop, NULL);
            printf("loop:%d db ops took %lld msec\n",loop,
                   (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec -
                   start.tv_usec);
        } else {
            fprintf(stderr, "Can't open database: %s\n",
                    sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
    }
    return 0;
}
