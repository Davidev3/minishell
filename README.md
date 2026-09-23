# Terminal Registration System in C

A learning project that uses `ncurses` to create, list, search, edit, and soft-delete records from a keyboard-driven terminal interface. It demonstrates structs, arrays, and in-memory state.

> Demo data stays in memory only and disappears when the program exits. The form includes personal-data fields such as email and CPF; use fictional values when trying the project. Do not use it to manage real personal records.

## Build and run

Install a C compiler and ncurses development files. On Debian/Ubuntu: `sudo apt install gcc libncurses-dev`.

```bash
gcc -std=c11 -Wall -Wextra main.c -lncurses -o cadastro
./cadastro
```

Use a color-capable terminal. Navigate with arrow keys, Enter, or keys 1–5; press Q to quit.

## Features

| Key | Action |
|---|---|
| 1 | Create a record |
| 2 | List active records |
| 3 | Search by name or city |
| 4 | Edit by ID |
| 5 | Soft-delete by ID |
| Q | Quit |

The project lives in a repository named `minishell`, but implements a registration panel rather than a command shell.
