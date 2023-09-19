#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>

#define K * 1024
#define M K K
#define G M M

#define BUFFER_MAX 1 G
#define BUFFER_MIN 1 K
#define CURSOR_MAX 4
#define EDITOR_EXIT 1

typedef struct
{
    size_t x;
    size_t y;
    bool active;
} Cursor;

typedef struct
{
    Cursor *cursors;
    size_t buffer_size;
    size_t buffer_index;
    char *buffer;
    const char * file_name;
    bool file_exists;
    int mode;
} Editor;

void load_buffer(Editor *editor)
{
    FILE *input_file = fopen(editor->file_name, "r");
    size_t file_len = 0;

    fseek(input_file, 0, SEEK_END);
    file_len = ftell(input_file);
    fseek(input_file, 0, 0);

    editor->buffer = malloc(file_len + 1);
    editor->buffer_size = file_len + 1;
    editor->buffer_index = 0;

    for(char c = fgetc(input_file); !feof(input_file); c = fgetc(input_file))
    {
        editor->buffer[editor->buffer_index] = c;
        editor->buffer_index++;
    }
    fputs(editor->buffer, stderr);
}

void init_editor(Editor *editor, const char *init_file_name)
{
    editor->file_name = init_file_name;
    editor->file_exists = (bool) (init_file_name != NULL);

    if(editor->file_exists)
        load_buffer(editor);
    else
    {
        editor->buffer = malloc(BUFFER_MIN);
        editor->buffer_size = BUFFER_MIN;

        memset(editor->buffer, 0, editor->buffer_size - 1);
    }
    editor->buffer_index = 0;

    editor->mode=1;

    static Cursor init_cursors[CURSOR_MAX];
    init_cursors[0].x = 0;
    init_cursors[0].y = 0;
    init_cursors[0].active = false;
    for(int i = 1; i < CURSOR_MAX; i++)
    {
        init_cursors[i].x = 0;
        init_cursors[i].y = 0;
        init_cursors[i].active = false;
    }
    editor->cursors = init_cursors;
}

void render_editor(const Editor *editor)
{
    size_t win_x = 0;
    size_t win_y = 0;

    for(size_t i = 0; i < editor->buffer_size - 1; i++)
    {
        register const char c = editor->buffer[i];
        if(c == '\n')
        {
            win_y++;
            win_x = 0;
        }
        if(c != 0)
        {
            mvaddch(win_y, win_x, editor->buffer[i]);
            win_x++;
        }
    }
}

void write_buffer(const Editor *editor)
{
    FILE* output_file;
    if(editor->file_name == NULL)
    {
        char * fname = malloc(255);
        mvprintw(LINES-2, 0, "Please name your text file");
        mvgetnstr(LINES-1, 0,fname, 254);
        output_file = fopen(fname, "w+");
    }
    else
        output_file = fopen(editor->file_name, "w+");
    fputs(editor->buffer, output_file);
    fclose(output_file);
}

int interact(Editor* editor,const char input)
{
    if(input == 4)
    {
        return EDITOR_EXIT;
    }
    if(input == 23)
    {
        write_buffer(editor);
    }
    if(input == '\b')
    {
        return 0;
    }

    editor->buffer[editor->buffer_index] = input;

    if(editor->buffer_index <= editor->buffer_size - 1)
        editor->buffer_index++;

    return 0;
}

int main(int argc, char **argv)
{
    initscr();
    Editor editor;
    init_editor(&editor, argv[1]);
    for(;;)
    {
        refresh();
        clear();
        render_editor(&editor);
        if(interact(&editor, getch()) == EDITOR_EXIT)
        {
            break;
        }
    }
    endwin();
    return 0;
}
