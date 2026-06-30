/* ============================================================
   LIBRARY MANAGEMENT SYSTEM
   Language : C
   Concepts : structures, file handling, arrays, functions, pointers
   Modules  : Book Management, User Management, Transaction (Issue/Return)
   Storage  : Flat binary files (books.dat, users.dat, transactions.dat)
   ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------- File names ---------------------- */
#define BOOK_FILE "books.dat"
#define USER_FILE "users.dat"
#define TRANS_FILE "transactions.dat"

/* ---------------------- Limits ---------------------- */
#define TITLE_LEN 100
#define AUTHOR_LEN 60
#define NAME_LEN 60
#define MAX_RECORDS 1000   /* used only for in-memory array operations like reports */

/* ============================================================
   STRUCTURES
   ============================================================ */

typedef struct {
    int bookId;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    int totalCopies;
    int availableCopies;
} Book;

typedef struct {
    int userId;
    char name[NAME_LEN];
    char phone[15];
} User;

typedef struct {
    int transId;
    int bookId;
    int userId;
    char status[10];   /* "ISSUED" or "RETURNED" */
} Transaction;

/* ============================================================
   FUNCTION PROTOTYPES
   ============================================================ */

/* Utility */
void clearInputBuffer(void);
int  getNextId(const char *filename, size_t recordSize);
void pause_screen(void);

/* Book module */
void addBook(void);
void viewBooks(void);
void updateBook(void);
void deleteBook(void);
int  findBookById(int bookId, Book *outBook, long *outOffset);

/* User module */
void addUser(void);
void viewUsers(void);
void updateUser(void);
void deleteUser(void);
int  findUserById(int userId, User *outUser, long *outOffset);

/* Transaction module */
void issueBook(void);
void returnBook(void);
void viewTransactions(void);

/* Menus */
void bookMenu(void);
void userMenu(void);
void transactionMenu(void);
void mainMenu(void);

/* ============================================================
   UTILITY FUNCTIONS
   ============================================================ */

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_screen(void) {
    printf("\nPress Enter to continue...");
    clearInputBuffer();
}

/* Generic helper: scans a binary record file and returns the next
   available auto-increment ID (max existing ID + 1). Demonstrates
   file handling + pointer arithmetic via fread into a generic buffer. */
int getNextId(const char *filename, size_t recordSize) {
    FILE *fp = fopen(filename, "rb");
    int maxId = 0;
    if (fp == NULL) return 1; /* file doesn't exist yet */

    void *buffer = malloc(recordSize);
    while (fread(buffer, recordSize, 1, fp) == 1) {
        int currentId = *((int *)buffer); /* first field of every struct is its ID */
        if (currentId > maxId) maxId = currentId;
    }
    free(buffer);
    fclose(fp);
    return maxId + 1;
}

/* ============================================================
   BOOK MODULE  (CRUD)
   ============================================================ */

void addBook(void) {
    Book b;
    FILE *fp = fopen(BOOK_FILE, "ab");
    if (fp == NULL) {
        printf("Error opening book file!\n");
        return;
    }

    b.bookId = getNextId(BOOK_FILE, sizeof(Book));

    printf("\n--- Add New Book ---\n");
    printf("Book Title : ");
    clearInputBuffer();
    fgets(b.title, TITLE_LEN, stdin);
    b.title[strcspn(b.title, "\n")] = '\0';

    printf("Author Name: ");
    fgets(b.author, AUTHOR_LEN, stdin);
    b.author[strcspn(b.author, "\n")] = '\0';

    printf("Total Copies: ");
    scanf("%d", &b.totalCopies);
    b.availableCopies = b.totalCopies;

    fwrite(&b, sizeof(Book), 1, fp);
    fclose(fp);

    printf("\nBook added successfully! (Book ID: %d)\n", b.bookId);
}

