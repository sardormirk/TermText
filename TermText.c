#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

// OS defines
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// EDITOR CONSTANTS
#define T_TEXT_VERS "0.0.1"
#define TERM_TAB_STOP 8
#define TERM_QUIT_TIMES 3

// macros/data
#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT \
  {               \
    NULL, 0       \
  }
#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

enum editorKey
{
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

enum editorHighlight
{
  HL_NORMAL = 0,
  HL_COMMENT,
  HL_MLCOMMENT,
  HL_KEYWORD1,
  HL_KEYWORD2,
  HL_STRING,
  HL_NUMBER,
  HL_MATCH
};

// data

struct editorSyntax
{
  char *filetype;
  char **filematch;
  char **keywords;
  char *singleline_comment_start;

  char *multiline_comment_start;
  char *multiline_comment_end;

  int flags;
};

typedef struct erow
{
  int id;
  int size;
  int rsize;

  char *chars;
  char *render;
  unsigned char *hl;
  int hl_open_comment;

} erow;

struct editorConfig
{
  int screenrows;
  int screencols;
  int rowoff;
  int coloff;

  int numrows;
  erow *row;

  char *filename;
  char statusmsg[80];
  time_t statusmsg_time;
  int unch;

  int cx, cy;
  int rx;

  struct editorSyntax *syntax;
  struct termios orig_termios;
};

struct editorConfig edt;

struct abuf
{
  char *b;
  int len;
};

bool NORMAL_MODE = true;
bool INSERT_MODE = false;

int command_count = 0;
char prev = '\0';
time_t time_last = 0;

// filetypes

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};

char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL};

struct editorSyntax HLDB[] = {
    {"c", C_HL_extensions, C_HL_keywords, "//", "/*", "*/", HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

// func prototypes

void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen();
char *editorPrompt(char *prompt, void (*callback)(char *, int));

// terminal

void abAppend(struct abuf *ab, const char *s, int len)
{
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL)
    return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab)
{
  free(ab->b);
}

void terminate(const char *s)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode()
{
  if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &edt.orig_termios) == -1)
  {
    terminate("tcsetattr");
  }
}

void enableRawMode()
{

  if (tcgetattr(STDIN_FILENO, &edt.orig_termios))
  {
    terminate("tcgetattr");
  }

  atexit(disableRawMode);

  struct termios raw = edt.orig_termios;

  raw.c_iflag &= ~(IXON | BRKINT | ICRNL | INPCK | ISTRIP);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cflag |= (CS8);
  raw.c_oflag &= ~(OPOST);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
  {
    terminate("tcsetattr");
  }
}

int editorReadKey()
{

  int nread;
  char c, seq[3];

  while ((nread = read(STDIN_FILENO, &c, 1)) == 0)
  {
    if (nread == -1 && errno != EAGAIN)
    {
      terminate("read");
    }
    if (c == '\x1b')
      break;
  }

  while (1)
  {
    switch (c)
    {
    case '\x1b': /* escape sequence handling */
      if (read(STDIN_FILENO, seq, 1) == 0)
        return '\x1b';
      if (read(STDIN_FILENO, seq + 1, 1) == 0)
        return '\x1b';

      if (seq[0] == '[')
      {
        if (seq[1] >= '0' && seq[1] <= '9')
        {

          if (read(STDIN_FILENO, seq + 2, 1) == 0)
            return '\x1b';
          if (seq[2] == '~')
          {
            switch (seq[1])
            {
            case '3':
              return DEL_KEY;
            case '5':
              return PAGE_UP;
            case '6':
              return PAGE_DOWN;
            }
          }
        }
        else
        {
          switch (seq[1])
          {
          case 'A':
            return ARROW_UP;
          case 'B':
            return ARROW_DOWN;
          case 'C':
            return ARROW_RIGHT;
          case 'D':
            return ARROW_LEFT;
          case 'H':
            return HOME_KEY;
          case 'F':
            return END_KEY;
          }
        }
      }

      /* ESC O sequences. */
      else if (seq[0] == 'O')
      {
        switch (seq[1])
        {
        case 'H':
          return HOME_KEY;
        case 'F':
          return END_KEY;
        }
      }
      break;
    default:
      return c;
    }
  }
}

