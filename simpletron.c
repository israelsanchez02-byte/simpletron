#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Codigos de operacion SML */
#define READ        10
#define WRITE       11
#define NEWLINE     12
#define READ_STR    13
#define WRITE_STR   14
#define LOAD        20
#define STORE       21
#define ADD         30
#define SUBTRACT    31
#define DIVIDE      32
#define MULTIPLY    33
#define MODULUS     34
#define EXPONENT    35
#define BRANCH      40
#define BRANCHNEG   41
#define BRANCHZERO  42
#define HALT        43

#define MEM_SIZE           1000
#define MIN_VAL           -99999
#define MAX_VAL            99999
#define MAX_STRING_LENGTH  99

/* Memoria y registros */
float memory[MEM_SIZE];
float accumulator = 0.0f;
int instructionCounter = 0;
int instructionRegister = 0;
int operationCode = 0;
int operand = 0;

/* Indica si scanf dejo pendiente el resto de la linea actual. */
static bool pendingInputNewline = false;

/* Prototipos */
static bool isValidWord(float word);
static void clearInputBuffer(void);
static bool loadProgram(const char *programPath, bool allowInteractive);
static bool loadProgramFromFile(FILE *file, const char *programPath);
static bool loadProgramInteractively(void);
static void executeProgram(void);
static void dumpMemory(void);
static void printRuntimeError(const char *message);
static bool readString(int baseAddr);
static bool writeString(int baseAddr);

int main(int argc, char *argv[])
{
    const char *programPath = (argc > 1) ? argv[1] : "programa.simp";
    bool allowInteractive = (argc == 1);

    if (argc > 2) {
        printf("Uso: %s [archivo.simp]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!loadProgram(programPath, allowInteractive)) {
        return EXIT_FAILURE;
    }

    executeProgram();
    return EXIT_SUCCESS;
}

static bool isValidWord(float word)
{
    return isfinite(word) && word >= (float)MIN_VAL && word <= (float)MAX_VAL;
}

static void clearInputBuffer(void)
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        /* Descarta el resto de la linea. */
    }

    pendingInputNewline = false;
}

static void printRuntimeError(const char *message)
{
    printf("*** %s ***\n", message);
    dumpMemory();
}

static bool loadProgram(const char *programPath, bool allowInteractive)
{
    FILE *file;
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        memory[i] = 0.0f;
    }

    accumulator = 0.0f;
    instructionCounter = 0;
    instructionRegister = 0;
    operationCode = 0;
    operand = 0;
    pendingInputNewline = false;

    printf("*** Bienvenido a Simpletron v2.1 ***\n");

    file = fopen(programPath, "r");

    if (file != NULL) {
        bool loaded = loadProgramFromFile(file, programPath);
        fclose(file);

        if (!loaded) {
            return false;
        }
    } else {
        if (!allowInteractive) {
            printf("*** No se pudo abrir el archivo '%s'. ***\n", programPath);
            return false;
        }

        printf("*** Archivo '%s' no encontrado. Modo interactivo. ***\n", programPath);

        if (!loadProgramInteractively()) {
            return false;
        }
    }

    printf("*** Comienza la ejecucion del programa ***\n\n");
    instructionCounter = 0;

    return true;
}

static bool loadProgramFromFile(FILE *file, const char *programPath)
{
    float value;
    int scanResult;
    int lineNum = 0;

    printf("*** Cargando programa desde '%s'... ***\n", programPath);

    while (instructionCounter < MEM_SIZE) {

        scanResult = fscanf(file, "%f", &value);

        if (scanResult == EOF) {
            break;
        }

        if (scanResult != 1) {
            printf("*** Error: dato no numerico en '%s', cerca del valor %d. ***\n",
                   programPath, lineNum + 1);
            return false;
        }

        lineNum++;

        if (value == 99999.0f) {
            break;
        }

        if (!isValidWord(value)) {
            printf("*** Error en '%s' (valor %d): palabra fuera de rango. ***\n",
                   programPath, lineNum);
            return false;
        }

        memory[instructionCounter++] = value;
    }

    if (instructionCounter == MEM_SIZE) {
        printf("*** Error: el programa excede las %d posiciones de memoria. ***\n",
               MEM_SIZE);
        return false;
    }

    if (ferror(file)) {
        printf("*** Error al leer el archivo '%s'. ***\n", programPath);
        return false;
    }

    printf("*** Carga completada: %d palabras (instrucciones y datos). ***\n\n",
           instructionCounter);

    return true;
}