void viewBooks(void) {
    Book b;
    FILE *fp = fopen(BOOK_FILE, "rb");
    if (fp == NULL) {
        printf("\nNo books found. Add some books first.\n");
        return;
    }

    printf("\n%-6s %-30s %-20s %-8s %-10s\n", "ID", "Title", "Author", "Total", "Available");
    printf("--------------------------------------------------------------------------\n");

    int count = 0;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        printf("%-6d %-30s %-20s %-8d %-10d\n",
               b.bookId, b.title, b.author, b.totalCopies, b.availableCopies);
        count++;
    }
    if (count == 0) printf("(No records)\n");
    fclose(fp);
}

/* Searches the book file for a given ID. Returns 1 if found and
   fills outBook with the data and outOffset with its byte position
   in the file (needed later for in-place update/delete). */
int findBookById(int bookId, Book *outBook, long *outOffset) {
    Book b;
    FILE *fp = fopen(BOOK_FILE, "rb");
    if (fp == NULL) return 0;

    long pos = 0;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookId == bookId) {
            *outBook = b;
            *outOffset = pos;
            fclose(fp);
            return 1;
        }
        pos += sizeof(Book);
    }
    fclose(fp);
    return 0;
}

void updateBook(void) {
    int id;
    Book b;
    long offset;

    printf("\nEnter Book ID to update: ");
    scanf("%d", &id);

    if (!findBookById(id, &b, &offset)) {
        printf("Book not found!\n");
        return;
    }

    printf("Current Title : %s\n", b.title);
    printf("New Title (leave blank to keep current): ");
    clearInputBuffer();
    char buf[TITLE_LEN];
    fgets(buf, TITLE_LEN, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strcpy(b.title, buf);

    printf("Current Author: %s\n", b.author);
    printf("New Author (leave blank to keep current): ");
    fgets(buf, AUTHOR_LEN, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strcpy(b.author, buf);

    printf("Current Total Copies: %d\n", b.totalCopies);
    printf("New Total Copies (-1 to keep current): ");
    int newTotal;
    scanf("%d", &newTotal);
    if (newTotal >= 0) {
        int issuedCopies = b.totalCopies - b.availableCopies;
        b.totalCopies = newTotal;
        b.availableCopies = newTotal - issuedCopies;
        if (b.availableCopies < 0) b.availableCopies = 0;
    }

    FILE *fp = fopen(BOOK_FILE, "rb+");
    fseek(fp, offset, SEEK_SET);
    fwrite(&b, sizeof(Book), 1, fp);
    fclose(fp);

    printf("\nBook updated successfully!\n");
}

void deleteBook(void) {
    int id;
    printf("\nEnter Book ID to delete: ");
    scanf("%d", &id);

    FILE *fp = fopen(BOOK_FILE, "rb");
    if (fp == NULL) {
        printf("No records found!\n");
        return;
    }

    FILE *temp = fopen("temp.dat", "wb");
    Book b;
    int found = 0;

    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookId == id) {
            found = 1;  /* skip writing this record -> effectively deletes it */
            continue;
        }
        fwrite(&b, sizeof(Book), 1, temp);
    }
    fclose(fp);
    fclose(temp);

    remove(BOOK_FILE);
    rename("temp.dat", BOOK_FILE);

    if (found)
        printf("\nBook deleted successfully!\n");
    else
        printf("\nBook ID not found!\n");
}

/* ============================================================
   USER MODULE  (CRUD)
   ============================================================ */

void addUser(void) {
    User u;
    FILE *fp = fopen(USER_FILE, "ab");
    if (fp == NULL) {
        printf("Error opening user file!\n");
        return;
    }

    u.userId = getNextId(USER_FILE, sizeof(User));

    printf("\n--- Add New User ---\n");
    printf("Name : ");
    clearInputBuffer();
    fgets(u.name, NAME_LEN, stdin);
    u.name[strcspn(u.name, "\n")] = '\0';

    printf("Phone: ");
    fgets(u.phone, 15, stdin);
    u.phone[strcspn(u.phone, "\n")] = '\0';

    fwrite(&u, sizeof(User), 1, fp);
    fclose(fp);

    printf("\nUser added successfully! (User ID: %d)\n", u.userId);
}