int getCursorPosition(int *rows, int *cols)
{

  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1)
  {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }

  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;

  return 0;
}

int getWindowSize(int *rows, int *cols)
{
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  }
  else
  {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
  }

  return 0;
}

// syntax highlighting

int is_separator(int c)
{
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorUpdateSyntax(erow *row)
{
  row->hl = realloc(row->hl, row->rsize);
  memset(row->hl, HL_NORMAL, row->rsize);

  if (edt.syntax == NULL)
    return;

  char **keywords = edt.syntax->keywords;

  char *scs = edt.syntax->singleline_comment_start;
  char *mcs = edt.syntax->multiline_comment_start;
  char *mce = edt.syntax->multiline_comment_end;

  int mcs_len = mcs ? strlen(mcs) : 0;
  int mce_len = mce ? strlen(mce) : 0;
  int scs_len = scs ? strlen(scs) : 0;

  int prev_sep = 1;
  int in_string = 0;
  int in_comment = (row->id > 0 && edt.row[row->id - 1].hl_open_comment);

  int i = 0;
  while (i < row->rsize)
  {
    char c = row->render[i];
    unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

    if (scs_len && !in_string && !in_comment)
    {
      if (!strncmp(&row->render[i], scs, scs_len))
      {
        memset(&row->hl[i], HL_COMMENT, row->rsize - i);
        break;
      }
    }

    if (mcs_len && mce_len && !in_string)
    {
      if (in_comment)
      {
        row->hl[i] = HL_MLCOMMENT;
        if (!strncmp(&row->render[i], mce, mce_len))
        {
          memset(&row->hl[i], HL_MLCOMMENT, mce_len);
          i += mce_len;
          in_comment = 0;
          prev_sep = 1;
          continue;
        }
        else
        {
          i++;
          continue;
        }
      }
      else if (!strncmp(&row->render[i], mcs, mcs_len))
      {
        memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
        i += mcs_len;
        in_comment = 1;
        continue;
      }
    }

    if (edt.syntax->flags & HL_HIGHLIGHT_STRINGS)
    {
      if (in_string)
      {
        row->hl[i] = HL_STRING;
        if (c == '\\' && i + 1 < row->rsize)
        {
          row->hl[i + 1] = HL_STRING;
          i += 2;
          continue;
        }
        if (c == in_string)
          in_string = 0;
        i++;
        prev_sep = 1;
        continue;
      }
      else
      {
        if (c == '"' || c == '\'')
        {
          in_string = c;
          row->hl[i] = HL_STRING;
          i++;
          continue;
        }
      }
    }

    if (edt.syntax->flags & HL_HIGHLIGHT_NUMBERS)
    {
      if ((isdigit(c)) && (prev_sep || prev_hl == HL_NUMBER) || (c == '.' && prev_hl == HL_NUMBER))
      {
        row->hl[i] = HL_NUMBER;
        i++;
        prev_sep = 0;
        continue;
      }
    }

    if (prev_sep)
    {
      int j;
      for (j = 0; keywords[j]; j++)
      {
        int klen = strlen(keywords[j]);
        int kw2 = keywords[j][klen - 1] == '|';
        if (kw2)
          klen--;
        if (!strncmp(&row->render[i], keywords[j], klen) &&
            is_separator(row->render[i + klen]))
        {
          memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
          i += klen;
          break;
        }
      }
      if (keywords[j] != NULL)
      {
        prev_sep = 0;
        continue;
      }
    }

    prev_sep = is_separator(c);
    i++;
  }

  int changed = (row->hl_open_comment != in_comment);
  row->hl_open_comment = in_comment;
  if (changed && row->id + 1 < edt.numrows)
    editorUpdateSyntax(&edt.row[row->id + 1]);
}

int editorSyntaxToColour(int hl)
{
  switch (hl)
  {
  case HL_COMMENT:
  case HL_MLCOMMENT:
    return 36;
  case HL_KEYWORD1:
    return 33;
  case HL_KEYWORD2:
    return 32;
  case HL_STRING:
    return 35;
  case HL_NUMBER:
    return 31;
  case HL_MATCH:
    return 34;
  default:
    return 37;
  }
}

void editorSelectSyntaxHighlight()
{
  edt.syntax = NULL;
  if (edt.filename == NULL)
    return;
  char *ext = strrchr(edt.filename, '.');
  for (unsigned int j = 0; j < HLDB_ENTRIES; j++)
  {
    struct editorSyntax *s = &HLDB[j];
    unsigned int i = 0;
    while (s->filematch[i])
    {
      int is_ext = (s->filematch[i][0] == '.');
      if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
          (!is_ext && strstr(edt.filename, s->filematch[i])))
      {
        edt.syntax = s;

        int filerow;
        for (filerow = 0; filerow < edt.numrows; filerow++)
        {
          editorUpdateSyntax(&edt.row[filerow]);
        }

        return;
      }
      i++;
    }
  }
}

