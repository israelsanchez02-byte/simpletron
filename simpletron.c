#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --- CODIGOS DE OPERACION SML MEJORADOS ---
// Entrada / Salida
#define READ        10
#define WRITE       11
#define NEWLINE     12
#define READ_STR    13
#define WRITE_STR   14

// Carga / Almacenamiento
#define LOAD        20
#define STORE       21

// Aritmetica (Soporta enteros y flotantes)
#define ADD         30
#define SUBTRACT    31
#define DIVIDE      32
#define MULTIPLY    33
#define MODULUS     34
#define EXPONENT    35

// Control / Bucle
#define BRANCH      40
#define BRANCHNEG   41
#define BRANCHZERO  42
#define HALT        43

// --- CONSTANTES DEL SISTEMA ---
#define MEM_SIZE    1000        // Memoria expandida a 1000 posiciones (0 a 999)
#define MIN_VAL     -99999      // Rango de palabra extendido para 5 digitos
#define MAX_VAL     99999

// --- REGISTROS Y MEMORIA ---
// Soporte de punto flotante: Se almacena como 'float' para operar preservando decimales
float memory[MEM_SIZE];
float accumulator = 0.0f;
int instructionCounter = 0;
int instructionRegister = 0;
int operationCode = 0;
int operand = 0;

// Prototipos de funciones
void loadProgram(void);
void executeProgram(void);
void dumpMemory(void);
bool isValidWord(float word);
void clearInputBuffer(void);

int main(void) {
    loadProgram();
    executeProgram();
    return 0;
}

// Limpia el buffer de teclado en lecturas
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Valida si la palabra o valor esta dentro del rango permitido
bool isValidWord(float word) {
    return (word >= (float)MIN_VAL && word <= (float)MAX_VAL);
}

// Fase 1: Carga del programa (Desde archivo "programa.simp" o teclado)
void loadProgram(void) {
    FILE *file = fopen("programa.simp", "r");
    
    // Inicializar toda la memoria en 0.0
    for (int i = 0; i < MEM_SIZE; i++) {
        memory[i] = 0.0f;
    }

    printf("*** ¡Bienvenido a Simpletron v2.0! ***\n");

    if (file != NULL) {
        // MEJORA 1: Carga desde archivo programa.simp
        printf("*** Cargando programa desde 'programa.simp'... ***\n");
        float instruction = 0.0f;
        int lineNum = 0;

        while (fscanf(file, "%f", &instruction) == 1 && instructionCounter < MEM_SIZE) {
            lineNum++;
            if ((int)instruction == 99999) { // Centinela 99999 para terminar carga
                break;
            }

            if (!isValidWord(instruction)) {
                printf("*** Error en archivo 'programa.simp' (linea %d): valor fuera de rango. ***\n", lineNum);
                printf("*** Abortando lectura de archivo. ***\n");
                fclose(file);
                exit(1);
            }

            memory[instructionCounter] = instruction;
            instructionCounter++;
        }
        fclose(file);
        printf("*** Carga desde archivo completada exitosamente (%d instrucciones). ***\n\n", instructionCounter);

    } else {
        // Carga interactiva desde teclado
        printf("*** Archivo 'programa.simp' no encontrado. Modo Interactivo Activado. ***\n");
        printf("*** Introduzca su programa una instruccion a la vez. ***\n");
        printf("*** Ingrese 99999 para terminar la carga. ***\n\n");

        float instruction = 0.0f;

        while (instructionCounter < MEM_SIZE) {
            printf("%03d ? ", instructionCounter);
            
            if (scanf("%f", &instruction) != 1) {
                printf("*** Entrada invalida. Introduzca un numero numerico. ***\n");
                clearInputBuffer();
                continue;
            }

            if ((int)instruction == 99999) {
                break;
            }

            if (!isValidWord(instruction)) {
                printf("*** Palabra invalida (debe estar entre %d y +%d). ***\n", MIN_VAL, MAX_VAL);
                continue;
            }

            memory[instructionCounter] = instruction;
            instructionCounter++;
        }
        printf("\n*** Se termino de cargar el programa manualmente ***\n");
    }

    printf("*** Comienza la ejecucion del programa ***\n\n");
    instructionCounter = 0;
}

