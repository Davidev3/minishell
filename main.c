/*
 * PAINEL DE CADASTRO - C com ncurses
 * Funcionalidades: cadastrar, listar, buscar, editar, deletar
 */

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_REGISTROS 100
#define TAM_NOME      50
#define TAM_EMAIL     60
#define TAM_FONE      20
#define TAM_CPF       15
#define TAM_CIDADE    40

/* ── Estrutura do registro ── */
typedef struct {
    int  id;
    char nome[TAM_NOME];
    char email[TAM_EMAIL];
    char fone[TAM_FONE];
    char cpf[TAM_CPF];
    char cidade[TAM_CIDADE];
    int  ativo;
} Pessoa;

/* ── Estado global ── */
static Pessoa db[MAX_REGISTROS];
static int    total    = 0;
static int    prox_id  = 1;

/* ══════════════════════════════════════════════
   UTILITÁRIOS DE JANELA
══════════════════════════════════════════════ */

static void desenha_borda_titulo(WINDOW *win, const char *titulo) {
    box(win, 0, 0);
    int largura = getmaxx(win);
    int pos = (largura - (int)strlen(titulo) - 4) / 2;
    if (pos < 1) pos = 1;
    mvwprintw(win, 0, pos, "[ %s ]", titulo);
}

static void msg_status(WINDOW *status_win, const char *msg, int cor) {
    werase(status_win);
    wattron(status_win, COLOR_PAIR(cor) | A_BOLD);
    mvwprintw(status_win, 0, 2, " %s ", msg);
    wattroff(status_win, COLOR_PAIR(cor) | A_BOLD);
    wrefresh(status_win);
}

/* Lê string com edição básica; retorna 0 se ESC */
static int le_campo(WINDOW *win, int y, int x, char *buf, int maxlen, int largura) {
    int pos = strlen(buf);
    curs_set(1);
    wmove(win, y, x);

    while (1) {
        /* Redesenha campo */
        mvwprintw(win, y, x, "%-*s", largura, "");
        mvwprintw(win, y, x, "%.*s", largura, buf);
        wmove(win, y, x + pos);
        wrefresh(win);

        int ch = wgetch(win);

        if (ch == 27) { curs_set(0); return 0; }               /* ESC */
        if (ch == '\n' || ch == KEY_ENTER) { curs_set(0); return 1; } /* Enter */
        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && pos > 0) {
            buf[--pos] = '\0';
        } else if (ch >= 32 && ch < 256 && pos < maxlen - 1) {
            buf[pos++] = (char)ch;
            buf[pos]   = '\0';
        }
    }
}

/* ══════════════════════════════════════════════
   FORMULÁRIO GENÉRICO (cadastro / edição)
══════════════════════════════════════════════ */

static int formulario(WINDOW *status_win, Pessoa *p, int editando) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int h = 20, w = 60;
    int sy = (rows - h) / 2, sx = (cols - w) / 2;

    WINDOW *form = newwin(h, w, sy, sx);
    keypad(form, TRUE);

    const char *titulo = editando ? "EDITAR REGISTRO" : "NOVO CADASTRO";
    desenha_borda_titulo(form, titulo);

    /* Labels */
    wattron(form, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(form,  2, 3, "Nome    :");
    mvwprintw(form,  4, 3, "E-mail  :");
    mvwprintw(form,  6, 3, "Telefone:");
    mvwprintw(form,  8, 3, "CPF     :");
    mvwprintw(form, 10, 3, "Cidade  :");
    wattroff(form,  COLOR_PAIR(3) | A_BOLD);

    mvwprintw(form, h-3, 3, "[ENTER] confirma  [ESC] cancela  [TAB] avança");

    int campos_y[]    = { 2,  4,  6,  8, 10 };
    char *campos[]    = { p->nome, p->email, p->fone, p->cpf, p->cidade };
    int  tamanhos[]   = { TAM_NOME, TAM_EMAIL, TAM_FONE, TAM_CPF, TAM_CIDADE };
    int  larguras[]   = { 40, 40, 16, 13, 30 };
    int  campo_x      = 14;
    int  atual        = 0;
    int  n_campos     = 5;

    /* Desenha valores iniciais */
    for (int i = 0; i < n_campos; i++) {
        wattron(form, A_UNDERLINE);
        mvwprintw(form, campos_y[i], campo_x, "%-*s", larguras[i], campos[i]);
        wattroff(form, A_UNDERLINE);
    }

    wrefresh(form);
    msg_status(status_win, "Preencha os campos | TAB/ENTER avança | ESC cancela", 5);

    int salvar = 0;
    while (1) {
        /* Destaca campo atual */
        for (int i = 0; i < n_campos; i++) {
            if (i == atual)
                wattron(form, A_REVERSE | A_UNDERLINE);
            else
                wattron(form, A_UNDERLINE);
            mvwprintw(form, campos_y[i], campo_x, "%-*s", larguras[i], campos[i]);
            wattroff(form, A_REVERSE | A_UNDERLINE | A_UNDERLINE);
        }
        wrefresh(form);

        int r = le_campo(form, campos_y[atual], campo_x,
                         campos[atual], tamanhos[atual], larguras[atual]);

        if (r == 0) { /* ESC */
            salvar = 0;
            break;
        }

        atual++;
        if (atual >= n_campos) {
            /* Chegou no último — confirma se nome não vazio */
            if (strlen(p->nome) == 0) {
                msg_status(status_win, "Nome é obrigatório!", 2);
                atual = 0;
            } else {
                salvar = 1;
                break;
            }
        }
    }

    delwin(form);
    touchwin(stdscr);
    refresh();
    return salvar;
}