// editor operations

int editorRowCxtoRx(erow *row, int cx)
{
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++)
  {
    if (row->chars[j] == '\t')
      rx += (TERM_TAB_STOP - 1) - (rx % TERM_TAB_STOP);
    rx++;
  }

  return rx;
}

int editorRowRxtoCx(erow *row, int rx)
{
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++)
  {
    if (row->chars[cx] == '\t')
      cur_rx += (TERM_TAB_STOP - 1) - (cur_rx % TERM_TAB_STOP);
    cur_rx++;

    if (cur_rx > rx)
      return cx;
  }

  return cx;
}

void editorUpdateRow(erow *row)
{
  int tabs = 0;
  int j;

  for (j = 0; j < row->size; j++)
  {
    if (row->chars[j] == '\t')
      tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (TERM_TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++)
  {
    if (row->chars[j] == '\t')
    {
      row->render[idx++] = ' ';
      while (idx % TERM_TAB_STOP != 0)
        row->render[idx++] = ' ';
    }
    else
      row->render[idx++] = row->chars[j];
  }

  row->render[idx] = '\0';
  row->rsize = idx;

  editorUpdateSyntax(row);
}

void editorInsertRow(int idx, char *s, size_t len)
{

  if (idx < 0 || idx > edt.numrows)
    return;

  edt.row = realloc(edt.row, sizeof(erow) * (edt.numrows + 1));
  memmove(&edt.row[idx + 1], &edt.row[idx], sizeof(erow) * (edt.numrows - idx));
  for (int j = idx + 1; j <= edt.numrows; j++)
    edt.row[j].id++;

  edt.row[idx].id = idx;
  edt.row[idx].size = len;
  edt.row[idx].chars = malloc(len + 1);
  memcpy(edt.row[idx].chars, s, len);
  edt.row[idx].chars[len] = '\0';

  edt.row[idx].rsize = 0;
  edt.row[idx].render = NULL;
  edt.row[idx].hl = NULL;
  edt.row[idx].hl_open_comment = 0;
  editorUpdateRow(&edt.row[idx]);
  edt.numrows++;
  edt.unch++;
}

void editorFreeRow(erow *row)
{
  free(row->render);
  free(row->chars);
  free(row->hl);
}

void editorDelRow(int idx)
{
  if (idx < 0 || idx >= edt.numrows)
    return;

  editorFreeRow(&edt.row[idx]);
  memmove(&edt.row[idx], &edt.row[idx + 1], sizeof(erow) * (edt.numrows - idx - 1));
  for (int j = idx; j < edt.numrows - 1; j++)
    edt.row[j].id--;

  edt.numrows--;
  edt.unch++;
}

void editorRowInsertChar(erow *row, int idx, int c)
{
  if (idx < 0 || idx > row->size)
    idx = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[idx + 1], &row->chars[idx], row->size - idx + 1);

  row->size++;
  row->chars[idx] = c;
  editorUpdateRow(row);
  edt.unch++;
}

