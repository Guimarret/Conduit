# Conduit

Conduit is a simple, multi-threaded task scheduler written in C. It lets you schedule and run tasks based on cron expressions, managing everything with threads and storing task info in SQLite.

## What It Does
- Keeps track of tasks and their schedules
- Uses cron syntax to decide when to run tasks
- Runs tasks in separate worker threads
- Saves tasks in an SQLite database
- Executes external programs from a `dags` folder

## How It Works
- **Main thread:** Starts everything and keeps the app alive
- **Scheduler thread:** Checks which tasks should run and when
- **Worker threads:** Run tasks as needed

## Tech Stuff
- Written in C with POSIX threads
- Uses SQLite3 for storing tasks
- Runs external binaries using fork/exec
- Uses thread syncing to handle concurrency

Conduit is a lightweight, no-frills scheduler for running tasks efficiently without the complexity of big workflow systems.

![Conduit Architecture Diagram](images/flow.png)
