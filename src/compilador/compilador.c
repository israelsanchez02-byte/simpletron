
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "../expresiones/expresiones.h"

/* Compilador del lenguaje Simple a SML para Simpletron. */
#define MAX_LINE_LENGTH 512
#define MAX_SOURCE_LINES 500
#define MAX_CODE 950
#define MAX_DATA 500
#define MAX_TOTAL_MEMORY 1000
#define MAX_EXPR_DEPTH 100
#define MAX_PATCHES 1000

/* Opcodes compatibles con el simulador Simpletron del proyecto. */
#define OP_READ       10
#define OP_WRITE      11
#define OP_LOAD       20
#define OP_STORE      21
#define OP_ADD        30
#define OP_SUBTRACT   31
#define OP_DIVIDE     32
#define OP_MULTIPLY   33
#define OP_MODULUS    34
#define OP_EXPONENT   35
#define OP_BRANCH     40
#define OP_BRANCHNEG  41
#define OP_BRANCHZERO 42
#define OP_HALT       43

typedef struct {
    int line_number;
    char statement[MAX_LINE_LENGTH];
} SourceLine;

typedef struct {
    int line_number;
    int address;
} LineLabel;

typedef struct {
    int instruction_index;
    int data_id;
} DataPatch;

typedef struct {
    int instruction_index;
    int target_line;
} LinePatch;

typedef struct {
    int instruction_index;
    int target_address;
} LocalPatch;

static SourceLine source_lines[MAX_SOURCE_LINES];
static int source_count = 0;

static int code[MAX_CODE];
static int code_count = 0;

static int data_values[MAX_DATA];
static int data_count = 26; /* a-z: una celda fija por variable */

static LineLabel labels[MAX_SOURCE_LINES];
static int label_count = 0;

static DataPatch data_patches[MAX_PATCHES];
static int data_patch_count = 0;

static LinePatch line_patches[MAX_PATCHES];
static int line_patch_count = 0;

static LocalPatch local_patches[MAX_PATCHES];
static int local_patch_count = 0;

static int current_source_line = 0;
static int saw_end = 0;

static void trim(char *text) {
    size_t start = 0;
    size_t length;

    while (text[start] != '\0' &&
           isspace((unsigned char)text[start])) {
        start++;
    }

    if (start > 0) {
        memmove(text, text + start, strlen(text + start) + 1);
    }

    length = strlen(text);

    while (length > 0 &&
           isspace((unsigned char)text[length - 1])) {
        text[--length] = '\0';
    }
}

static int report_error(const char *message) {
    if (current_source_line > 0) {
        fprintf(stderr, "Error en linea %d: %s\n",
                current_source_line, message);
    } else {
        fprintf(stderr, "Error: %s\n", message);
    }

    return 0;
}

static int read_source(const char *path) {
    FILE *file = fopen(path, "r");
    char buffer[MAX_LINE_LENGTH];
    int previous_line = -1;

    if (file == NULL) {
        perror("No se pudo abrir el archivo fuente");
        return 0;
    }

    source_count = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char *endptr;
        long line_number;
        char *statement;
        size_t len = strlen(buffer);

        if (len > 0 &&
            buffer[len - 1] != '\n' &&
            !feof(file)) {

            int ch;

            while ((ch = fgetc(file)) != '\n' && ch != EOF) {
            }

            fclose(file);
            current_source_line = previous_line;

            return report_error(
                "la linea fuente excede el limite permitido"
            );
        }

        trim(buffer);

        if (buffer[0] == '\0') {
            continue;
        }

        errno = 0;
        line_number = strtol(buffer, &endptr, 10);

        if (endptr == buffer ||
            errno == ERANGE ||
            line_number < 0 ||
            line_number > INT_MAX) {

            fclose(file);
            current_source_line = 0;

            return report_error(
                "cada instruccion debe comenzar con un numero de linea valido"
            );
        }

        if (*endptr != '\0' &&
            !isspace((unsigned char)*endptr)) {

            fclose(file);
            current_source_line = (int)line_number;

            return report_error(
                "debe existir un espacio entre el numero de linea y el comando"
            );
        }

        if ((int)line_number <= previous_line) {
            fclose(file);
            current_source_line = (int)line_number;

            return report_error(
                "los numeros de linea deben estar en orden estrictamente ascendente"
            );
        }

        if (source_count >= MAX_SOURCE_LINES) {
            fclose(file);
            current_source_line = (int)line_number;

            return report_error(
                "el programa contiene demasiadas lineas"
            );
        }

        statement = endptr;
        trim(statement);

        if (*statement == '\0') {
            fclose(file);
            current_source_line = (int)line_number;

            return report_error(
                "falta el comando despues del numero de linea"
            );
        }

        source_lines[source_count].line_number = (int)line_number;

        if (strlen(statement) >=
            sizeof(source_lines[source_count].statement)) {

            fclose(file);
            current_source_line = (int)line_number;

            return report_error("comando demasiado largo");
        }

        strcpy(source_lines[source_count].statement, statement);

        source_count++;
        previous_line = (int)line_number;
    }

    if (ferror(file)) {
        fclose(file);
        return report_error("error al leer el archivo fuente");
    }

    fclose(file);

    if (source_count == 0) {
        return report_error("el archivo fuente esta vacio");
    }

    return 1;
}