/* ══════════════════════════════════════════════
   OPERAÇÕES DE DADOS
══════════════════════════════════════════════ */

static void cadastrar(WINDOW *status_win) {
    if (total >= MAX_REGISTROS) {
        msg_status(status_win, "Limite de registros atingido!", 2);
        return;
    }
    Pessoa nova;
    memset(&nova, 0, sizeof(nova));

    if (formulario(status_win, &nova, 0)) {
        nova.id    = prox_id++;
        nova.ativo = 1;
        db[total++] = nova;
        msg_status(status_win, "Cadastro realizado com sucesso!", 1);
    } else {
        msg_status(status_win, "Cadastro cancelado.", 5);
    }
}

static void listar(WINDOW *status_win) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int h = rows - 6, w = cols - 4;
    int sy = 3, sx = 2;

    WINDOW *lst = newwin(h, w, sy, sx);
    keypad(lst, TRUE);
    desenha_borda_titulo(lst, "LISTA DE CADASTROS");

    int por_pag = h - 4;
    int inicio  = 0;

    while (1) {
        werase(lst);
        desenha_borda_titulo(lst, "LISTA DE CADASTROS");

        /* Cabeçalho */
        wattron(lst, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
        mvwprintw(lst, 1, 1, " %-4s %-22s %-28s %-14s ", "ID", "Nome", "E-mail", "Cidade");
        wattroff(lst, COLOR_PAIR(3) | A_BOLD | A_REVERSE);

        int ativos = 0, linha = 2;
        for (int i = 0; i < total && linha < h - 2; i++) {
            if (!db[i].ativo) continue;
            ativos++;
            if (ativos <= inicio) continue;

            mvwprintw(lst, linha, 1, " %-4d %-22.22s %-28.28s %-14.14s ",
                      db[i].id, db[i].nome, db[i].email, db[i].cidade);
            linha++;
        }

        if (ativos == 0) {
            mvwprintw(lst, h/2, (w-20)/2, "Nenhum registro encontrado.");
        }

        mvwprintw(lst, h-2, 2, "[↑↓] rolar  [Q] fechar  |  Total: %d", ativos);
        wrefresh(lst);
        msg_status(status_win, "Navegue com ↑↓ | Q para fechar", 5);

        int ch = wgetch(lst);
        if (ch == 'q' || ch == 'Q' || ch == 27) break;
        if (ch == KEY_DOWN && inicio + por_pag < ativos) inicio++;
        if (ch == KEY_UP   && inicio > 0)                inicio--;
    }

    delwin(lst);
    touchwin(stdscr);
    refresh();
}

