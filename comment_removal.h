#ifndef MACRO_REPLACEMENT_H
#define MACRO_REPLACEMENT_H
#include <stdio.h>
#include <string.h>
#include "macro_replacement.h"

#define MAX_MACROS 200
#define MAX_NAME 100
#define MAX_TEXT 4000
#define MAX_PARAMS 10

typedef struct
{
    char name[MAX_NAME];
    char replacement[MAX_TEXT];
    char params[MAX_PARAMS][MAX_NAME];
    int param_count;
    int function_like;
} Macro;

static Macro macros[MAX_MACROS];
static int macro_count = 0;

static int is_letter(char c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_name_char(char c)
{
    return is_letter(c) || is_digit(c) || c == '_';
}

static int skip_spaces(const char *text, int i)
{
    while (text[i] == ' ' || text[i] == '\t')
        i++;

    return i;
}

static int find_macro(const char *name)
{
    int i;

    for (i = 0; i < macro_count; i++)
    {
        if (strcmp(macros[i].name, name) == 0)
            return i;
    }

    return -1;
}

static int parse_macro_name(const char *line, int *position,
                            char *name)
{
    int i = *position;
    int j = 0;

    while (is_name_char(line[i]) && j < MAX_NAME - 1)
        name[j++] = line[i++];

    name[j] = '\0';
    *position = i;

    return j > 0;
}

static int parse_parameters(const char *line, int *position,
                            Macro *macro)
{
    int i = *position;
    int j;

    macro->function_like = 1;
    macro->param_count = 0;
    i++;

    i = skip_spaces(line, i);

    if (line[i] == ')')
    {
        *position = i + 1;
        return 1;
    }

    while (line[i] != '\0' && line[i] != ')')
    {
        char parameter[MAX_NAME];

        j = 0;

        while (is_name_char(line[i]) && j < MAX_NAME - 1)
            parameter[j++] = line[i++];

        parameter[j] = '\0';

        if (j == 0 || macro->param_count >= MAX_PARAMS)
            return 0;

        strcpy(macro->params[macro->param_count], parameter);
        macro->param_count++;

        i = skip_spaces(line, i);

        if (line[i] == ',')
        {
            i++;
            i = skip_spaces(line, i);
        }
        else if (line[i] != ')')
        {
            return 0;
        }
    }

    if (line[i] != ')')
        return 0;

    *position = i + 1;
    return 1;
}

static void parse_replacement(const char *line, int position,
                              Macro *macro)
{
    int length;

    position = skip_spaces(line, position);

    strncpy(macro->replacement, &line[position], MAX_TEXT - 1);
    macro->replacement[MAX_TEXT - 1] = '\0';

    length = (int)strlen(macro->replacement);

    while (length > 0 &&
           (macro->replacement[length - 1] == '\n' ||
            macro->replacement[length - 1] == '\r'))
    {
        macro->replacement[--length] = '\0';
    }
}

static void add_macro(char *line)
{
    int i = 0;
    int index;
    char name[MAX_NAME];
    Macro macro = {0};

    i = skip_spaces(line, i);

    if (strncmp(&line[i], "#define", 7) != 0 ||
        is_name_char(line[i + 7]))
        return;

    i += 7;
    i = skip_spaces(line, i);

    if (!parse_macro_name(line, &i, name))
        return;

    macro.function_like = 0;

    if (line[i] == '(')
    {
        if (!parse_parameters(line, &i, &macro))
            return;
    }

    parse_replacement(line, i, &macro);

    index = find_macro(name);

    if (index == -1)
    {
        if (macro_count >= MAX_MACROS)
            return;

        index = macro_count++;
    }

    strcpy(macro.name, name);
    macros[index] = macro;
}

static int read_arguments(const char *line, int position,
                          char args[MAX_PARAMS][MAX_TEXT],
                          int *end_position)
{
    int i = position;
    int count = 0;
    int length = 0;
    int depth = 0;

    i = skip_spaces(line, i);

    if (line[i] != '(')
        return -1;

    i++;
    i = skip_spaces(line, i);

    if (line[i] == ')')
    {
        *end_position = i + 1;
        return 0;
    }

    while (line[i] != '\0')
    {
        char c = line[i];

        if (c == '(')
        {
            depth++;

            if (length < MAX_TEXT - 1)
                args[count][length++] = c;
        }
        else if (c == ')')
        {
            if (depth == 0)
            {
                args[count][length] = '\0';
                count++;
                *end_position = i + 1;
                return count;
            }

            depth--;

            if (length < MAX_TEXT - 1)
                args[count][length++] = c;
        }
        else if (c == ',' && depth == 0)
        {
            args[count][length] = '\0';
            count++;

            if (count >= MAX_PARAMS)
                return -1;

            length = 0;
        }
        else
        {
            if (length < MAX_TEXT - 1)
                args[count][length++] = c;
        }

        i++;
    }

    return -1;
}

static int find_parameter(const char *name, const Macro *macro)
{
    int i;

    for (i = 0; i < macro->param_count; i++)
    {
        if (strcmp(name, macro->params[i]) == 0)
            return i;
    }

    return -1;
}

static void append_text(char *destination, int *length,
                        const char *source)
{
    int i = 0;

    while (source[i] != '\0' && *length < MAX_TEXT - 1)
        destination[(*length)++] = source[i++];

    destination[*length] = '\0';
}

static void expand_function_macro(int index,
                                  char args[MAX_PARAMS][MAX_TEXT],
                                  char *expanded)
{
    const char *replacement = macros[index].replacement;
    int i = 0;
    int length = 0;

    expanded[0] = '\0';

    while (replacement[i] != '\0')
    {
        if (is_letter(replacement[i]) || replacement[i] == '_')
        {
            char name[MAX_NAME];
            int j = 0;
            int parameter;

            while (is_name_char(replacement[i]) && j < MAX_NAME - 1)
                name[j++] = replacement[i++];

            name[j] = '\0';

            parameter = find_parameter(name, &macros[index]);

            if (parameter >= 0)
                append_text(expanded, &length, args[parameter]);
            else
                append_text(expanded, &length, name);
        }
        else
        {
            char character[2];

            character[0] = replacement[i++];
            character[1] = '\0';

            append_text(expanded, &length, character);
        }
    }
}

static void expand_line(char *line, FILE *output)
{
    char result[MAX_TEXT];
    int i = 0;
    int length = 0;

    result[0] = '\0';

    while (line[i] != '\0' && length < MAX_TEXT - 1)
    {
        if (is_letter(line[i]) || line[i] == '_')
        {
            char name[MAX_NAME];
            int j = 0;
            int index;

            while (is_name_char(line[i]) && j < MAX_NAME - 1)
                name[j++] = line[i++];

            name[j] = '\0';
            index = find_macro(name);

            if (index == -1)
            {
                append_text(result, &length, name);
            }
            else if (!macros[index].function_like)
            {
                append_text(result, &length,
                            macros[index].replacement);
            }
            else
            {
                char args[MAX_PARAMS][MAX_TEXT] = {{0}};
                char expanded[MAX_TEXT];
                int end_position = i;
                int count = read_arguments(line, i, args,
                                           &end_position);

                if (count == macros[index].param_count)
                {
                    expand_function_macro(index, args, expanded);
                    append_text(result, &length, expanded);
                    i = end_position;
                }
                else
                {
                    append_text(result, &length, name);
                }
            }
        }
        else
        {
            char character[2];

            character[0] = line[i++];
            character[1] = '\0';

            append_text(result, &length, character);
        }
    }

    fputs(result, output);
}

static int is_define_line(const char *line)
{
    int i = skip_spaces(line, 0);

    return strncmp(&line[i], "#define", 7) == 0 &&
           !is_name_char(line[i + 7]);
}

void process_macros(FILE *input, FILE *output)
{
    char line[MAX_TEXT];

    macro_count = 0;

    while (fgets(line, sizeof(line), input) != NULL)
    {
        if (is_define_line(line))
            add_macro(line);
        else
            expand_line(line, output);
    }
}
#endif
