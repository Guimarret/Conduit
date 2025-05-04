#include <sqlite3.h>

sqlite3* initialize_database();
void shutdown_database(sqlite3 *db);
sqlite3* dag_migration(sqlite3 *db);