static int emit_instruction(int opcode, int operand) {
    if (code_count >= MAX_CODE) {
        return report_error(
            "el codigo generado excede la capacidad permitida"
        );
    }

    if (opcode < 0 || opcode > 99 ||
        operand < 0 || operand > 999) {

        return report_error("instruccion SML fuera de rango");
    }

    code[code_count++] = opcode * 1000 + operand;

    return 1;
}

static int emit_data_instruction(int opcode, int data_id) {
    if (data_id < 0 ||
        data_id >= MAX_DATA ||
        data_patch_count >= MAX_PATCHES) {

        return report_error("referencia de datos invalida");
    }

    if (!emit_instruction(opcode, 0)) {
        return 0;
    }

    data_patches[data_patch_count].instruction_index = code_count - 1;
    data_patches[data_patch_count].data_id = data_id;

    data_patch_count++;

    return 1;
}

static int emit_line_branch(int opcode, int target_line) {
    if (line_patch_count >= MAX_PATCHES) {
        return report_error(
            "demasiadas referencias a lineas de destino"
        );
    }

    if (!emit_instruction(opcode, 0)) {
        return 0;
    }

    line_patches[line_patch_count].instruction_index = code_count - 1;
    line_patches[line_patch_count].target_line = target_line;

    line_patch_count++;

    return 1;
}

static int emit_local_branch(int opcode, int *patch_index) {
    if (!emit_instruction(opcode, 0)) {
        return 0;
    }

    if (local_patch_count >= MAX_PATCHES) {
        return report_error("demasiados saltos internos");
    }

    local_patches[local_patch_count].instruction_index = code_count - 1;
    local_patches[local_patch_count].target_address = -1;

    *patch_index = local_patch_count++;

    return 1;
}

static void resolve_local_patch(int patch_index) {
    if (patch_index >= 0 &&
        patch_index < local_patch_count) {

        local_patches[patch_index].target_address = code_count;
    }
}

static int allocate_data(int initial_value) {
    if (data_count >= MAX_DATA) {
        report_error(
            "se excedio la cantidad maxima de variables, constantes y temporales"
        );

        return -1;
    }

    data_values[data_count] = initial_value;

    return data_count++;
}

static int get_constant(int value) {
    int i;

    /* Las constantes se reutilizan para reducir el area de datos. */
    for (i = 26; i < data_count; i++) {
        if (data_values[i] == value) {
            return i;
        }
    }

    return allocate_data(value);
}

static int add_label(int line_number, int address) {
    if (label_count >= MAX_SOURCE_LINES) {
        return report_error("demasiadas etiquetas de linea");
    }

    labels[label_count].line_number = line_number;
    labels[label_count].address = address;

    label_count++;

    return 1;
}

static int find_label(int line_number) {
    int i;

    for (i = 0; i < label_count; i++) {
        if (labels[i].line_number == line_number) {
            return labels[i].address;
        }
    }

    return -1;
}