void editorRowAppendString(erow *row, char *s, size_t len)
{
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  editorUpdateRow(row);
  edt.unch++;
}

void editorRowDelChar(erow *row, int idx)
{
  if (idx < 0 || idx >= row->size)
    return;
  memmove(&row->chars[idx], &row->chars[idx + 1], row->size - idx);
  row->size--;
  editorUpdateRow(row);
  edt.unch++;
}

void editorInsertChar(int c)
{
  if (edt.cy == edt.numrows)
  {
    editorInsertRow(edt.numrows, "", 0);
  }

  editorRowInsertChar(&edt.row[edt.cy], edt.cx, c);
  edt.cx++;
}

void editorInsertNewLine()
{
  if (edt.cx == 0)
  {
    editorInsertRow(edt.cy, "", 0);
  }
  else
  {
    erow *row = &edt.row[edt.cy];
    editorInsertRow(edt.cy + 1, &row->chars[edt.cx], row->size - edt.cx);
    row = &edt.row[edt.cy];
    row->size = edt.cx;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
  }

  edt.cy++;
  edt.cx = 0;
}

void editorDelChar()
{
  if (edt.cy == edt.numrows)
    return;
  if (edt.cx == 0 && edt.cy == 0)
    return;

  erow *row = &edt.row[edt.cy];

  if (edt.cx > 0)
  {
    editorRowDelChar(row, edt.cx - 1);
    edt.cx--;
  }
  else
  {
    edt.cx = edt.row[edt.cy - 1].size;
    editorRowAppendString(&edt.row[edt.cy - 1], row->chars, row->size);
    editorDelRow(edt.cy);
    edt.cy--;
  }
}

// file i/o

