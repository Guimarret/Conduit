# Conduit

[![Development Status](https://img.shields.io/badge/status-in%20development-orange.svg)](https://github.com/guimarret/conduit)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**A modern task scheduling and workflow automation system with DAG (Directed Acyclic Graph) support.**

> **Note:** This project is currently in active development. Features may be incomplete or subject to change. We welcome contributions, bug reports, and feature requests!

## Features

- **Cron-based Task Scheduling** - Schedule tasks using familiar cron expressions
- **DAG Workflow Management** - Create complex workflows with task dependencies
- **Web Dashboard** - Modern React-based interface for managing tasks and DAGs
- **Real-time Monitoring** - Track task executions and view detailed logs
- **SQLite Database** - Persistent storage for tasks, DAGs, and execution history
- **Multi-threaded Architecture** - Concurrent execution of scheduler, DAG processor, and web server
- **RESTful API** - Full API support for programmatic access

## Architecture

Conduit consists of several key components:

- **Core Scheduler**: Legacy cron-based task scheduling system
- **DAG Scheduler**: Advanced workflow management with dependency resolution
- **Web Server**: HTTP API and frontend dashboard (Next.js + React)
- **Database Layer**: SQLite-based persistence with transaction support
- **Logger**: Comprehensive logging system for monitoring and debugging

## Quick Start

### Prerequisites

- GCC compiler
- SQLite3 development libraries
- libcjson development libraries
- Node.js and npm (for the web interface)

### Building and Running

1. **Clone the repository**
   ```bash
   git clone https://github.com/guimarret/conduit.git
   cd conduit
   ```

2. **Build the application**
   ```bash
   make
   ```

3. **Run Conduit**
   ```bash
   ./output
   ```

4. **Start the web interface** (in a separate terminal)
   ```bash
   cd webserver/front
   npm install
   npm run dev
   ```

5. **Access the dashboard**
   Open your browser to `http://localhost:3000`

## Project Structure

```
conduit/
├── main.c                    # Application entry point
├── scheduler.*               # Legacy task scheduling
├── dag.*                     # DAG workflow management
├── dag_scheduler.*           # DAG execution engine
├── webserver.*               # HTTP server and API
├── database.*                # SQLite database operations
├── logger.*                  # Logging system
├── webserver/front/          # Next.js frontend application
│   ├── app/
│   │   ├── components/       # React components
│   │   ├── api/              # API routes
│   │   └── types/            # TypeScript definitions
│   └── components/ui/        # Reusable UI components
├── dags/                     # DAG definition files
└── tests/                    # Test files
```

## Core Features

### Task Scheduling
Create and manage individual tasks with cron expressions:
```c
add_task("Daily Backup", "0 0 * * *", "backup_system");
add_task("Hourly Health Check", "0 * * * *", "check_health");
```

### DAG Workflows
Build complex workflows with task dependencies:
- Define tasks within DAGs
- Set up dependencies between tasks
- Automatic dependency resolution
- Cycle detection and validation
- Parallel execution where possible

### Web Dashboard
- Visual DAG representation
- Task management interface
- Execution monitoring
- Configuration management
- Responsive design with Tailwind CSS

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/api/tasks` | List all tasks |
| `POST` | `/api/tasks` | Create new task |
| `GET` | `/api/dags` | List all DAGs |
| `POST` | `/api/dag` | Create new DAG |
| `GET` | `/api/dag/[id]` | Get DAG details |
| `POST` | `/api/dag/[id]/trigger` | Trigger DAG execution |
| `GET` | `/api/dag/[id]/status` | Get DAG execution status |

## Development

### Prerequisites for Development
- C development environment
- Node.js 18+ and npm
- SQLite3
- Git

### Known Issues & TODOs

- [ ] Complete DAG visualization in the web interface
- [ ] Add user authentication and authorization
- [ ] Implement DAG scheduling based on cron expressions
- [ ] Add more comprehensive error handling
- [ ] Performance optimization for large DAGs
- [ ] Add unit and integration tests
- [ ] Docker containerization
- [ ] Configuration file support

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Issues and pull requests are welcome! For major changes, please open an issue first to discuss what you would like to change.

### Bug Reports
If you find a bug, please open an issue with:
- Description of the bug
- Steps to reproduce
- Expected vs actual behavior
- System information (OS, compiler version, etc.)

### Feature Requests
Have an idea for a new feature? We'd love to hear it! Open an issue with:
- Clear description of the feature
- Use case and motivation
- Proposed implementation (if you have ideas)

---

<div align="center">

**Built using C, SQLite, and React**

[Report Bug](https://github.com/guimarret/conduit/issues) • [Request Feature](https://github.com/guimarret/conduit/issues) • [View Documentation](https://github.com/guimarret/conduit/wiki)

</div>
