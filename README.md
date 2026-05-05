terminal — bash
$ gcc -o cadastro main.c -lncurses
$ ./cadastro
SISTEMA DE CADASTRO | Linguagem C + ncurses
Visão geral
Estruturas
Operações
Arquitetura
Build
Linguagem   C99
Dependência   libncurses5
Armazenamento   em memória (sem persistência)
Limite   100 registros por sessão
Delete   soft delete — campo ativo = 0
Cores   sucesso erro labels título
Sistema CRUD de terminal com interface em ncurses. Menu navegável por teclado, formulário reutilizável para cadastro e edição, busca case-insensitive e barra de status contextual.
