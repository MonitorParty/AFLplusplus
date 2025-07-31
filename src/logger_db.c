#include <sqlite3.h> 
#include <logger_db.h> 
#include <afl-fuzz.h>

int db_init(afl_db_t *db, const char *filename) {
  if (sqlite3_open(filename, &db->db)) {
    return -1;
  }

  // Create tables
  const char *schema = 
    "CREATE TABLE IF NOT EXISTS testcases ("
    "id INTEGER, " // hlt, remove primary key 
    "value BLOB, "
    "timestamp INTEGER, "
    "crash INTEGER DEFAULT 0, "
    "origin TEXT);"

    "CREATE TABLE IF NOT EXISTS queue ("
    "id INTEGER, "
    "queue_id INTEGER, "
    "timestamp INTEGER);"

    "CREATE TABLE IF NOT EXISTS restarts ("
    "id INTEGER, "
    "eventtype TEXT);";

  if (sqlite3_exec(db->db, schema, 0, 0, 0) != SQLITE_OK) {
    return -2;
  }

  //performance, hopefully. Otherwise, this will die. 
  sqlite3_exec(db->db, "PRAGMA journal_mode = WAL;", NULL, NULL, NULL);
  sqlite3_exec(db->db, "PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
  sqlite3_exec(db->db, "PRAGMA temp_store = MEMORY;", NULL, NULL, NULL);
  sqlite3_exec(db->db, "PRAGMA locking_mode = EXCLUSIVE;", NULL, NULL, NULL);
  sqlite3_exec(db->db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

  // Prepare statements
  sqlite3_prepare_v2(db->db,
    "INSERT INTO testcases (id, value, timestamp, crash, origin) VALUES (?, ?, ?, ?, ?);",
    -1, &db->insert_testcase, 0);

  sqlite3_prepare_v2(db->db,
    "UPDATE testcases SET queue_id = ?, queue_time = ? WHERE id = ?;",
    -1, &db->update_testcase_queue, 0);

  sqlite3_prepare_v2(db->db,
    "INSERT INTO queue (id, queue_id, timestamp) VALUES (?, ?, ?);",
    -1, &db->insert_queue_entry, 0);

  sqlite3_prepare_v2(db->db,
    "INSERT INTO restarts (id, eventtype) VALUES (?, ?);",
    -1, &db->insert_restart, 0);

  return 0;
}

int db_log_testcase(afl_db_t *db, u64 id, const u8 *data, u32 len, u64 timestamp, int is_crash, const char *origin) {
  sqlite3_bind_int64(db->insert_testcase, 1, id);
  sqlite3_bind_blob(db->insert_testcase, 2, data, len, SQLITE_TRANSIENT);
  sqlite3_bind_int64(db->insert_testcase, 3, timestamp);
  sqlite3_bind_int(db->insert_testcase, 4, is_crash);
  sqlite3_bind_text(db->insert_testcase, 5, origin, -1, SQLITE_TRANSIENT);

  int rc = sqlite3_step(db->insert_testcase);
  sqlite3_reset(db->insert_testcase);
  return 0;
  return rc == SQLITE_DONE ? 0 : -1;
}


int db_mark_in_queue(afl_db_t *db, u64 id, u64 queue_id, u64 timestamp) {
  sqlite3_bind_int64(db->update_testcase_queue, 1, queue_id);
  sqlite3_bind_int64(db->update_testcase_queue, 2, timestamp);
  sqlite3_bind_int64(db->update_testcase_queue, 3, id);
  int ret_tc = sqlite3_step(db->update_testcase_queue);
  sqlite3_reset(db->update_testcase_queue);

  sqlite3_bind_int64(db->insert_queue_entry, 1, id);
  sqlite3_bind_int64(db->insert_queue_entry, 2, queue_id);
  sqlite3_bind_int64(db->insert_queue_entry, 3, timestamp);
  int ret_queue = sqlite3_step(db->insert_queue_entry);
  sqlite3_reset(db->insert_queue_entry);

  if(ret_queue == SQLITE_DONE && ret_queue == SQLITE_DONE){
	  return 0;
  }else{
	  return ret_tc;
  }
}

int db_log_restart(afl_db_t *db, u64 id, const char *event_type) {
  sqlite3_bind_int64(db->insert_restart, 1, id);
  sqlite3_bind_text(db->insert_restart, 2, event_type, -1, SQLITE_TRANSIENT);
  sqlite3_step(db->insert_restart);
  sqlite3_reset(db->insert_restart);
  return 0;
}

void db_close(afl_db_t *db) {
  if (!db || !db->db) return;
  sqlite3_finalize(db->insert_testcase);
  sqlite3_finalize(db->update_testcase_queue);
  sqlite3_finalize(db->insert_queue_entry);
  sqlite3_finalize(db->insert_restart);
  sqlite3_close(db->db);
  OKF("closed db!");
}

void db_commit(afl_db_t *db){
	if(!db || !db->db) return;
	sqlite3_exec(db->db, "COMMIT;", NULL, NULL, NULL);
	sqlite3_exec(db->db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}