static bool loadProgramInteractively(void)
{
    float value;
    int result;

    printf("Introduce una palabra por linea. Escribe 99999 para terminar.\n\n");

    while (instructionCounter < MEM_SIZE) {

        printf("%03d ? ", instructionCounter);

        result = scanf("%f", &value);

        if (result == EOF) {
            printf("\n*** Fin de entrada antes del centinela 99999. ***\n");
            return false;
        }

        if (result != 1) {
            printf("*** Entrada no numerica. Intenta de nuevo. ***\n");
            clearInputBuffer();
            continue;
        }

        pendingInputNewline = true;

        if (value == 99999.0f) {
            clearInputBuffer();
            printf("\n*** Carga interactiva terminada. ***\n\n");
            return true;
        }

        if (!isValidWord(value)) {
            printf("*** Palabra fuera de rango (%d a %d). ***\n",
                   MIN_VAL, MAX_VAL);
            clearInputBuffer();
            continue;
        }

        memory[instructionCounter++] = value;
    }

    printf("*** Error: memoria llena antes de recibir el centinela. ***\n");

    return false;
}

static bool readString(int baseAddr)
{
    char strBuffer[MAX_STRING_LENGTH + 3];
    size_t len;
    size_t i;
    char *newline;

    if (baseAddr < 0 || baseAddr >= MEM_SIZE) {
        printf("*** Error: direccion de memoria invalida para READ_STR. ***\n");
        return false;
    }

    printf("Ingrese cadena (maximo %d caracteres): ", MAX_STRING_LENGTH);

    /* Solo se limpia si una lectura numerica anterior dejo el salto de linea. */
    if (pendingInputNewline) {
        clearInputBuffer();
    }

    if (fgets(strBuffer, sizeof(strBuffer), stdin) == NULL) {
        printf("*** Error: no se pudo leer la cadena. ***\n");
        return false;
    }

    newline = strpbrk(strBuffer, "\r\n");

    if (newline != NULL) {
        *newline = '\0';
    } else if (!feof(stdin) && strlen(strBuffer) > MAX_STRING_LENGTH) {
        clearInputBuffer();
        printf("*** Error: la cadena supera los %d caracteres. ***\n",
               MAX_STRING_LENGTH);
        return false;
    }

    len = strlen(strBuffer);

    if (len > MAX_STRING_LENGTH || baseAddr + (int)len >= MEM_SIZE) {
        printf("*** Error: la cadena excede el limite permitido. ***\n");
        return false;
    }

    memory[baseAddr] = (float)(len * 1000);

    for (i = 0; i < len; i++) {
        int asciiVal = (unsigned char)strBuffer[i];
        int packedVal = ((int)i + 1) * 1000 + asciiVal;

        memory[baseAddr + 1 + (int)i] = (float)packedVal;
    }

    pendingInputNewline = false;

    return true;
}

static bool writeString(int baseAddr)
{
    float headerValue;
    int headerWord;
    int len;
    int i;

    if (baseAddr < 0 || baseAddr >= MEM_SIZE) {
        printf("*** Error: direccion de memoria invalida para WRITE_STR. ***\n");
        return false;
    }

    headerValue = memory[baseAddr];

    if (!isValidWord(headerValue) || floorf(headerValue) != headerValue) {
        printf("*** Error: encabezado de cadena invalido. ***\n");
        return false;
    }

    headerWord = (int)headerValue;
    len = headerWord / 1000;

    if (headerWord < 0 || len < 0 || len > MAX_STRING_LENGTH ||
        baseAddr + len >= MEM_SIZE) {
        printf("*** Error: cadena corrupta o fuera de los limites de memoria. ***\n");
        return false;
    }

    printf("Cadena Simpletron: ");

    for (i = 0; i < len; i++) {

        float packedValue = memory[baseAddr + 1 + i];
        int packedWord;
        int storedIndex;
        int asciiVal;

        if (!isValidWord(packedValue) || floorf(packedValue) != packedValue) {
            printf("\n*** Error: caracter invalido en la cadena. ***\n");
            return false;
        }

        packedWord = (int)packedValue;
        storedIndex = packedWord / 1000;
        asciiVal = packedWord % 1000;

        if (storedIndex != i + 1 || asciiVal < 0 || asciiVal > 255) {
            printf("\n*** Error: formato de cadena invalido. ***\n");
            return false;
        }

        printf("%c", (unsigned char)asciiVal);
    }

    printf("\n");

    return true;
}

