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
}Editor;

void init_editor(Editor *editor, const char *init_file_name)
{
    editor->file_name = init_file_name;
    editor->file_exists = (bool) (init_file_name == NULL);

    editor->buffer = malloc(BUFFER_MIN);
    editor->buffer_size = BUFFER_MIN;
    editor->buffer_index = 0;
    
    memset(editor->buffer, 0, editor->buffer_size - 1);

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
}

void write_buffer(const Editor *editor)
{
    FILE* output_file;
    if(editor->file_name == NULL)
    {
        char * fname = malloc(255);
        getnstr(fname, 254);
        output_file = fopen(fname, "w+");
    } 
    else
        output_file = fopen(editor->file_name, "w+");
    fputs(editor->buffer, output_file);
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
        render_editor(&editor);
        if(interact(&editor, getch()) == EDITOR_EXIT)
        {
            break;
        }
    }
    endwin();
    return 0;
}