/*
    Convierte una expresion infija a postfija
    reutilizando la funcion del modulo de expresiones.
*/
static int compile_expression(
    const char *expression,
    int *result_data_id
) {
    char postfix[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int postfix_count = 0;

    int value_stack[MAX_TOKENS];
    int stack_top = -1;
    int i;

    if (!convertirPostfija(expression, postfix, &postfix_count)) {
        return 0;
    }

    for (i = 0; i < postfix_count; i++) {
        char *token = postfix[i];

        /* Identificar variables de una sola letra minuscula */
        if (strlen(token) == 1 &&
            token[0] >= 'a' &&
            token[0] <= 'z') {

            value_stack[++stack_top] = token[0] - 'a';
            continue;
        }

        /* Identificar constantes enteras */
        if (isdigit((unsigned char)token[0])) {
            char *endptr;
            long value;

            errno = 0;
            value = strtol(token, &endptr, 10);

            if (errno == ERANGE ||
                *endptr != '\0' ||
                value > 99999L) {

                return report_error(
                    "constante entera fuera del rango permitido de SML"
                );
            }

            {
                int id = get_constant((int)value);

                if (id < 0) {
                    return 0;
                }

                value_stack[++stack_top] = id;
            }

            continue;
        }

        /* Procesar signos unarios */
        if (strcmp(token, "p") == 0 ||
            strcmp(token, "n") == 0) {

            int operand_id;
            int temp_id;
            int minus_one_id;

            if (stack_top < 0) {
                return report_error(
                    "falta un operando para el signo unario"
                );
            }

            operand_id = value_stack[stack_top];

            if (token[0] == 'p') {
                continue;
            }

            minus_one_id = get_constant(-1);

            if (minus_one_id < 0) {
                return 0;
            }

            temp_id = allocate_data(0);

            if (temp_id < 0) {
                return 0;
            }

            if (!emit_data_instruction(OP_LOAD, operand_id) ||
                !emit_data_instruction(OP_MULTIPLY, minus_one_id) ||
                !emit_data_instruction(OP_STORE, temp_id)) {

                return 0;
            }

            value_stack[stack_top] = temp_id;

            continue;
        }

        /* Procesar operadores binarios */
        {
            int right_id;
            int left_id;
            int temp_id;
            int opcode;

            if (stack_top < 1) {
                return report_error(
                    "faltan operandos para una operacion"
                );
            }

            right_id = value_stack[stack_top--];
            left_id = value_stack[stack_top];

            switch (token[0]) {
                case '+':
                    opcode = OP_ADD;
                    break;

                case '-':
                    opcode = OP_SUBTRACT;
                    break;

                case '*':
                    opcode = OP_MULTIPLY;
                    break;

                case '/':
                    opcode = OP_DIVIDE;
                    break;

                case '^':
                    opcode = OP_EXPONENT;
                    break;

                default:
                    return report_error(
                        "operador no reconocido por el compilador"
                    );
            }

            temp_id = allocate_data(0);

            if (temp_id < 0) {
                return 0;
            }

            if (!emit_data_instruction(OP_LOAD, left_id) ||
                !emit_data_instruction(opcode, right_id) ||
                !emit_data_instruction(OP_STORE, temp_id)) {

                return 0;
            }

            value_stack[stack_top] = temp_id;
        }
    }

    if (stack_top != 0) {
        return report_error(
            "la expresion no produce un unico resultado"
        );
    }

    *result_data_id = value_stack[0];

    return 1;
}

static int parse_single_variable(
    const char *text,
    int *variable_id
) {
    char copy[MAX_LINE_LENGTH];

    strcpy(copy, text);
    trim(copy);

    if (strlen(copy) != 1 ||
        copy[0] < 'a' ||
        copy[0] > 'z') {

        return report_error(
            "se esperaba una variable de una sola letra minuscula"
        );
    }

    *variable_id = copy[0] - 'a';

    return 1;
}

static int parse_target_line(const char *text, int *target) {
    char *endptr;
    long value;
    char copy[MAX_LINE_LENGTH];

    strcpy(copy, text);
    trim(copy);

    errno = 0;
    value = strtol(copy, &endptr, 10);

    if (copy[0] == '\0' ||
        endptr == copy ||
        errno == ERANGE ||
        value < 0 ||
        value > INT_MAX) {

        return report_error("destino goto invalido");
    }

    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    if (*endptr != '\0') {
        return report_error(
            "hay texto adicional despues del destino goto"
        );
    }

    *target = (int)value;

    return 1;
}

static int compile_input(char *arguments) {
    int variable_id;

    if (!parse_single_variable(arguments, &variable_id)) {
        return 0;
    }

    return emit_data_instruction(OP_READ, variable_id);
}

static int compile_let(char *arguments) {
    char *p = arguments;
    char *equals;

    int variable_id;
    int result_id;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (p[0] < 'a' || p[0] > 'z') {
        return report_error(
            "let requiere una variable de una letra minuscula"
        );
    }

    variable_id = p[0] - 'a';

    p++;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p != '=') {
        return report_error(
            "sintaxis esperada: let variable = expresion"
        );
    }

    equals = p;
    (void)equals;

    p++;

    trim(p);

    if (*p == '\0') {
        return report_error("let requiere una expresion");
    }

    if (!compile_expression(p, &result_id)) {
        return 0;
    }

    return emit_data_instruction(OP_LOAD, result_id) &&
           emit_data_instruction(OP_STORE, variable_id);
}

