sqlite3* transactions_status_migration(sqlite3 *db);
int log_task_status(sqlite3 *db, int task_id, const char *status, const char *details);