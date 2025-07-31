#ifndef LOGGER_DB_H
#define LOGGER_DB_H

#include <sqlite3.h>
#include "types.h"

typedef struct {
  sqlite3 *db;
  sqlite3_stmt *insert_testcase;
  sqlite3_stmt *update_testcase_queue;
  sqlite3_stmt *insert_queue_entry;
  sqlite3_stmt *insert_restart;
} afl_db_t;

int db_init(afl_db_t *db, const char *filename);
void db_close(afl_db_t *db);
int db_log_testcase(afl_db_t *db, u64 id, const u8 *data, u32 len, u64 timestamp, int is_crash, const char* origin);
int db_mark_in_queue(afl_db_t *db, u64 id, u64 queue_id, u64 timestamp);
int db_log_restart(afl_db_t *db, u64 id, const char *event_type);
void db_commit(afl_db_t *db);

#endif