static int compile_print(char *arguments) {
    int result_id;

    trim(arguments);

    if (*arguments == '\0') {
        return report_error(
            "print requiere una variable o expresion"
        );
    }

    if (!compile_expression(arguments, &result_id)) {
        return 0;
    }

    return emit_data_instruction(OP_WRITE, result_id);
}

static char *find_relation(
    char *text,
    char relation[3],
    int *relation_length
) {
    int depth = 0;
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        char c = text[i];

        if (c == '(' || c == '[' || c == '{') {
            depth++;
        }
        else if (c == ')' || c == ']' || c == '}') {
            depth--;
        }

        if (depth == 0) {
            if ((c == '<' || c == '>') &&
                text[i + 1] == '=') {

                relation[0] = c;
                relation[1] = '=';
                relation[2] = '\0';

                *relation_length = 2;

                return &text[i];
            }

            if ((c == '=' && text[i + 1] == '=') ||
                (c == '!' && text[i + 1] == '=')) {

                relation[0] = c;
                relation[1] = '=';
                relation[2] = '\0';

                *relation_length = 2;

                return &text[i];
            }

            if (c == '<' || c == '>') {
                relation[0] = c;
                relation[1] = '\0';
                relation[2] = '\0';

                *relation_length = 1;

                return &text[i];
            }
        }
    }

    return NULL;
}

static int compile_if(char *arguments) {
    char *goto_text;
    char *relation_pos;

    char relation[3];

    int relation_length = 0;
    int target_line;

    char left_expression[MAX_LINE_LENGTH];
    char right_expression[MAX_LINE_LENGTH];

    int left_id;
    int right_id;

    int patch1;
    int patch2;

    trim(arguments);

    goto_text = strstr(arguments, " goto ");

    if (goto_text == NULL) {
        return report_error(
            "sintaxis esperada: if expresion operador expresion goto linea"
        );
    }

    *goto_text = '\0';
    goto_text += 6;

    if (!parse_target_line(goto_text, &target_line)) {
        return 0;
    }

    relation_pos = find_relation(
        arguments,
        relation,
        &relation_length
    );

    if (relation_pos == NULL) {
        return report_error(
            "condicion if sin operador relacional valido"
        );
    }

    if ((size_t)(relation_pos - arguments) >=
        sizeof(left_expression)) {

        return report_error(
            "expresion izquierda demasiado larga"
        );
    }

    memcpy(
        left_expression,
        arguments,
        (size_t)(relation_pos - arguments)
    );

    left_expression[relation_pos - arguments] = '\0';

    strcpy(right_expression, relation_pos + relation_length);

    trim(left_expression);
    trim(right_expression);

    if (left_expression[0] == '\0' ||
        right_expression[0] == '\0') {

        return report_error(
            "la condicion if requiere dos expresiones"
        );
    }

    if (!compile_expression(left_expression, &left_id) ||
        !compile_expression(right_expression, &right_id)) {

        return 0;
    }

    if (!emit_data_instruction(OP_LOAD, left_id) ||
        !emit_data_instruction(OP_SUBTRACT, right_id)) {

        return 0;
    }

    if (strcmp(relation, "<") == 0) {
        return emit_line_branch(OP_BRANCHNEG, target_line);
    }

    if (strcmp(relation, "<=") == 0) {
        return emit_line_branch(OP_BRANCHNEG, target_line) &&
               emit_line_branch(OP_BRANCHZERO, target_line);
    }

    if (strcmp(relation, ">") == 0) {
        if (!emit_local_branch(OP_BRANCHNEG, &patch1) ||
            !emit_local_branch(OP_BRANCHZERO, &patch2) ||
            !emit_line_branch(OP_BRANCH, target_line)) {

            return 0;
        }

        resolve_local_patch(patch1);
        resolve_local_patch(patch2);

        return 1;
    }

    if (strcmp(relation, ">=") == 0) {
        if (!emit_local_branch(OP_BRANCHNEG, &patch1) ||
            !emit_line_branch(OP_BRANCH, target_line)) {

            return 0;
        }

        resolve_local_patch(patch1);

        return 1;
    }

    if (strcmp(relation, "==") == 0) {
        return emit_line_branch(OP_BRANCHZERO, target_line);
    }

    if (strcmp(relation, "!=") == 0) {
        if (!emit_local_branch(OP_BRANCHZERO, &patch1) ||
            !emit_line_branch(OP_BRANCH, target_line)) {

            return 0;
        }

        resolve_local_patch(patch1);

        return 1;
    }

    return report_error("operador relacional no soportado");
}

