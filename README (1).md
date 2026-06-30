# Library Management System (C)

A console-based Library Management System written in C, built for managing
book records, user details, and issue/return transactions using file
handling for permanent storage.

## Features

- **Book Management** – Add, View, Update, Delete book records
- **User Management** – Add, View, Update, Delete user/member records
- **Transactions** – Issue a book to a user, Return a book, View transaction history
- **Persistent storage** – All data is saved to binary files (`books.dat`,
  `users.dat`, `transactions.dat`) so records survive across program runs
- **Auto-incrementing IDs** for books, users, and transactions
- **Available-copy tracking** – issuing a book reduces available copies,
  returning a book restores them

## Concepts Demonstrated

| Concept              | Where it's used                                              |
|-----------------------|---------------------------------------------------------------|
| Structures             | `Book`, `User`, `Transaction`                                |
| File handling           | `fopen`, `fread`, `fwrite`, `fseek` for CRUD operations      |
| Pointers                | Passing struct pointers (`Book *outBook`) to fill data from search functions, generic buffer handling in `getNextId()` |
| Arrays / Buffers         | Character arrays for titles, names, phone numbers           |
| Functions / Modularity  | Each operation (`addBook`, `issueBook`, etc.) is a separate function |
| Dynamic memory           | `malloc`/`free` in `getNextId()`                             |

## Project Structure

```
library_system/
├── library.c     # Complete source code
└── README.md     # This file
```

Running the program creates three data files in the same directory:
```
books.dat
users.dat
transactions.dat
```

## How to Compile & Run

### On Linux / macOS (gcc)
```bash
gcc -Wall -o library library.c
./library
```

### On Windows (MinGW / gcc)
```bash
gcc -Wall -o library.exe library.c
library.exe
```

### Using Code::Blocks / Dev-C++ / Turbo C
Simply create a new C console project, add `library.c` as the source file,
and build/run.

## Menu Overview

```
LIBRARY MANAGEMENT SYSTEM
1. Book Management
   1. Add Book
   2. View All Books
   3. Update Book
   4. Delete Book
2. User Management
   1. Add User
   2. View All Users
   3. Update User
   4. Delete User
3. Transactions (Issue/Return)
   1. Issue Book
   2. Return Book
   3. View All Transactions
4. View All Books
5. View All Users
6. View All Transactions
0. Exit
```

## Sample Workflow

1. Go to **Book Management → Add Book** to add a title (e.g. "The C
   Programming Language", 5 copies).
2. Go to **User Management → Add User** to register a member.
3. Go to **Transactions → Issue Book**, enter the Book ID and User ID —
   available copies for that book drop by 1 and a transaction record with
   status `ISSUED` is created.
4. Go to **Transactions → Return Book**, enter the Transaction ID —
   the book's available copies go back up and the transaction status
   changes to `RETURNED`.
5. All data is saved automatically; close and reopen the program and the
   records will still be there.

## Notes / Possible Extensions

- Due dates and fine calculation could be added to the `Transaction` struct.
- Search-by-title/author could be added using linear search over `books.dat`.
- The current "delete" implementation rewrites the file without the deleted
  record (common technique for fixed-size record files without a database).
- Record IDs are recomputed as `max existing ID + 1`, so IDs are not reused
  after deletion within the same session in a colliding way.