char *editorRowsToString(int *buflen)
{
  int totlen = 0;
  int j;
  for (j = 0; j < edt.numrows; j++)
  {
    totlen += edt.row[j].size + 1;
  }
  *buflen = totlen;

  char *buf = malloc(totlen);
  char *p = buf;
  for (int j = 0; j < edt.numrows; j++)
  {
    memcpy(p, edt.row[j].chars, edt.row[j].size);
    p += edt.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void editorSave()
{
  if (edt.filename == NULL)
  {

    edt.filename = editorPrompt("Save as: %s", NULL);
    if (edt.filename == NULL)
    {
      editorSetStatusMessage("Save Aborted");
      return;
    }

    editorSelectSyntaxHighlight();
  }

  int len;
  char *buf = editorRowsToString(&len);

  int fd = open(edt.filename, O_RDWR | O_CREAT, 0644);
  if (fd != -1)
  {
    if (ftruncate(fd, len) != -1)
    {
      if (write(fd, buf, len) == len)
      {
        close(fd);
        free(buf);
        edt.unch = 0;
        editorSetStatusMessage("%d bytes have been written to disk", len);
        return;
      }
    }
    close(fd);
  }
  free(buf);
  editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

// search func

void editorFindCallback(char *query, int key)
{
  static int last_match = -1;
  static int direction = 1;

  static int saved_hl_line;
  static char *saved_hl = NULL;

  if (saved_hl)
  {
    memcpy(edt.row[saved_hl_line].hl, saved_hl, edt.row[saved_hl_line].rsize);
    free(saved_hl);
    saved_hl = NULL;
  }

  if (key == '\r' || key == '\x1b')
  {
    last_match = -1;
    direction = 1;
    return;
  }
  else if (key == ARROW_RIGHT || key == ARROW_DOWN)
  {
    direction = 1;
  }
  else if (key == ARROW_LEFT || key == ARROW_UP)
  {
    direction = -1;
  }
  else
  {
    last_match = -1;
    direction = 1;
  }
  if (last_match == -1)
    direction = 1;
  int current = last_match;
  int i;
  for (i = 0; i < edt.numrows; i++)
  {
    current += direction;
    if (current == -1)
      current = edt.numrows - 1;
    else if (current == edt.numrows)
      current = 0;
    erow *row = &edt.row[current];
    char *match = strstr(row->render, query);
    if (match)
    {
      last_match = current;
      edt.cy = current;
      edt.cx = editorRowRxtoCx(row, match - row->render);
      edt.rowoff = edt.numrows;

      saved_hl_line = current;
      saved_hl = malloc(row->rsize);
      memcpy(saved_hl, row->hl, row->rsize);
      memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
      break;
    }
  }
}

void editorFind()
{
  int saved_cx = edt.cx;
  int saved_cy = edt.cy;
  int saved_coloff = edt.coloff;
  int saved_rowoff = edt.rowoff;
  char *query = editorPrompt("Search: %s (Use ESC/Arrows/Enter)",
                             editorFindCallback);
  if (query)
  {
    free(query);
  }
  else
  {
    edt.cx = saved_cx;
    edt.cy = saved_cy;
    edt.coloff = saved_coloff;
    edt.rowoff = saved_rowoff;
  }
}

// input

char *editorPrompt(char *prompt, void (*callback)(char *, int))
{
  size_t bufsize = 128;
  char *buf = malloc(bufsize);
  size_t buflen = 0;
  buf[0] = '\0';
  while (1)
  {
    editorSetStatusMessage(prompt, buf);
    editorRefreshScreen();
    int c = editorReadKey();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE)
    {
      if (buflen != 0)
        buf[--buflen] = '\0';
    }
    else if (c == '\x1b')
    {
      editorSetStatusMessage("");
      if (callback)
        callback(buf, c);
      free(buf);
      return NULL;
    }
    else if (c == '\r')
    {
      if (buflen != 0)
      {
        editorSetStatusMessage("");
        if (callback)
          callback(buf, c);
        return buf;
      }
    }
    else if (!iscntrl(c) && c < 128)
    {
      if (buflen == bufsize - 1)
      {
        bufsize *= 2;
        buf = realloc(buf, bufsize);
      }
      buf[buflen++] = c;
      buf[buflen] = '\0';
    }
    if (callback)
      callback(buf, c);
  }
}

void editorMoveCursor(int key)
{

  erow *row = (edt.cy >= edt.numrows) ? NULL : &edt.row[edt.cy];

  switch (key)
  {
  case ARROW_LEFT:
    if (edt.cx != 0)
      edt.cx--;
    else if (edt.cy > 0)
    {
      edt.cy--;
      edt.cx = edt.row[edt.cy].size;
    }
    break;
  case ARROW_RIGHT:
    if (row && edt.cx < row->size)
      edt.cx++;
    else if (row && edt.cx == row->size)
    {
      edt.cy++;
      edt.cx = 0;
    }
    break;
  case ARROW_UP:
    if (edt.cy != 0)
      edt.cy--;
    break;
  case ARROW_DOWN:
    if (edt.cy < edt.numrows)
      edt.cy++;
    break;

  case 'h':
    if (edt.cx != 0)
      edt.cx--;
    else if (edt.cy > 0)
    {
      edt.cy--;
      edt.cx = edt.row[edt.cy].size;
    }
    break;
  case 'l':
    if (row && edt.cx < row->size)
      edt.cx++;
    else if (row && edt.cx == row->size)
    {
      edt.cy++;
      edt.cx = 0;
    }
    break;
  case 'k':
    if (edt.cy != 0)
      edt.cy--;
    break;
  case 'j':
    if (edt.cy < edt.numrows)
      edt.cy++;
    break;
  }

  row = (edt.cy >= edt.numrows) ? NULL : &edt.row[edt.cy];
  int rowlen = row ? row->size : 0;
  if (edt.cx > rowlen)
    edt.cx = rowlen;
}

void editorProcessKeypress()
{

  static int quit_times = TERM_QUIT_TIMES;

  int c = editorReadKey();
  switch (c)
  {

  case '\r':
    editorInsertNewLine();
    break;

  case CTRL_KEY('q'):

    if (edt.unch && quit_times > 0)
    {
      editorSetStatusMessage("WARNING: File has unsaved changes. "
                             "Press Ctrl-Q %d more times to quit",
                             quit_times);
      quit_times--;
      return;
    }

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;

  case 'i':
    if (NORMAL_MODE)
    {
      NORMAL_MODE = false;
      INSERT_MODE = true;
    }
    break;

  case '\x1b':
    if (INSERT_MODE)
    {
      NORMAL_MODE = true;
      INSERT_MODE = false;
    }
    break;

  case CTRL_KEY('s'):
    editorSave();
    break;

  case CTRL_KEY('f'):
    editorFind();
    break;

  case HOME_KEY:
    edt.cx = 0;
    break;
  case END_KEY:
    if (edt.cx < edt.numrows)
      edt.cx = edt.row[edt.cy].size;
    break;

  case BACKSPACE:
  case CTRL_KEY('h'):
  case DEL_KEY:
    if (c == DEL_KEY)
      editorMoveCursor(ARROW_RIGHT);
    editorDelChar();

    break;

  case PAGE_UP:
  case PAGE_DOWN:
  {

    if (c == PAGE_UP)
    {
      edt.cy = edt.rowoff;
    }
    else if (c == PAGE_DOWN)
    {
      edt.cy = edt.rowoff + edt.screenrows - 1;
      if (edt.cy > edt.numrows)
        edt.cy = edt.numrows;
    }
    int times = edt.screenrows;
    while (times--)
      editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
  }
  break;
  case ARROW_LEFT:
  case ARROW_RIGHT:
  case ARROW_DOWN:
  case ARROW_UP:
    editorMoveCursor(c);
    break;

  case CTRL_KEY('l'):

  default:
    if (INSERT_MODE)
      editorInsertChar(c);
    if (NORMAL_MODE)
    {
      if (c != 'h' || c != 'j' || c != 'k' || c != 'l')
      {
        command_count = 1;

        if (command_count)
        {
          editorSetStatusMessage("Command: %c", c);
          switch (c)
          {
          case 'd':
            if (prev == '\0')
            {
              prev = c;
              time_last = time(NULL);
            }

            else if (prev == 'd')
            {
              editorDelRow(edt.cy);
              prev = '\0';
            }
            break;

          default:
            prev = c;
          }
        }
      }
      else
      {
        editorSetStatusMessage("");
        command_count = 0;
        editorMoveCursor(c);
      }
    }
    break;
  }

  quit_times = TERM_QUIT_TIMES;
}

// file handling

void editorOpen(char *filename)
{

  free(edt.filename);
  edt.filename = strdup(filename);

  editorSelectSyntaxHighlight();

  FILE *fp = fopen(filename, "r");
  if (!fp)
    terminate("fopen");

  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;

  while ((linelen = getline(&line, &linecap, fp)) != -1)
  {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      linelen--;
    editorInsertRow(edt.numrows, line, linelen);
  }

  free(line);
  fclose(fp);
  edt.unch = 0;
}

// output

void editorSetStatusMessage(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(edt.statusmsg, sizeof(edt.statusmsg), fmt, ap);
  va_end(ap);
  edt.statusmsg_time = time(NULL);
}
void editorScroll()
{
  edt.rx = 0;
  if (edt.cy < edt.numrows)
  {
    edt.rx = editorRowCxtoRx(&edt.row[edt.cy], edt.cx);
  }

  if (edt.cy < edt.rowoff)
  {
    edt.rowoff = edt.cy;
  }
  if (edt.cy >= edt.rowoff + edt.screenrows)
  {
    edt.rowoff = edt.cy - edt.screenrows + 1;
  }
  if (edt.rx < edt.coloff)
  {
    edt.coloff = edt.rx;
  }
  if (edt.rx >= edt.coloff + edt.screencols)
  {
    edt.coloff = edt.rx - edt.screencols + 1;
  }
}

void editorDrawRows(struct abuf *ab)
{
  int y;
  for (y = 0; y < edt.screenrows; y++)
  {
    int filerow = y + edt.rowoff;
    if (filerow >= edt.numrows)
    {
      if (y == edt.screenrows / 3 && edt.numrows == 0)
      {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
                                  "TermText editor -- version %s", T_TEXT_VERS);
        if (welcomelen > edt.screencols)
          welcomelen = edt.screencols;

        int padding = (edt.screencols - welcomelen) / 2;
        if (padding)
        {
          abAppend(ab, "~", 1);
          padding--;
        }
        while (padding--)
          abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      }
      else
      {
        abAppend(ab, "~", 1);
      }
    }
    else
    {
      int len = edt.row[filerow].rsize - edt.coloff;
      if (len < 0)
        len = 0;
      if (len > edt.screencols)
        len = edt.screencols;
      char *c = &edt.row[filerow].render[edt.coloff];
      unsigned char *hl = &edt.row[filerow].hl[edt.coloff];
      int current_colour = -1;
      int j;
      for (j = 0; j < len; j++)
      {
        if (iscntrl(c[j]))
        {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';
          abAppend(ab, "\x1b[7m", 4);
          abAppend(ab, &sym, 1);
          abAppend(ab, "\x1b[m", 3);
          if (current_colour != -1)
          {
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_colour);
            abAppend(ab, buf, clen);
          }
        }

        else if (hl[j] == HL_NORMAL)
        {
          if (current_colour != -1)
          {
            abAppend(ab, "\x1b[39m", 5);
            current_colour = -1;
          }
          abAppend(ab, &c[j], 1);
        }
        else
        {
          int colour = editorSyntaxToColour(hl[j]);
          if (colour != current_colour)
          {
            current_colour = colour;
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", colour);
            abAppend(ab, buf, clen);
          }
          abAppend(ab, &c[j], 1);
        }
      }
      abAppend(ab, "\x1b[39m", 5);
    }

    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

void editorDrawMessageBar(struct abuf *ab)
{
  abAppend(ab, "\x1b[K", 3);
  int msglen = strlen(edt.statusmsg);
  if (msglen > edt.screencols)
    msglen = edt.screencols;
  if (msglen && time(NULL) - edt.statusmsg_time < 5)
    abAppend(ab, edt.statusmsg, msglen);
}

void editorDrawStatusBar(struct abuf *ab)
{
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];

  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                     edt.filename ? edt.filename : "[No Name]", edt.numrows,
                     edt.unch ? ("modified") : "");

  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d  | %s", edt.syntax ? edt.syntax->filetype : "no ft", edt.cy + 1, edt.numrows, NORMAL_MODE ? "NORMAL MODE " : "INSERT MODE ");

  if (len > edt.screencols)
    len = edt.screencols;
  abAppend(ab, status, len);

  while (len < edt.screencols)
  {
    if (edt.screencols - len == rlen)
    {
      abAppend(ab, rstatus, rlen);
      break;
    }
    else
    {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void editorRefreshScreen()
{

  editorScroll();

  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);
  editorDrawStatusBar(&ab);
  editorDrawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (edt.cy - edt.rowoff) + 1, (edt.rx - edt.coloff) + 1);
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

// Main edit loop

void initEditor()
{
  edt.cx = 0;
  edt.cy = 0;
  edt.rx = 0;
  edt.rowoff = 0;
  edt.coloff = 0;
  edt.numrows = 0;
  edt.row = NULL;
  edt.unch = 0;
  edt.filename = NULL;
  edt.statusmsg[0] = '\0';
  edt.statusmsg_time = 0;
  edt.syntax = NULL;

  if (getWindowSize(&edt.screenrows, &edt.screencols) == -1)
    terminate("getWindowSize");

  edt.screenrows -= 2;
}

int main(int argc, char *argv[])
{

  enableRawMode();
  initEditor();
  if (argc >= 2)
  {
    editorOpen(argv[1]);
  }

  editorSetStatusMessage("HELP: Ctrl-Q = quit | Ctrl-S = save | Ctrl-F = find");

  while (1)
  {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