void viewUsers(void) {
    User u;
    FILE *fp = fopen(USER_FILE, "rb");
    if (fp == NULL) {
        printf("\nNo users found. Add some users first.\n");
        return;
    }

    printf("\n%-6s %-25s %-15s\n", "ID", "Name", "Phone");
    printf("---------------------------------------------\n");

    int count = 0;
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        printf("%-6d %-25s %-15s\n", u.userId, u.name, u.phone);
        count++;
    }
    if (count == 0) printf("(No records)\n");
    fclose(fp);
}

int findUserById(int userId, User *outUser, long *outOffset) {
    User u;
    FILE *fp = fopen(USER_FILE, "rb");
    if (fp == NULL) return 0;

    long pos = 0;
    while (fread(&u, sizeof(User), 1, fp) == 1) {
        if (u.userId == userId) {
            *outUser = u;
            *outOffset = pos;
            fclose(fp);
            return 1;
        }
        pos += sizeof(User);
    }
    fclose(fp);
    return 0;
}

void updateUser(void) {
    int id;
    User u;
    long offset;

    printf("\nEnter User ID to update: ");
    scanf("%d", &id);

    if (!findUserById(id, &u, &offset)) {
        printf("User not found!\n");
        return;
    }

    printf("Current Name : %s\n", u.name);
    printf("New Name (leave blank to keep current): ");
    clearInputBuffer();
    char buf[NAME_LEN];
    fgets(buf, NAME_LEN, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strcpy(u.name, buf);

    printf("Current Phone: %s\n", u.phone);
    printf("New Phone (leave blank to keep current): ");
    fgets(buf, 15, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strcpy(u.phone, buf);

    FILE *fp = fopen(USER_FILE, "rb+");
    fseek(fp, offset, SEEK_SET);
    fwrite(&u, sizeof(User), 1, fp);
    fclose(fp);

    printf("\nUser updated successfully!\n");
}

void deleteUser(void) {
    int id;
    printf("\nEnter User ID to delete: ");
    scanf("%d", &id);

    FILE *fp = fopen(USER_FILE, "rb");
    if (fp == NULL) {
        printf("No records found!\n");
        return;
    }

    FILE *temp = fopen("temp.dat", "wb");
    User u;
    int found = 0;

    while (fread(&u, sizeof(User), 1, fp) == 1) {
        if (u.userId == id) {
            found = 1;
            continue;
        }
        fwrite(&u, sizeof(User), 1, temp);
    }
    fclose(fp);
    fclose(temp);

    remove(USER_FILE);
    rename("temp.dat", USER_FILE);

    if (found)
        printf("\nUser deleted successfully!\n");
    else
        printf("\nUser ID not found!\n");
}

/* ============================================================
   TRANSACTION MODULE  (Issue / Return)
   ============================================================ */

void issueBook(void) {
    int bookId, userId;
    Book b;
    User u;
    long bOffset, uOffset;

    printf("\nEnter Book ID to issue: ");
    scanf("%d", &bookId);
    printf("Enter User ID: ");
    scanf("%d", &userId);

    if (!findBookById(bookId, &b, &bOffset)) {
        printf("Book not found!\n");
        return;
    }
    if (!findUserById(userId, &u, &uOffset)) {
        printf("User not found!\n");
        return;
    }
    if (b.availableCopies <= 0) {
        printf("No copies available for this book right now!\n");
        return;
    }

    /* decrement available copies and save back to book file */
    b.availableCopies--;
    FILE *bfp = fopen(BOOK_FILE, "rb+");
    fseek(bfp, bOffset, SEEK_SET);
    fwrite(&b, sizeof(Book), 1, bfp);
    fclose(bfp);

    /* record the transaction */
    Transaction t;
    t.transId = getNextId(TRANS_FILE, sizeof(Transaction));
    t.bookId = bookId;
    t.userId = userId;
    strcpy(t.status, "ISSUED");

    FILE *tfp = fopen(TRANS_FILE, "ab");
    fwrite(&t, sizeof(Transaction), 1, tfp);
    fclose(tfp);

    printf("\nBook '%s' issued to %s successfully! (Transaction ID: %d)\n",
           b.title, u.name, t.transId);
}

void returnBook(void) {
    int transId;
    printf("\nEnter Transaction ID to return: ");
    scanf("%d", &transId);

    FILE *fp = fopen(TRANS_FILE, "rb+");
    if (fp == NULL) {
        printf("No transactions found!\n");
        return;
    }

    Transaction t;
    long pos = 0;
    int found = 0;

    while (fread(&t, sizeof(Transaction), 1, fp) == 1) {
        if (t.transId == transId && strcmp(t.status, "ISSUED") == 0) {
            found = 1;
            strcpy(t.status, "RETURNED");
            fseek(fp, pos, SEEK_SET);
            fwrite(&t, sizeof(Transaction), 1, fp);
            break;
        }
        pos += sizeof(Transaction);
    }
    fclose(fp);

    if (!found) {
        printf("Transaction not found or already returned!\n");
        return;
    }

    /* increment book's available copies back */
    Book b;
    long bOffset;
    if (findBookById(t.bookId, &b, &bOffset)) {
        b.availableCopies++;
        FILE *bfp = fopen(BOOK_FILE, "rb+");
        fseek(bfp, bOffset, SEEK_SET);
        fwrite(&b, sizeof(Book), 1, bfp);
        fclose(bfp);
    }

    printf("\nBook returned successfully!\n");
}

void viewTransactions(void) {
    Transaction t;
    FILE *fp = fopen(TRANS_FILE, "rb");
    if (fp == NULL) {
        printf("\nNo transactions found.\n");
        return;
    }

    printf("\n%-8s %-8s %-8s %-10s\n", "TransID", "BookID", "UserID", "Status");
    printf("----------------------------------------\n");

    int count = 0;
    while (fread(&t, sizeof(Transaction), 1, fp) == 1) {
        printf("%-8d %-8d %-8d %-10s\n", t.transId, t.bookId, t.userId, t.status);
        count++;
    }
    if (count == 0) printf("(No records)\n");
    fclose(fp);
}

/* ============================================================
   MENUS
   ============================================================ */

void bookMenu(void) {
    int choice;
    do {
        printf("\n========= BOOK MANAGEMENT =========\n");
        printf("1. Add Book\n");
        printf("2. View All Books\n");
        printf("3. Update Book\n");
        printf("4. Delete Book\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addBook(); break;
            case 2: viewBooks(); break;
            case 3: updateBook(); break;
            case 4: deleteBook(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
        if (choice != 0) pause_screen();
    } while (choice != 0);
}

void userMenu(void) {
    int choice;
    do {
        printf("\n========= USER MANAGEMENT =========\n");
        printf("1. Add User\n");
        printf("2. View All Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addUser(); break;
            case 2: viewUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
        if (choice != 0) pause_screen();
    } while (choice != 0);
}

void transactionMenu(void) {
    int choice;
    do {
        printf("\n========= TRANSACTIONS =========\n");
        printf("1. Issue Book\n");
        printf("2. Return Book\n");
        printf("3. View All Transactions\n");
        printf("0. Back to Main Menu\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: issueBook(); break;
            case 2: returnBook(); break;
            case 3: viewTransactions(); break;
            case 0: break;
            default: printf("Invalid choice!\n");
        }
        if (choice != 0) pause_screen();
    } while (choice != 0);
}

void mainMenu(void) {
    int choice;
    do {
        printf("\n=========================================\n");
        printf("       LIBRARY MANAGEMENT SYSTEM\n");
        printf("=========================================\n");
        printf("1. Book Management\n");
        printf("2. User Management\n");
        printf("3. Transactions (Issue/Return)\n");
        printf("4. View All Books\n");
        printf("5. View All Users\n");
        printf("6. View All Transactions\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: bookMenu(); break;
            case 2: userMenu(); break;
            case 3: transactionMenu(); break;
            case 4: viewBooks(); pause_screen(); break;
            case 5: viewUsers(); pause_screen(); break;
            case 6: viewTransactions(); pause_screen(); break;
            case 0: printf("\nExiting... Thank you for using the Library Management System!\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);
}

/* ============================================================
   MAIN
   ============================================================ */

int main(void) {
    mainMenu();
    return 0;
}