static void buscar(WINDOW *status_win) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int h = rows - 6, w = cols - 4;
    WINDOW *bwin = newwin(h, w, 3, 2);
    keypad(bwin, TRUE);
    desenha_borda_titulo(bwin, "BUSCAR");

    char termo[TAM_NOME] = "";
    mvwprintw(bwin, 1, 2, "Busca: ");
    wattron(bwin, A_UNDERLINE);
    mvwprintw(bwin, 1, 9, "%-40s", "");
    wattroff(bwin, A_UNDERLINE);
    wrefresh(bwin);
    msg_status(status_win, "Digite o nome ou cidade para buscar | ESC cancela", 5);

    le_campo(bwin, 1, 9, termo, TAM_NOME, 40);

    if (strlen(termo) == 0) {
        delwin(bwin);
        touchwin(stdscr); refresh();
        msg_status(status_win, "Busca cancelada.", 5);
        return;
    }

    /* Converte para minúsculo */
    char termo_low[TAM_NOME];
    for (int i = 0; termo[i]; i++) termo_low[i] = tolower((unsigned char)termo[i]);
    termo_low[strlen(termo)] = '\0';

    werase(bwin);
    desenha_borda_titulo(bwin, "RESULTADOS DA BUSCA");
    wattron(bwin, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(bwin, 1, 1, " %-4s %-22s %-28s %-14s ", "ID", "Nome", "E-mail", "Cidade");
    wattroff(bwin, COLOR_PAIR(3) | A_BOLD | A_REVERSE);

    int encontrados = 0, linha = 2;
    for (int i = 0; i < total; i++) {
        if (!db[i].ativo) continue;
        char nome_low[TAM_NOME], cid_low[TAM_CIDADE];
        for (int j = 0; db[i].nome[j]; j++) nome_low[j] = tolower((unsigned char)db[i].nome[j]);
        nome_low[strlen(db[i].nome)] = '\0';
        for (int j = 0; db[i].cidade[j]; j++) cid_low[j] = tolower((unsigned char)db[i].cidade[j]);
        cid_low[strlen(db[i].cidade)] = '\0';

        if (strstr(nome_low, termo_low) || strstr(cid_low, termo_low)) {
            if (linha < h - 2) {
                mvwprintw(bwin, linha++, 1, " %-4d %-22.22s %-28.28s %-14.14s ",
                          db[i].id, db[i].nome, db[i].email, db[i].cidade);
            }
            encontrados++;
        }
    }

    if (encontrados == 0) {
        mvwprintw(bwin, h/2, (w-24)/2, "Nenhum resultado encontrado.");
    }
    mvwprintw(bwin, h-2, 2, "[Q] fechar  |  Resultados: %d", encontrados);
    wrefresh(bwin);

    char tmp[80];
    snprintf(tmp, sizeof(tmp), "Busca por '%s': %d resultado(s) | Q fecha", termo, encontrados);
    msg_status(status_win, tmp, encontrados > 0 ? 1 : 2);

    while (wgetch(bwin) != 'q' && wgetch(bwin) != 'Q');

    delwin(bwin);
    touchwin(stdscr); refresh();
}

/* Seleciona registro por ID; retorna índice ou -1 */
static int selecionar_por_id(WINDOW *status_win, const char *acao) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    WINDOW *sw = newwin(5, 40, rows/2-2, (cols-40)/2);
    box(sw, 0, 0);
    char titulo[60];
    snprintf(titulo, sizeof(titulo), "[ %s — Informe o ID ]", acao);
    mvwprintw(sw, 0, 1, "%.*s", 38, titulo);
    mvwprintw(sw, 2, 3, "ID: ");
    wattron(sw, A_UNDERLINE);
    mvwprintw(sw, 2, 7, "      ");
    wattroff(sw, A_UNDERLINE);
    mvwprintw(sw, 3, 3, "[ESC] cancela");
    wrefresh(sw);
    msg_status(status_win, "Digite o ID do registro", 5);

    char buf[10] = "";
    le_campo(sw, 2, 7, buf, 8, 6);
    delwin(sw);
    touchwin(stdscr); refresh();

    if (strlen(buf) == 0) return -1;
    int id = atoi(buf);
    for (int i = 0; i < total; i++) {
        if (db[i].ativo && db[i].id == id) return i;
    }
    msg_status(status_win, "ID não encontrado.", 2);
    return -1;
}

static void editar(WINDOW *status_win) {
    int idx = selecionar_por_id(status_win, "EDITAR");
    if (idx < 0) return;

    Pessoa copia = db[idx];
    if (formulario(status_win, &copia, 1)) {
        copia.id    = db[idx].id;
        copia.ativo = 1;
        db[idx]     = copia;
        msg_status(status_win, "Registro atualizado com sucesso!", 1);
    } else {
        msg_status(status_win, "Edição cancelada.", 5);
    }
}

static void deletar(WINDOW *status_win) {
    int idx = selecionar_por_id(status_win, "DELETAR");
    if (idx < 0) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WINDOW *conf = newwin(7, 50, rows/2-3, (cols-50)/2);
    box(conf, 0, 0);
    mvwprintw(conf, 0, 14, "[ CONFIRMAR EXCLUSÃO ]");
    wattron(conf, A_BOLD);
    mvwprintw(conf, 2, 3, "Remover: %s", db[idx].nome);
    wattroff(conf, A_BOLD);
    mvwprintw(conf, 4, 3, "[S] Confirmar    [N / ESC] Cancelar");
    wrefresh(conf);
    msg_status(status_win, "Confirme a exclusão: S = sim | N/ESC = não", 2);

    int ch = wgetch(conf);
    delwin(conf);
    touchwin(stdscr); refresh();

    if (ch == 's' || ch == 'S') {
        db[idx].ativo = 0;
        msg_status(status_win, "Registro removido.", 1);
    } else {
        msg_status(status_win, "Exclusão cancelada.", 5);
    }
}