static int compile_statement(const char *statement) {
    char copy[MAX_LINE_LENGTH];

    char *command;
    char *arguments;
    char *space;

    if (strlen(statement) >= sizeof(copy)) {
        return report_error("comando demasiado largo");
    }

    strcpy(copy, statement);
    trim(copy);

    if (copy[0] == '\0') {
        return 1;
    }

    space = copy;

    while (*space != '\0' &&
           !isspace((unsigned char)*space)) {

        space++;
    }

    if (*space != '\0') {
        *space = '\0';
        arguments = space + 1;
        trim(arguments);
    } else {
        arguments = space;
    }

    command = copy;

    /* rem permite comentarios con cualquier combinacion de mayusculas. */
    if (strcmp(command, "rem") == 0) {
        return 1;
    }

    if (strcmp(command, "input") == 0) {
        return compile_input(arguments);
    }

    if (strcmp(command, "let") == 0) {
        return compile_let(arguments);
    }

    if (strcmp(command, "print") == 0) {
        return compile_print(arguments);
    }

    if (strcmp(command, "goto") == 0) {
        int target_line;

        if (!parse_target_line(arguments, &target_line)) {
            return 0;
        }

        return emit_line_branch(OP_BRANCH, target_line);
    }

    if (strcmp(command, "if") == 0) {
        return compile_if(arguments);
    }

    if (strcmp(command, "end") == 0) {
        if (arguments[0] != '\0') {
            return report_error("end no recibe argumentos");
        }

        saw_end = 1;

        return emit_instruction(OP_HALT, 0);
    }

    return report_error(
        "comando Simple no reconocido; use rem, input, let, print, goto, if o end"
    );
}

static int first_pass(void) {
    int i;

    code_count = 0;
    label_count = 0;
    data_patch_count = 0;
    line_patch_count = 0;
    local_patch_count = 0;

    data_count = 26;

    memset(data_values, 0, sizeof(data_values));

    saw_end = 0;

    for (i = 0; i < source_count; i++) {
        current_source_line = source_lines[i].line_number;

        if (!add_label(
            source_lines[i].line_number,
            code_count
        )) {
            return 0;
        }

        if (!compile_statement(source_lines[i].statement)) {
            return 0;
        }
    }

    if (!saw_end) {
        current_source_line = 0;

        return report_error(
            "falta el comando end para terminar el programa"
        );
    }

    return 1;
}