// Fase 2: Ciclo de Busqueda, Decodificacion y Ejecucion
void executeProgram(void) {
    bool isRunning = true;
    bool branched = false;

    while (isRunning && instructionCounter < MEM_SIZE) {
        // Fetch
        instructionRegister = (int)memory[instructionCounter];

        // Decode (Instruccion de 5 digitos: 2 digitos para OpCode, 3 para Operando/Direccion 0-999)
        int absInstruction = abs(instructionRegister);
        operationCode = absInstruction / 1000;
        operand = absInstruction % 1000;

        branched = false;

        // Execute
        switch (operationCode) {
            case READ:
                printf("? ");
                while (scanf("%f", &memory[operand]) != 1 || !isValidWord(memory[operand])) {
                    printf("*** Valor fuera de rango (%d a +%d). Reingrese: ***\n? ", MIN_VAL, MAX_VAL);
                    clearInputBuffer();
                }
                break;

            case WRITE:
                // Si el valor es entero puro se imprime sin decimales, si no como float
                if (memory[operand] == (int)memory[operand]) {
                    printf("Salida Simpletron: %+06d\n", (int)memory[operand]);
                } else {
                    printf("Salida Simpletron: %+08.2f\n", memory[operand]);
                }
                break;

            case NEWLINE: // MEJORA 5: Imprimir salto de linea
                printf("\n");
                break;

            case READ_STR: { // MEJORA 6: Entrada de cadenas en formato [ Longitud | ASCII 3d ]
                char strBuffer[256];
                printf("Ingrese cadena: ");
                clearInputBuffer();
                if (fgets(strBuffer, sizeof(strBuffer), stdin) != NULL) {
                    // Quitar salto de linea sobrante
                    strBuffer[strcspn(strBuffer, "\r\n")] = '\0';
                }

                int len = strlen(strBuffer);
                int baseAddr = operand;

                if (baseAddr + len >= MEM_SIZE) {
                    printf("*** Error: La cadena excede el limite de memoria ***\n");
                    dumpMemory();
                    return;
                }

                // Posicion base: almacena la longitud (ejemplo: 05000 para len 5)
                memory[baseAddr] = (float)(len * 1000);

                // Posiciones subsecuentes: [ Indice_1d | ASCII_3d ] (ej. 01077 para 'M')
                for (int i = 0; i < len; i++) {
                    int posIdx = i + 1;
                    int asciiVal = (unsigned char)strBuffer[i];
                    int packedVal = (posIdx * 1000) + asciiVal;
                    memory[baseAddr + 1 + i] = (float)packedVal;
                }
                break;
            }

            case WRITE_STR: { // MEJORA 7: Salida de cadenas procesando memoria
                int baseAddr = operand;
                if (baseAddr < 0 || baseAddr >= MEM_SIZE) {
                    printf("*** Error: Direccion de memoria invalida para cadena ***\n");
                    dumpMemory();
                    return;
                }

                int headerWord = (int)memory[baseAddr];
                int len = headerWord / 1000; // Extrae los 2 primeros digitos (longitud)

                if (len < 0 || baseAddr + len >= MEM_SIZE) {
                    printf("*** Error: Cadena corrupta o fuera de limites ***\n");
                    dumpMemory();
                    return;
                }

                printf("Cadena Simpletron: ");
                for (int i = 0; i < len; i++) {
                    int word = (int)memory[baseAddr + 1 + i];
                    int asciiVal = word % 1000; // Extrae los ultimos 3 digitos (ASCII)
                    printf("%c", (char)asciiVal);
                }
                printf("\n");
                break;
            }

            case LOAD:
                accumulator = memory[operand];
                break;

            case STORE:
                memory[operand] = accumulator;
                break;

            case ADD:
                accumulator += memory[operand];
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento del acumulador ***\n");
                    dumpMemory();
                    return;
                }
                break;

            case SUBTRACT:
                accumulator -= memory[operand];
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento del acumulador ***\n");
                    dumpMemory();
                    return;
                }
                break;

            case DIVIDE:
                if (memory[operand] == 0.0f) {
                    printf("*** Intento de dividir entre cero ***\n");
                    dumpMemory();
                    return;
                }
                accumulator /= memory[operand];
                break;

            case MULTIPLY:
                accumulator *= memory[operand];
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento del acumulador ***\n");
                    dumpMemory();
                    return;
                }
                break;

            case MODULUS: // MEJORA 3: Operacion residuo/modulo
                if ((int)memory[operand] == 0) {
                    printf("*** Intento de division/modulo entre cero ***\n");
                    dumpMemory();
                    return;
                }
                accumulator = (float)((int)accumulator % (int)memory[operand]);
                break;

            case EXPONENT: // MEJORA 4: Exponenciacion A^B
                accumulator = powf(accumulator, memory[operand]);
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento en exponenciacion ***\n");
                    dumpMemory();
                    return;
                }
                break;

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
                printf("*** Termino la ejecucion de Simpletron ***\n\n");
                isRunning = false;
                dumpMemory();
                return;

            default:
                printf("*** Codigo de operacion no valido (%02d) ***\n", operationCode);
                dumpMemory();
                return;
        }

        if (!branched) {
            instructionCounter++;
        }
    }
}

// Fase 3: Vaciado de Memoria (Dump adaptado a 1000 posiciones)
void dumpMemory(void) {
    printf("\nREGISTROS:\n");
    printf("acumulador:          %+08.2f\n", accumulator);
    printf("instructionCounter:     %03d\n", instructionCounter);
    printf("instructionRegister: %+06d\n", instructionRegister);
    printf("operationCode:          %02d\n", operationCode);
    printf("operand:               %03d\n\n", operand);

    printf("MEMORIA (Muestra primeros 100 bloques y celdas activas):\n");
    printf("%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

    // Muestra las primeras filas y celdas modificadas/usadas para no saturar pantalla
    for (int row = 0; row < MEM_SIZE / 10; row++) {
        bool hasData = false;
        for (int col = 0; col < 10; col++) {
            if (memory[row * 10 + col] != 0.0f) {
                hasData = true;
                break;
            }
        }

        // Imprime solo filas dentro del rango base (0-90) o las que contengan datos
        if (row < 10 || hasData || row == (instructionCounter / 10)) {
            printf("%03d ", row * 10);
            for (int col = 0; col < 10; col++) {
                printf("%+06d ", (int)memory[row * 10 + col]);
            }
            printf("\n");
        }
    }
    printf("\n");
}