static void executeProgram(void)
{
    bool branched;

    while (instructionCounter >= 0 && instructionCounter < MEM_SIZE) {

        float rawInstruction = memory[instructionCounter];
        int absoluteInstruction;

        if (!isValidWord(rawInstruction) || floorf(rawInstruction) != rawInstruction) {
            printRuntimeError("Instruccion invalida: debe ser un numero entero dentro del rango SML");
            return;
        }

        instructionRegister = (int)rawInstruction;

        absoluteInstruction = (instructionRegister < 0)
                                  ? -instructionRegister
                                  : instructionRegister;

        operationCode = absoluteInstruction / 1000;
        operand = absoluteInstruction % 1000;

        branched = false;

        switch (operationCode) {

            case READ: {
                float inputValue;
                int result;

                while (true) {
                    printf("? ");

                    result = scanf("%f", &inputValue);

                    if (result == EOF) {
                        printRuntimeError("Fin de entrada durante READ");
                        return;
                    }

                    if (result != 1) {
                        printf("*** Entrada no numerica. Reingresa el valor. ***\n");
                        clearInputBuffer();
                        continue;
                    }

                    pendingInputNewline = true;

                    if (!isValidWord(inputValue)) {
                        printf("*** Valor fuera de rango (%d a %d). Reingresa. ***\n",
                               MIN_VAL, MAX_VAL);
                        clearInputBuffer();
                        continue;
                    }

                    memory[operand] = inputValue;

                    break;
                }

                break;
            }

            case WRITE:

                if (floorf(memory[operand]) == memory[operand]) {
                    printf("Salida Simpletron: %+.0f\n", memory[operand]);
                } else {
                    printf("Salida Simpletron: %+.2f\n", memory[operand]);
                }

                break;

            case NEWLINE:

                printf("\n");

                break;

            case READ_STR:

                if (!readString(operand)) {
                    dumpMemory();
                    return;
                }

                break;

            case WRITE_STR:

                if (!writeString(operand)) {
                    dumpMemory();
                    return;
                }

                break;

            case LOAD:

                accumulator = memory[operand];

                break;

            case STORE:

                memory[operand] = accumulator;

                break;

            case ADD: {
                float result = accumulator + memory[operand];

                if (!isValidWord(result)) {
                    printRuntimeError("Desbordamiento del acumulador en ADD");
                    return;
                }

                accumulator = result;

                break;
            }

            case SUBTRACT: {
                float result = accumulator - memory[operand];

                if (!isValidWord(result)) {
                    printRuntimeError("Desbordamiento del acumulador en SUBTRACT");
                    return;
                }

                accumulator = result;

                break;
            }

            case DIVIDE: {
                float divisor = memory[operand];
                float result;

                if (divisor == 0.0f) {
                    printRuntimeError("Intento de dividir entre cero");
                    return;
                }

                result = accumulator / divisor;

                if (!isValidWord(result)) {
                    printRuntimeError("Desbordamiento del acumulador en DIVIDE");
                    return;
                }

                accumulator = result;

                break;
            }

            case MULTIPLY: {
                float result = accumulator * memory[operand];

                if (!isValidWord(result)) {
                    printRuntimeError("Desbordamiento del acumulador en MULTIPLY");
                    return;
                }

                accumulator = result;

                break;
            }

            case MODULUS: {
                float divisor = memory[operand];
                float result;

                if (divisor == 0.0f) {
                    printRuntimeError("Intento de modulo entre cero");
                    return;
                }

                result = fmodf(accumulator, divisor);

                if (!isValidWord(result)) {
                    printRuntimeError("Resultado fuera de rango en MODULUS");
                    return;
                }

                accumulator = result;

                break;
            }

            case EXPONENT: {
                float result = powf(accumulator, memory[operand]);

                if (!isValidWord(result)) {
                    printRuntimeError("Resultado fuera de rango en EXPONENT");
                    return;
                }

                accumulator = result;

                break;
            }

            case BRANCH:

                instructionCounter = operand;
                branched = true;

                break;

            case BRANCHNEG:

                if (accumulator < 0.0f) {
                    instructionCounter = operand;
                    branched = true;
                }

                break;

            case BRANCHZERO:

                if (accumulator == 0.0f) {
                    instructionCounter = operand;
                    branched = true;
                }

                break;

            case HALT:

                printf("*** Termino la ejecucion de Simpletron ***\n");
                dumpMemory();

                return;

            default:

                printRuntimeError("Codigo de operacion no valido");
                printf("Codigo detectado: %02d\n", operationCode);

                return;
        }

        if (!branched) {
            instructionCounter++;
        }
    }

    printf("*** La ejecucion termino al alcanzar el limite de memoria. ***\n");
    dumpMemory();
}

static void dumpMemory(void)
{
    int row;
    int col;

    printf("\nREGISTROS:\n");
    printf("acumulador:             %+10.2f\n", accumulator);
    printf("instructionCounter:        %03d\n", instructionCounter);
    printf("instructionRegister:    %+06d\n", instructionRegister);
    printf("operationCode:             %02d\n", operationCode);
    printf("operand:                  %03d\n\n", operand);

    printf("MEMORIA COMPLETA (%d posiciones):\n", MEM_SIZE);
    printf("%8s", "Bloque");

    for (col = 0; col < 10; col++) {
        printf("%11d", col);
    }

    printf("\n");

    for (row = 0; row < MEM_SIZE / 10; row++) {

        printf("%03d     ", row * 10);

        for (col = 0; col < 10; col++) {
            printf("%+10.2f ", memory[row * 10 + col]);
        }

        printf("\n");
    }

    printf("\n");
}