/* Segunda pasada: resolver referencias a datos y saltos hacia otras lineas. */
static int second_pass(void) {
    int data_base = code_count;
    int total_memory = code_count + data_count;
    int i;

    if (total_memory > MAX_TOTAL_MEMORY) {
        return report_error(
            "el programa compilado excede las 1000 posiciones de memoria Simpletron"
        );
    }

    for (i = 0; i < data_patch_count; i++) {
        int index = data_patches[i].instruction_index;

        int data_address =
            data_base + data_patches[i].data_id;

        int opcode = code[index] / 1000;

        if (data_address < 0 || data_address > 999) {
            return report_error(
                "direccion de datos fuera del rango de Simpletron"
            );
        }

        code[index] = opcode * 1000 + data_address;
    }

    for (i = 0; i < line_patch_count; i++) {
        int index = line_patches[i].instruction_index;

        int address =
            find_label(line_patches[i].target_line);

        int opcode = code[index] / 1000;

        if (address < 0 || address >= code_count) {
            current_source_line = 0;

            fprintf(
                stderr,
                "Error: goto hace referencia a la linea inexistente %d.\n",
                line_patches[i].target_line
            );

            return 0;
        }

        code[index] = opcode * 1000 + address;
    }

    for (i = 0; i < local_patch_count; i++) {
        int index = local_patches[i].instruction_index;
        int address = local_patches[i].target_address;

        int opcode = code[index] / 1000;

        if (address < 0 || address >= code_count) {
            return report_error(
                "salto condicional interno invalido"
            );
        }

        code[index] = opcode * 1000 + address;
    }

    return 1;
}

static void make_output_path(
    const char *input_path,
    char *output_path,
    size_t output_size
) {
    const char *base1 = strrchr(input_path, '/');
    const char *base2 = strrchr(input_path, '\\');

    const char *base = input_path;

    char name[256];
    char *dot;

    size_t length;

    if (base1 != NULL &&
        (base2 == NULL || base1 > base2)) {

        base = base1 + 1;
    }
    else if (base2 != NULL) {
        base = base2 + 1;
    }

    strncpy(name, base, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    dot = strrchr(name, '.');

    if (dot != NULL) {
        *dot = '\0';
    }

    length = strlen(name);

    if (length == 0) {
        strcpy(name, "programa");
    }

    (void)snprintf(
        output_path,
        output_size,
        "programas/sml/%s.sml",
        name
    );
}

static int write_sml(const char *output_path) {
    FILE *file = fopen(output_path, "w");

    int i;

    if (file == NULL) {
        perror("No se pudo crear el archivo SML");

        fprintf(
            stderr,
            "Verifique que exista la carpeta programas/sml.\n"
        );

        return 0;
    }

    for (i = 0; i < code_count; i++) {
        if (fprintf(file, "%05d\n", code[i]) < 0) {
            fclose(file);

            return report_error(
                "error al escribir las instrucciones SML"
            );
        }
    }

    for (i = 0; i < data_count; i++) {
        if (fprintf(file, "%d\n", data_values[i]) < 0) {
            fclose(file);

            return report_error(
                "error al escribir el area de datos SML"
            );
        }
    }

    if (fclose(file) != 0) {
        return report_error("error al cerrar el archivo SML");
    }

    return 1;
}

int main(int argc, char *argv[]) {
    char output_path[512];

    if (argc != 2) {
        fprintf(
            stderr,
            "Uso: %s archivo.simple\n",
            argv[0]
        );

        return EXIT_FAILURE;
    }

    if (!read_source(argv[1])) {
        return EXIT_FAILURE;
    }

    if (!first_pass()) {
        return EXIT_FAILURE;
    }

    if (!second_pass()) {
        return EXIT_FAILURE;
    }

    make_output_path(
        argv[1],
        output_path,
        sizeof(output_path)
    );

    if (!write_sml(output_path)) {
        return EXIT_FAILURE;
    }

    printf("Compilacion correcta.\n");
    printf("Lineas fuente: %d\n", source_count);
    printf("Instrucciones SML: %d\n", code_count);
    printf("Celdas de datos: %d\n", data_count);

    printf(
        "Memoria utilizada: %d / %d\n",
        code_count + data_count,
        MAX_TOTAL_MEMORY
    );

    printf("Archivo generado: %s\n", output_path);

    return EXIT_SUCCESS;
}
