# C Database Engine with Perfect Indexing

## Overview
This project is a C-based, file-backed relational database engine with a command-line interface. It supports core SQL-like operations and features **perfect indexing** using B+ trees for all queries with WHERE clauses, ensuring optimal performance for lookups and range queries.

## Core Features
- **Table Management:** Create, drop, and evolve tables with flexible schemas.
- **Data Manipulation:** Insert, input, update, and delete rows with type-checked columns.
- **Advanced Querying:** SELECT supports complex WHERE clauses (AND/OR, ranges, inequalities).
- **Perfect Indexing:** Every SELECT/UPDATE/DELETE with a WHERE clause uses a B+ tree index for each involved column, guaranteeing optimal search performance.
- **Binary Storage:** Data and schema are stored in binary files for efficiency.
- **Extensible Design:** Modular command and utility structure for easy feature expansion.

## Command Reference
- `create <table> (col:type, ...)` — Create a new table.
- `input <table> (col:val, ...)` — Insert a row.
- `insert <table> (col:type, ...)` — Add columns to a table.
- `select <table> (col=val && col2>val2 || ...)` — Query with full index support.
- `update <table> (<where>) values (col:val, ...)` — Update rows matching a condition.
- `delete <table> (<where>)` — Delete rows matching a condition.
- `drop <table>` — Drop a table.
- `exit` — Exit the CLI.

## Build & Run
1. **Requirements:** GCC (or compatible C compiler)
2. **Build:**
   ```sh
   make
   ```
3. **Run:**
   ```sh
   ./myDb.exe   # or myDb.exe on Windows
   ```

## File Structure
- `main.c` — Entry point, command loop.
- `commands/` — Command handlers (create, insert, select, etc.).
- `db/schemas/` — Schema definitions, B+ tree index, and utilities.
- `db/tables/` — Table data files (created at runtime).

## Indexing Details
- B+ tree implementation (`db/schemas/bplustree.c/h`) is used for all indexed operations.
- Indexes are built in-memory for each query, ensuring up-to-date and perfect coverage.
- Supports equality, range, and compound conditions.

## Why "Perfect Indexing"?
- Every query is as fast as possible for the given data size and memory, with no missed optimizations.
- No need for manual index management—indexes are always correct and up-to-date.

## Example Usage
```
>> create users (id:int, name:str, age:int)
>> input users (id:1, name:"Alice", age:30)
>> input users (id:2, name:"Bob", age:25)
>> select users (age>20 && name!="Charlie")
>> update users (id=2) values (age:26)
>> delete users (age<25)
>> drop users
>> exit
```

---

**Enjoy blazing-fast queries with perfect indexing!**
