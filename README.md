# ChronoDB

A lightweight, version-controlled key-value database engine built in **C**. ChronoDB provides a terminal-based interface for organizing typed key-value data, tracking changes, rolling back values, and saving database state to disk.

## Project Proposal

### Project Description

ChronoDB is a command-line database engine implemented in C. It organizes data using an **Entity → Record → Key/Value** model. In addition to basic data operations, it supports value history, rollback, transactions, snapshots, and persistent storage.

### Goals

- Implement core database operations in C.
- Organize data into entities and records containing key-value entries.
- Track changes to values and support rollback to earlier versions.
- Provide transaction controls to commit or abort a group of changes.
- Create, list, and restore database snapshots.
- Save and load database state using a custom text-based format.
- Practice modular C programming, file handling, validation, and automated testing.

### Specifications

| Item | Specification |
|---|---|
| Language | C |
| Interface | Interactive command-line shell |
| Compiler | GCC |
| Build tool | Make |
| Data model | Entities → Records → Key/Value entries |
| Supported value types | String, integer, float, boolean |
| Persistence | Custom text-based database file |
| Tests | Automated C storage tests |

### Design

ChronoDB separates the command-line interface and command parsing from the storage layer. The storage layer manages entities, records, typed values, history, rollback, transactions, snapshots, and persistence. Automated storage tests check core operations and error handling.

## Features

- **Entity and record management:** create entities and records, and switch the active record.
- **Key-value operations:** set new keys, update existing keys, retrieve values, delete keys, and list stored entries.
- **Typed values:** support strings, integers, floating-point numbers, and booleans.
- **Search:** search stored data using case-insensitive matching.
- **Version history:** inspect a key's history and roll back to an earlier value.
- **Transactions:** use `BEGIN`, `COMMIT`, and `ABORT`.
- **Snapshots:** create snapshots, list them, and restore a snapshot.
- **Statistics:** view database statistics with `STATS`.
- **Persistence:** save the database to `data/chronodb.db` and load it when the program starts.
- **Automated tests:** build and run the storage test suite using Make.

## Project Structure

```text
ChronoDB/
├── data/
│   └── chronodb.db          # Local database file (created/updated by the program)
├── docs/                    # Project documentation
├── include/
│   └── storage.h            # Storage declarations
├── src/
│   ├── main.c               # CLI entry point
│   ├── parser.c             # Command parsing and dispatch
│   └── storage.c            # Storage and database operations
├── tests/
│   └── test_storage.c       # Storage tests
├── .gitignore
├── Makefile
└── README.md
```

## Requirements

- Linux or another environment with GCC and Make installed.
- A C compiler compatible with the project's C source files.

On Ubuntu, install the build tools with:

```bash
sudo apt update
sudo apt install build-essential
```

## Build and Run

From the project root, compile ChronoDB:

```bash
make
```

Start the command-line interface:

```bash
./chronodb
```

To remove compiled binaries and common build artifacts:

```bash
make clean
```

## Run Tests

Run the automated storage tests:

```bash
make test
```

A successful test run prints:

```text
All ChronoDB storage tests passed.
```

## Command Reference

Commands are entered in the ChronoDB shell. Exact command syntax depends on the command parser; the examples below illustrate the main workflow.

| Command | Purpose |
|---|---|
| `CREATE ENTITY <name>` | Create an entity and select its initial record. |
| `CREATE RECORD` | Create and select another record in the current entity. |
| `USE RECORD <id>` | Switch to a record by ID. |
| `SET <key> <value>` | Add a new key-value entry. |
| `UPDATE <key> <value>` | Change the value of an existing key. |
| `GET <key>` | Retrieve a value. |
| `DELETE <key>` | Delete a key. |
| `LIST` | List entries in the active record. |
| `SEARCH <term>` | Search stored data. |
| `HISTORY <key>` | Show a key's version history. |
| `ROLLBACK <key> <version>` | Restore a key to an earlier version. |
| `BEGIN` | Start a transaction. |
| `COMMIT` | Commit a transaction. |
| `ABORT` | Abort a transaction. |
| `SNAPSHOT CREATE <name>` | Create a named snapshot. |
| `SNAPSHOT LIST` | List available snapshots. |
| `SNAPSHOT RESTORE <name>` | Restore a named snapshot. |
| `STATS` | Display database statistics. |

## Example Workflow

The following is an illustrative session; output and generated record IDs may differ.

```text
CREATE ENTITY students
SET name "Aarav"
SET age 21
GET name
UPDATE age 22
HISTORY age
STATS
```

Use `CREATE RECORD` to add another record to the selected entity. Use `USE RECORD <id>` to switch records when needed.

## Data Persistence

ChronoDB stores its database state in `data/chronodb.db` using its custom text-based format. The program loads saved state when it starts and writes state to disk through its persistence workflow. Do not manually edit the database file unless you understand the format.

## Testing Status

The project has been built with GCC using warning flags (`-Wall -Wextra`), and the storage test suite has passed with:

```text
All ChronoDB storage tests passed.
```

Run `make test` after making changes to check the storage layer again.

## Current Scope and Limitations

ChronoDB is a learning-oriented database engine, not a production database. Its current design uses fixed-size in-memory structures and a basic command parser. It is intended for local, single-process use and does not provide production-grade concurrency, networking, authentication, or crash-recovery guarantees.

## Possible Future Improvements

- Replace fixed-size structures with dynamic memory allocation.
- Add more advanced indexing and query capabilities.
- Improve parsing, quoting, and input validation.
- Add stronger crash recovery and persistence guarantees.
- Expand tests for edge cases and command-level behavior.

## License

No license is specified in this README. Add a `LICENSE` file to the repository if you want to publish the project under an explicit open-source license.
