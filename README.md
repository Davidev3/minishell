# Sistema de cadastro no terminal

Aplicação de estudo em C com interface `ncurses` para cadastrar, listar, buscar, editar e remover registros. Usa `struct`, array de até 100 registros, busca por nome ou cidade e exclusão lógica.

> Os registros ficam somente na memória durante a execução. Não há banco de dados nem persistência em arquivo.

## Compilar e executar

Instale um compilador C e a biblioteca de desenvolvimento ncurses (no Debian/Ubuntu: `sudo apt install gcc libncurses-dev`). Em um terminal com suporte a cores:

```bash
gcc -std=c11 -Wall -Wextra main.c -lncurses -o cadastro
./cadastro
```

No menu, use as setas, Enter ou as teclas 1–5; pressione Q para sair.

## Funcionalidades

| Tecla | Operação |
|---|---|
| 1 | Novo cadastro |
| 2 | Listar registros ativos |
| 3 | Buscar por nome ou cidade |
| 4 | Editar registro por ID |
| 5 | Remover registro por ID (exclusão lógica) |
| Q | Sair |

O arquivo `main.c` contém a interface e a lógica do cadastro. Este projeto é um painel de cadastro no terminal, embora o nome do repositório seja `minishell`.