/* ══════════════════════════════════════════════
   MENU PRINCIPAL
══════════════════════════════════════════════ */

static void desenha_menu(WINDOW *menu_win, int selecionado) {
    const char *itens[] = {
        "  [1]  Novo Cadastro  ",
        "  [2]  Listar Todos   ",
        "  [3]  Buscar         ",
        "  [4]  Editar         ",
        "  [5]  Deletar        ",
        "  [Q]  Sair           ",
    };
    int n = 6;

    werase(menu_win);
    desenha_borda_titulo(menu_win, "MENU");

    for (int i = 0; i < n; i++) {
        if (i == selecionado)
            wattron(menu_win, COLOR_PAIR(1) | A_BOLD | A_REVERSE);
        else
            wattron(menu_win, COLOR_PAIR(3));
        mvwprintw(menu_win, 2 + i * 2, 1, "%s", itens[i]);
        wattroff(menu_win, COLOR_PAIR(1) | A_BOLD | A_REVERSE | COLOR_PAIR(3));
    }
    wrefresh(menu_win);
}

/* ══════════════════════════════════════════════
   MAIN
══════════════════════════════════════════════ */

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Terminal não suporta cores.\n");
        return 1;
    }
    start_color();
    use_default_colors();

    /* Pares de cor */
    init_pair(1, COLOR_GREEN,  -1);  /* destaque / sucesso */
    init_pair(2, COLOR_RED,    -1);  /* erro / alerta       */
    init_pair(3, COLOR_CYAN,   -1);  /* labels              */
    init_pair(4, COLOR_YELLOW, -1);  /* título              */
    init_pair(5, COLOR_WHITE,  -1);  /* info neutra         */

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    /* ── Layout ── */
    WINDOW *header     = newwin(3,        cols,       0,        0);
    WINDOW *menu_win   = newwin(rows - 5, 24,         3,        0);
    WINDOW *corpo_win  = newwin(rows - 5, cols - 24,  3,       24);
    WINDOW *status_win = newwin(2,        cols,       rows - 2,  0);

    keypad(menu_win, TRUE);

    /* Cabeçalho */
    wattron(header, COLOR_PAIR(4) | A_BOLD);
    box(header, 0, 0);
    const char *cabecalho = "SISTEMA DE CADASTRO  |  Linguagem C + ncurses";
    mvwprintw(header, 1, (cols - (int)strlen(cabecalho)) / 2, "%s", cabecalho);
    wattroff(header, COLOR_PAIR(4) | A_BOLD);
    wrefresh(header);

    /* Borda da área de conteúdo */
    box(corpo_win, 0, 0);
    mvwprintw(corpo_win, getmaxy(corpo_win)/2, (getmaxx(corpo_win)-28)/2,
              "Selecione uma opcao no menu.");
    wrefresh(corpo_win);

    /* Status inicial */
    msg_status(status_win, "Use ↑↓ ou os números para navegar | Q sai", 5);

    int sel = 0, n_itens = 6;
    int rodando = 1;

    while (rodando) {
        desenha_menu(menu_win, sel);

        int ch = wgetch(menu_win);

        switch (ch) {
            case KEY_UP:   sel = (sel - 1 + n_itens) % n_itens; break;
            case KEY_DOWN: sel = (sel + 1)            % n_itens; break;

            case '1': cadastrar(status_win);  break;
            case '2': listar(status_win);     break;
            case '3': buscar(status_win);     break;
            case '4': editar(status_win);     break;
            case '5': deletar(status_win);    break;
            case 'q': case 'Q': rodando = 0;  break;

            case KEY_ENTER: case '\n': case ' ':
                switch (sel) {
                    case 0: cadastrar(status_win);  break;
                    case 1: listar(status_win);     break;
                    case 2: buscar(status_win);     break;
                    case 3: editar(status_win);     break;
                    case 4: deletar(status_win);    break;
                    case 5: rodando = 0;            break;
                }
                break;
        }

        /* Redezenha borda do corpo após cada ação */
        werase(corpo_win);
        box(corpo_win, 0, 0);
        int total_ativos = 0;
        for (int i = 0; i < total; i++) if (db[i].ativo) total_ativos++;
        mvwprintw(corpo_win, 1, 2, "Registros cadastrados: %d", total_ativos);
        wrefresh(corpo_win);
    }

    delwin(header);
    delwin(menu_win);
    delwin(corpo_win);
    delwin(status_win);
    endwin();

    printf("\nSistema encerrado. %d registro(s) na sessao.\n", total);
    return 0;
}