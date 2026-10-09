# ChronoDB

ChronoDB is a lightweight, terminal-based, version-controlled key-value database engine written in C.

It is an educational systems project demonstrating database concepts including structured data management, version history, rollback, transactions, snapshots, search, statistics, and persistent storage without relying on an external database engine.

## Features

- **Entity and Record model** — `Entity → Record → Key/Value`
- **CRUD operations** — `SET`, `GET`, `UPDATE`, `DELETE`, `LIST`
- **Typed values** — String, Integer, Float, Boolean
- **Version history** — preserves previous values of keys
- **Rollback** — restores a selected historical version as a new version
- **Transactions** — `BEGIN`, `COMMIT`, `ABORT`
- **Snapshots** — create, list, and restore database snapshots
- **Search** — case-insensitive search across keys and values
- **Statistics** — entity, record, entry, history, and snapshot counters
- **Persistence** — custom `CHRONODB 5` disk format with validation
- **Automated storage tests** — coverage for core storage functionality and validation

## Architecture

```text
ChronoDB
│
├── Entity
│   ├── Record 1
│   │   ├── key → value
│   │   └── key → value
│   │
│   ├── Record 2
│   │   └── key → value
│   │
│   └── ...
│
└── Version History
    ├── Entity context
    ├── Record context
    ├── Key
    ├── Value
    └── Version
```

ChronoDB maintains a current entity and record context. Data operations such as `SET`, `GET`, `UPDATE`, `DELETE`, `SEARCH`, `HISTORY`, and `ROLLBACK` operate on that context.

## Commands

### General

| Command | Description |
|---|---|
| `HELP` | Display available commands |
| `EXIT` | Exit ChronoDB |

### Entities and Records

| Command | Description |
|---|---|
| `CREATE ENTITY <name>` | Create and select an entity |
| `LIST ENTITIES` | List all entities |
| `USE <entity>` | Select an entity |
| `CREATE RECORD` | Create a record in the current entity |
| `LIST RECORDS` | List records in the current entity |
| `USE RECORD <id>` | Select a record |

### Data Operations

| Command | Description |
|---|---|
| `SET <key> <value>` | Create a new key |
| `GET <key>` | Read a key |
| `UPDATE <key> <value>` | Update an existing key |
| `DELETE <key>` | Delete a key |
| `LIST` | List keys and values in the current record |
| `SEARCH <term>` | Search keys and values |

`SET` creates a new key and does not overwrite an existing key. Use `UPDATE` for an existing key.

### Versioning

| Command | Description |
|---|---|
| `HISTORY <key>` | Display the version history of a key |
| `ROLLBACK <key> <version>` | Restore a historical version |

### Transactions

| Command | Description |
|---|---|
| `BEGIN` | Start a transaction |
| `COMMIT` | Keep transaction changes |
| `ABORT` | Discard transaction changes |

### Snapshots and Statistics

| Command | Description |
|---|---|
| `SNAPSHOT CREATE` | Create a snapshot |
| `SNAPSHOT LIST` | List snapshots |
| `SNAPSHOT RESTORE <id>` | Restore a snapshot |
| `STATS` | Display database statistics |

### Persistence

| Command | Description |
|---|---|
| `SAVE` | Save the current database |
| `LOAD` | Load the database from disk |

## Example

```text
chronodb> CREATE ENTITY user
OK: entity 'user' created successfully

chronodb> SET name Avani
OK: key 'name' created successfully

chronodb> SET age 21
OK: key 'age' created successfully

chronodb> GET age
VALUE: 21
TYPE: INTEGER

chronodb> UPDATE name Deeya
OK: key 'name' updated successfully

chronodb> HISTORY name

History for key: name
Version Value
-------------------------
1       Avani
2       Deeya

chronodb> ROLLBACK name 1
OK: key 'name' rolled back to version 1

chronodb> GET name
VALUE: Avani
TYPE: STRING
```

### Transaction

```text
chronodb> BEGIN
OK: transaction started

chronodb> UPDATE name Temporary
OK: key 'name' updated successfully

chronodb> ABORT
OK: transaction aborted
```

### Snapshot

```text
chronodb> SNAPSHOT CREATE
OK: snapshot 1 created

chronodb> UPDATE name Changed
OK: key 'name' updated successfully

chronodb> SNAPSHOT RESTORE 1
OK: snapshot 1 restored successfully
```

## Project Structure

```text
ChronoDB/
├── include/
│   └── storage.h
├── src/
│   ├── main.c
│   ├── parser.c
│   └── storage.c
├── tests/
│   └── test_storage.c
├── data/
│   └── chronodb.db
├── docs/
│   └── design-notes.md
├── .gitignore
└── README.md
```

## Building

From the project root:

```bash
gcc -Wall -Wextra -Iinclude src/storage.c src/parser.c src/main.c -o chronodb
```

Run:

```bash
./chronodb
```

## Running Tests

Compile:

```bash
gcc -Wall -Wextra -Iinclude tests/test_storage.c src/storage.c -o test_storage
```

Run:

```bash
./test_storage
```

Expected result:

```text
All ChronoDB storage tests passed.
```

## Persistence

ChronoDB stores its persistent database in:

```text
data/chronodb.db
```

It uses a custom text-based persistence format rather than SQLite, MySQL, PostgreSQL, or another external database engine.

The current format is identified by:

```text
CHRONODB 5
```

The loader validates the persisted structure before replacing the current in-memory state.

## Testing

The project includes automated storage tests covering:

- CRUD operations
- Typed values
- History and rollback
- Transactions
- Snapshots
- Persistence and validation

The CLI has also been tested through an end-to-end workflow covering the main database features.

## Limitations

- Fixed-size in-memory storage
- Single-process local usage
- Basic command parsing

## Future Improvements

- Dynamic memory allocation
- Advanced indexing and queries
- Improved command parsing
- Concurrency support

## Technologies

- **Language:** C
- **Platform:** Linux / Ubuntu
- **Compiler:** GCC
- **Storage:** Custom text-based persistence format
- **Testing:** C assertions and automated storage tests

## Project Goal

ChronoDB demonstrates core database-engineering concepts at a small scale: data organization, state management, versioning, rollback, transactions, snapshots, persistence, validation, and testing.

It provides practical experience implementing database functionality in C while keeping the internal design understandable and inspectable.
