#include <sqlite3.h>

sqlite3* initialize_database();
void shutdown_database(sqlite3 *db);
