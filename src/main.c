#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

#define K * 1024
#define M K K
#define G M M

#define BUFFER_MAX 1 G
#define BUFFER_MIN 1 K
#define CURSOR_EDGE 4
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
    size_t lines;
    size_t buffer_size;
    size_t buffer_usage;
    size_t buffer_index;
    char *buffer;
    const char * file_name;
    bool file_exists;
    int mode;
} Editor;

size_t count_lines(const Editor * editor)
{
    size_t lines = 0;
    for(size_t i = 0; i < editor->buffer_usage; i++)
        if(editor->buffer[i] == '\n')
            lines++;
    return lines;
}

void load_buffer_from_file(Editor *editor)
{
    FILE *input_file = fopen(editor->file_name, "r");

    fseek(input_file, 0,SEEK_END);
    size_t file_len = ftell(input_file);
    fseek(input_file, 0, 0);

    if(file_len > BUFFER_MIN)
        editor->buffer_size = file_len + 1;
    else
        editor->buffer_size = BUFFER_MIN;

    editor->buffer = malloc(editor->buffer_size);

    for(char c = fgetc(input_file); !feof(input_file); c = fgetc(input_file))
    {
        static int i = 0;
        editor->buffer[i] = c;
        i++;
    }

    editor->buffer_usage = file_len;

    fclose(input_file);
}

void init_editor(Editor *editor, const char *init_file_name)
{
    editor->file_name = init_file_name;
    if(access(editor->file_name, F_OK) == 0)
    {
        load_buffer_from_file(editor);
        editor->file_exists = true;
        editor->lines = count_lines(editor);
    }
    else
    {
        editor->buffer = malloc(BUFFER_MIN);
        editor->buffer_size = BUFFER_MIN;
        memset(editor->buffer, 0, editor->buffer_size - 1);
        editor->buffer_usage = 0;
    }
    editor->buffer_index = 0;

    editor->mode=1;

    static Cursor init_cursors[CURSOR_MAX];
    init_cursors[0].x = CURSOR_EDGE;
    init_cursors[0].y = 0;
    init_cursors[0].active = true;
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
    size_t win_x = 4;
    size_t win_y = 0;

    for(size_t i = 0; i <= editor->lines; i++)
        mvprintw(i, 0, "%ld", i);

    for(size_t i = 0; i < editor->buffer_usage; i++)
    {
        const char c = editor->buffer[i];
        if(c == '\n')
        {
            win_y++;
            win_x = 4;
        }
        if(c != 0)
        {
            mvaddch(win_y, win_x, editor->buffer[i]);
            win_x++;
        }
    }
    mvaddch(editor->cursors[0].y, editor->cursors[0].x, '|');

    mvprintw(LINES-3, 0, 
            "DEBUG_INFO BUFFER_LEN: %zu BUFFER_USAGE: %zu BUFFER_INDEX: %zu CURSOR_X: %zu CURSOR_Y: %zu, LINES: %zu",
             editor->buffer_size, 
             editor->buffer_usage, 
             editor->buffer_index, 
             editor->cursors[0].x, 
             editor->cursors[0].y, 
             editor->lines);
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
    fputc('\n', output_file);
    fclose(output_file);
}

size_t gen_index_from_cursor(const Editor * editor)
{
    size_t lines = 0;
    size_t x_pos = 0;
    for(size_t i = 0; i < editor->buffer_usage; ++i)
    {
        if(lines == editor->cursors[0].y)
        {
            x_pos++;
            if(x_pos == editor->cursors[0].x - 4)
            {
                return i;
            }
        }
        if(editor->buffer[i] == '\n')
            lines++;
    }
    return 0;
}

int interact(Editor* editor,const char input)
{
    if(input == 4)
        return EDITOR_EXIT;

    if(input == 23)
    {
        write_buffer(editor);
        return 0;
    }

    if(input == '\b')
        return 0;

    //Parsing Arrow key inputs
    if(input == '\033') //first value is esc
    {
        getch();
        switch(getch())
        {
        case 'A':
            editor->cursors[0].y--;
            editor->buffer_index = gen_index_from_cursor(editor);
            break;
        case 'B':
            editor->cursors[0].y++;
            editor->buffer_index = gen_index_from_cursor(editor);
            break;
        case 'C':
            editor->cursors[0].x++;
            editor->buffer_index = gen_index_from_cursor(editor);
            break;
        case 'D':
            editor->cursors[0].x--;
            editor->buffer_index = gen_index_from_cursor(editor);
        }
        return 0;
    }

    if(editor->mode == 1)
    {
        if(editor->buffer_index < editor->buffer_size)
        {
            editor->buffer[editor->buffer_index] = input;
            editor->buffer_usage++;
            editor->cursors[0].x++;
        }
        if(input == '\n')
        {
            editor->lines++;
            editor->cursors[0].x = CURSOR_EDGE;
            editor->cursors[0].y++;
        }
    }
    if(editor->buffer_index <= editor->buffer_size - 1)
        editor->buffer_index++;

    return 0;
}

int main(int argc, char **argv)
{
    if(argc > 2)
    {
        fputs("Please specify only one file as of now", stderr);
        return 1;
    }
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
            free(editor.buffer);
            break;
        }
    }
    fputs(editor.buffer, stderr);
    endwin();
    return 0;
}
