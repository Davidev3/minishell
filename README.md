# 📋 Painel de Cadastro — C + ncurses

<div align="center">

![Linguagem](https://img.shields.io/badge/Linguagem-C99-blue?style=flat-square&logo=c)
![Biblioteca](https://img.shields.io/badge/UI-ncurses-green?style=flat-square)
![Build](https://img.shields.io/badge/Build-gcc-orange?style=flat-square&logo=gnu)
![Plataforma](https://img.shields.io/badge/Plataforma-Linux%20%2F%20macOS-lightgrey?style=flat-square)
![Licença](https://img.shields.io/badge/Licença-MIT-purple?style=flat-square)

Sistema **CRUD de terminal** escrito em C puro com interface gráfica em ncurses.  
Gerencie cadastros de pessoas direto no seu terminal — sem banco de dados, sem servidor.

</div>

---

## 📌 Sumário

- [Visão geral](#-visão-geral)
- [Funcionalidades](#-funcionalidades)
- [Arquitetura de janelas](#-arquitetura-de-janelas)
- [Estrutura de dados](#-estrutura-de-dados)
- [Build e execução](#-build-e-execução)
- [Analogias web modernas](#-analogias-web-modernas)
  - [CSS](#-css--tokens-de-cor)
  - [React](#-react--componentes-equivalentes)
  - [Angular](#-angular--serviço-de-dados)

---

## 🖥️ Visão geral

```
┌─────────────────────────────────────────────────────────┐
│      SISTEMA DE CADASTRO  |  Linguagem C + ncurses      │
├────────────────┬────────────────────────────────────────┤
│  [1] Cadastrar │                                        │
│  [2] Listar    │   Área de conteúdo / corpo central     │
│  [3] Buscar    │   Registros cadastrados: 0             │
│  [4] Editar    │                                        │
│  [5] Deletar   │                                        │
│  [Q] Sair      │                                        │
├────────────────┴────────────────────────────────────────┤
│  Use ↑↓ ou os números para navegar | Q sai              │
└─────────────────────────────────────────────────────────┘
```

> **Sem persistência** — os registros vivem apenas em memória RAM durante a sessão.  
> Para salvar dados entre sessões, seria necessário serializar com `fwrite()` / `fread()`.

---

## ✨ Funcionalidades

| Tecla | Ação | Detalhe |
|-------|------|---------|
| `1` | Novo cadastro | Abre formulário zerado com 5 campos |
| `2` | Listar todos | Paginação com `↑ ↓`, exibe apenas registros ativos |
| `3` | Buscar | Case-insensitive por nome ou cidade via `strstr()` |
| `4` | Editar | Seleciona por ID, reutiliza o mesmo formulário de cadastro |
| `5` | Deletar | Soft delete — campo `ativo = 0`, registro permanece no array |
| `Q` | Sair | Libera janelas e chama `endwin()` |

**Dentro dos formulários:**

| Tecla | Ação |
|-------|------|
| `TAB` ou `Enter` | Avança para o próximo campo |
| `ESC` | Cancela e fecha o formulário |
| `Backspace` | Apaga o último caractere |

---

## 🏗️ Arquitetura de janelas

O layout é composto por **4 janelas `WINDOW*` simultâneas e independentes**:

```
newwin(3, cols, 0, 0)          → header       (barra superior)
newwin(rows-5, 24, 3, 0)       → menu_win     (menu lateral)
newwin(rows-5, cols-24, 3, 24) → corpo_win    (área central)
newwin(2, cols, rows-2, 0)     → status_win   (barra de status)
```

Cada janela é gerenciada de forma independente — atualizações em uma não afetam as outras. Esse modelo é o equivalente terminal do conceito de **componentes isolados** no desenvolvimento web.

---

## 📦 Estrutura de dados

```c
#define MAX_REGISTROS 100

typedef struct {
    int  id;
    char nome[50];
    char email[60];
    char fone[20];
    char cpf[15];
    char cidade[40];
    int  ativo;     // 1 = ativo | 0 = soft deleted
} Pessoa;

// Estado global — "banco de dados" em memória
static Pessoa db[MAX_REGISTROS];
static int    total   = 0;
static int    prox_id = 1;
```

### Decisões de design

- **Array estático** — sem alocação dinâmica, sem risco de `malloc` / `free` mal gerenciado
- **Soft delete** — `db[idx].ativo = 0` preserva o registro; a listagem filtra com `if (!db[i].ativo) continue`
- **IDs sequenciais** — `prox_id` nunca reutiliza IDs de registros deletados, como um `AUTO_INCREMENT` de banco de dados

---

## ⚙️ Build e execução

### Pré-requisitos

```bash
# Debian / Ubuntu
sudo apt install gcc libncurses5-dev

# Arch Linux
sudo pacman -S gcc ncurses

# macOS (Homebrew)
brew install ncurses
```

### Compilar

```bash
gcc -o cadastro main.c -lncurses -Wall -Wextra
```

### Executar

```bash
./cadastro
```

> **Requisito:** terminal com suporte a cores (`TERM=xterm-256color` ou equivalente).  
> Se o terminal não suportar cores, o programa exibe um erro e encerra automaticamente.

---

## 🌐 Analogias web modernas

O código C resolve os mesmos problemas de UI que frameworks modernos resolvem — só que no terminal. As seções abaixo mostram a equivalência conceitual.

---

### 🎨 CSS — tokens de cor

O ncurses usa `COLOR_PAIR(n)` como sistema de **design tokens** — exatamente o que fazemos com variáveis CSS:

```c
// C — ncurses color pairs
init_pair(1, COLOR_GREEN,  -1);  /* sucesso */
init_pair(2, COLOR_RED,    -1);  /* erro    */
init_pair(3, COLOR_CYAN,   -1);  /* labels  */
init_pair(4, COLOR_YELLOW, -1);  /* título  */

// Uso:
wattron(win, COLOR_PAIR(1) | A_BOLD);
mvwprintw(win, 0, 2, " Cadastro realizado! ");
wattroff(win, COLOR_PAIR(1) | A_BOLD);
```

```css
/* CSS — equivalente com variáveis */
:root {
  --color-success: #1D9E75;
  --color-danger:  #E24B4A;
  --color-label:   #378ADD;
  --color-title:   #EF9F27;
}

/* status bar — análogo a msg_status() */
.status          { color: var(--color-success); font-weight: 500; }
.status.error    { color: var(--color-danger); }

/* painel com título — análogo a desenha_borda_titulo() */
.panel {
  border: 1px solid var(--color-border);
  border-radius: 8px;
  position: relative;
}
.panel-title {
  position: absolute;
  top: -11px;
  background: white;
  padding: 0 8px;
  color: var(--color-title);
}
```

---

### ⚛️ React — componentes equivalentes

#### `formulario()` → componente controlado

```c
// C — flag editando diferencia cadastro de edição
static int formulario(WINDOW *status_win, Pessoa *p, int editando) { ... }
```

```jsx
// React — prop editando cumpre o mesmo papel
function PessoaForm({ pessoa, editando, onSave, onCancel }) {
  const [form, setForm] = useState(pessoa ?? {
    nome: '', email: '', fone: '', cpf: '', cidade: ''
  });

  const handleChange = (campo) => (e) =>
    setForm((f) => ({ ...f, [campo]: e.target.value }));

  return (
    <form onSubmit={(e) => { e.preventDefault(); onSave(form); }}>
      <h2>{editando ? 'Editar registro' : 'Novo cadastro'}</h2>

      <label>Nome</label>
      <input value={form.nome} onChange={handleChange('nome')} maxLength={49} required />

      <label>E-mail</label>
      <input value={form.email} onChange={handleChange('email')} maxLength={59} />

      {/* ... demais campos ... */}

      <button type="submit">Salvar</button>
      <button type="button" onClick={onCancel}>Cancelar</button>
    </form>
  );
}
```

#### `le_campo()` → input controlado com atalhos de teclado

```c
// C — lê char a char, trata ESC e Enter manualmente
static int le_campo(WINDOW *win, int y, int x, char *buf, int maxlen, int largura) { ... }
```

```jsx
// React — o browser cuida de tudo; apenas interceptamos ESC
<input
  maxLength={49}
  value={nome}
  onChange={(e) => setNome(e.target.value)}
  onKeyDown={(e) => e.key === 'Escape' && onCancel()}
/>
```

#### `listar()` → lista paginada com estado

```c
// C — paginação manual com variáveis de escopo local
int por_pag = h - 4, inicio = 0;
if (ch == KEY_DOWN && inicio + por_pag < ativos) inicio++;
if (ch == KEY_UP   && inicio > 0)                inicio--;
```

```jsx
// React — paginação via estado
const POR_PAG = 10;

function ListaPessoas({ pessoas }) {
  const [pag, setPag] = useState(0);
  const ativos = pessoas.filter((p) => p.ativo);
  const fatia  = ativos.slice(pag * POR_PAG, (pag + 1) * POR_PAG);

  return (
    <>
      <table>
        <tbody>
          {fatia.map((p) => (
            <tr key={p.id}>
              <td>{p.id}</td>
              <td>{p.nome}</td>
              <td>{p.email}</td>
              <td>{p.cidade}</td>
            </tr>
          ))}
        </tbody>
      </table>

      <button onClick={() => setPag((p) => Math.max(0, p - 1))}>← Anterior</button>
      <span>Página {pag + 1}</span>
      <button onClick={() => setPag((p) => p + 1)}
              disabled={(pag + 1) * POR_PAG >= ativos.length}>
        Próxima →
      </button>
    </>
  );
}
```

#### `msg_status()` → componente de feedback

```c
// C — escreve diretamente na janela de status com cor
static void msg_status(WINDOW *win, const char *msg, int cor) {
    wattron(win, COLOR_PAIR(cor) | A_BOLD);
    mvwprintw(win, 0, 2, " %s ", msg);
    wattroff(win, COLOR_PAIR(cor) | A_BOLD);
}
```

```jsx
// React — componente de toast/status reutilizável
const TIPOS = {
  sucesso: { bg: '#EAF3DE', color: '#27500A' },
  erro:    { bg: '#FCEBEB', color: '#791F1F' },
  info:    { bg: '#E6F1FB', color: '#0C447C' },
};

function StatusBar({ msg, tipo = 'info' }) {
  if (!msg) return null;
  const { bg, color } = TIPOS[tipo];
  return (
    <div style={{ background: bg, color, padding: '6px 12px', fontWeight: 500, borderRadius: 6 }}>
      {msg}
    </div>
  );
}

// Uso:
<StatusBar msg="Cadastro realizado com sucesso!" tipo="sucesso" />
<StatusBar msg="ID não encontrado."              tipo="erro"    />
```

---

### 🅰️ Angular — serviço de dados

O array global `db[]` com as funções de CRUD vira um `@Injectable` service — injetável em qualquer componente:

```c
// C — estado e funções globais
static Pessoa db[MAX_REGISTROS];
static int    total   = 0;
static int    prox_id = 1;

static void cadastrar(WINDOW *status_win) { ... }
static void editar(WINDOW *status_win)    { ... }
static void deletar(WINDOW *status_win)   { ... }
```

```typescript
// Angular — serviço equivalente
import { Injectable } from '@angular/core';

export interface Pessoa {
  id: number;
  nome: string;
  email: string;
  fone: string;
  cpf: string;
  cidade: string;
  ativo: boolean;
}

@Injectable({ providedIn: 'root' })
export class CadastroService {
  private db: Pessoa[] = [];
  private proxId = 1;

  cadastrar(dados: Omit<Pessoa, 'id' | 'ativo'>): void {
    this.db.push({ ...dados, id: this.proxId++, ativo: true });
  }

  listar(): Pessoa[] {
    return this.db.filter((p) => p.ativo);
  }

  buscar(termo: string): Pessoa[] {
    const t = termo.toLowerCase();
    return this.db.filter(
      (p) => p.ativo &&
        (p.nome.toLowerCase().includes(t) || p.cidade.toLowerCase().includes(t))
    );
  }

  editar(id: number, dados: Partial<Pessoa>): void {
    const p = this.db.find((x) => x.id === id);
    if (p) Object.assign(p, dados);
  }

  deletar(id: number): void {
    // soft delete — igual ao código C
    const p = this.db.find((x) => x.id === id);
    if (p) p.ativo = false;
  }
}
```

#### Componente consumindo o serviço

```typescript
@Component({
  selector: 'app-lista',
  template: `
    <table>
      <thead>
        <tr>
          <th>ID</th><th>Nome</th><th>E-mail</th><th>Cidade</th><th></th>
        </tr>
      </thead>
      <tbody>
        <tr *ngFor="let p of pessoas">
          <td>{{ p.id }}</td>
          <td>{{ p.nome }}</td>
          <td>{{ p.email }}</td>
          <td>{{ p.cidade }}</td>
          <td>
            <button (click)="editar(p.id)">Editar</button>
            <button (click)="deletar(p.id)">Remover</button>
          </td>
        </tr>
      </tbody>
    </table>
    <p *ngIf="pessoas.length === 0">Nenhum registro encontrado.</p>
  `,
})
export class ListaComponent implements OnInit {
  pessoas: Pessoa[] = [];

  constructor(private svc: CadastroService) {}

  ngOnInit(): void {
    this.pessoas = this.svc.listar();
  }

  editar(id: number): void {
    // navega para rota de edição
  }

  deletar(id: number): void {
    this.svc.deletar(id);
    this.pessoas = this.svc.listar(); // atualiza a lista
  }
}
```

#### Mapeamento C → Angular

| C | Angular |
|---|---------|
| `static Pessoa db[]` | campo privado no service |
| `static int prox_id` | campo privado `proxId` |
| `db[idx].ativo = 0` | `p.ativo = false` (soft delete) |
| `wgetch(menu_win)` | event binding `(click)`, `(keydown)` |
| `WINDOW *menu_win` | `RouterModule` com rotas nomeadas |
| `msg_status(win, msg, cor)` | componente `<app-toast>` ou `MatSnackBar` |

---

## 📁 Estrutura do projeto

```
cadastro-ncurses/
├── main.c        ← código-fonte único
└── README.md     ← este arquivo
```
