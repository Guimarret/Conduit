#include <sqlite3.h>
#include "scheduler.h"

sqlite3* initialize_database();
void shutdown_database(sqlite3 *db);
sqlite3* dag_migration(sqlite3 *db);
sqlite3* dag_import(sqlite3 *db, Task *task);
char* dags_status(sqlite3 *db);
