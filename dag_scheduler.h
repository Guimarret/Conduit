#ifndef CONDUIT_DAG_SCHEDULER_H
#define CONDUIT_DAG_SCHEDULER_H

#include <sqlite3.h>
#include <pthread.h>
#include "dag.h"
#include "cron.h"
#include "thread.h"

// Context structure for DAG execution threads
typedef struct DAGExecutionContext {
    sqlite3 *db;
    DAG *dag;
} DAGExecutionContext;

// DAG Scheduler Functions
void load_dags_from_database(sqlite3 *db);
int is_dag_time_to_run(const char *cronExpression, struct CronTime now);
int execute_dag(sqlite3 *db, DAG *dag);
int execute_task_sync(ThreadParams *params);
void dag_scheduler(sqlite3 *db);
void* dag_execution_thread(void *arg);
void reload_dags(sqlite3 *db);
int trigger_dag_execution(sqlite3 *db, int dag_id);

#endif