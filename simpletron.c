#include <stdio.h>
#include <stdbool.h>

// --- CODIGOS DE OPERACION SML ---
#define READ 10
#define WRITE 11

#define LOAD 20
#define STORE 21

#define ADD 30
#define SUBTRACT 31
#define DIVIDE 32
#define MULTIPLY 33

#define BRANCH 40
#define BRANCHNEG 41
#define BRANCHZERO 42
#define HALT 43

// --- CONSTANTES DEL SISTEMA ---
#define MEM_SIZE 100
#define MIN_VAL -9999
#define MAX_VAL 9999

// --- REGISTROS Y MEMORIA ---
int memory[MEM_SIZE];
int accumulator = 0;
int instructionCounter = 0;
int instructionRegister = 0;
int operationCode = 0;
int operand = 0;

// Declaración de funciones
void loadProgram(void);
void executeProgram(void);
void dumpMemory(void);
bool isValidWord(int word);

int main(void) {
    loadProgram();
    executeProgram();
    return 0;
}

// Función para verificar si la palabra ingresada está en el rango permitido
bool isValidWord(int word) {
    return (word >= MIN_VAL && word <= MAX_VAL);
}

// Fase 1: Carga del programa en la memoria
void loadProgram(void) {
    int instruction = 0;
    
    // Inicialización de la memoria en 0
    for (int i = 0; i < MEM_SIZE; i++) {
        memory[i] = 0;
    }

    printf("*** ¡Bienvenido a Simpletron! ***\n");
    printf("*** Introduzca su programa una instrucción ***\n");
    printf("*** (o palabra de datos) a la vez en la línea ***\n");
    printf("*** de texto de entrada. Yo indicaré el número ***\n");
    printf("*** de posición y una interrogación (?). Usted ***\n");
    printf("*** tecleará entonces la palabra para esa ***\n");
    printf("*** posición. Haga clic en el botón LISTO para ***\n");
    printf("*** dejar de introducir su programa. ***\n\n");

    while (instructionCounter < MEM_SIZE) {
        printf("%02d ? ", instructionCounter);
        scanf("%d", &instruction);

        // Centinela de fin de carga
        if (instruction == 9999) {
            break;
        }

        // Validación de datos de entrada
        if (!isValidWord(instruction)) {
            printf("*** Palabra inválida (debe estar entre -9999 y +9998). Intente de nuevo. ***\n");
            continue;
        }

        memory[instructionCounter] = instruction;
        instructionCounter++;
    }

    printf("\n*** Se terminó de cargar el programa ***\n");
    printf("*** Comienza la ejecución del programa ***\n\n");

    // Reiniciar contador de instrucciones para iniciar ejecución desde la posición 00
    instructionCounter = 0;
}

// Fase 2: Ciclo de Búsqueda, Decodificación y Ejecución
void executeProgram(void) {
    bool isRunning = true;
    bool branched = false;

    while (isRunning && instructionCounter < MEM_SIZE) {
        // Fetch (Búsqueda)
        instructionRegister = memory[instructionCounter];

        // Decode (Decodificación)
        operationCode = instructionRegister / 100;
        operand = instructionRegister % 100;

        branched = false;

        // Execute (Ejecución)
        switch (operationCode) {
            case READ:
                printf("? ");
                scanf("%d", &memory[operand]);
                while (!isValidWord(memory[operand])) {
                    printf("*** Valor fuera de rango (-9999 a +9999). Reingrese: ***\n? ");
                    scanf("%d", &memory[operand]);
                }
                break;

            case WRITE:
                printf("Salida Simpletron: %+05d\n", memory[operand]);
                break;

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
                    printf("*** La ejecución de Simpletron terminó anormalmente ***\n\n");
                    dumpMemory();
                    return;
                }
                break;

            case SUBTRACT:
                accumulator -= memory[operand];
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento del acumulador ***\n");
                    printf("*** La ejecución de Simpletron terminó anormalmente ***\n\n");
                    dumpMemory();
                    return;
                }
                break;

            case DIVIDE:
                if (memory[operand] == 0) {
                    printf("*** Intento de dividir entre cero ***\n");
                    printf("*** La ejecución de Simpletron terminó anormalmente ***\n\n");
                    dumpMemory();
                    return;
                }
                accumulator /= memory[operand];
                break;

            case MULTIPLY:
                accumulator *= memory[operand];
                if (!isValidWord(accumulator)) {
                    printf("*** Desbordamiento del acumulador ***\n");
                    printf("*** La ejecución de Simpletron terminó anormalmente ***\n\n");
                    dumpMemory();
                    return;
                }
                break;

            case BRANCH:
                instructionCounter = operand;
                branched = true;
                break;

            case BRANCHNEG:
                if (accumulator < 0) {
                    instructionCounter = operand;
                    branched = true;
                }
                break;

            case BRANCHZERO:
                if (accumulator == 0) {
                    instructionCounter = operand;
                    branched = true;
                }
                break;

            case HALT:
                printf("*** Terminó la ejecución de Simpletron ***\n\n");
                isRunning = false;
                dumpMemory();
                return;

            default:
                printf("*** Código de operación no válido (%02d) ***\n", operationCode);
                printf("*** La ejecución de Simpletron terminó anormalmente ***\n\n");
                dumpMemory();
                return;
        }

        // Si no hubo bifurcación, se incrementa al siguiente elemento
        if (!branched) {
            instructionCounter++;
        }
    }
}

// Fase 3: Vaciado de Memoria (Memory Dump)
void dumpMemory(void) {
    printf("Registros:\n");
    printf("acumulador:          %+05d\n", accumulator);
    printf("instructionCounter:     %02d\n", instructionCounter);
    printf("instructionRegister: %+05d\n", instructionRegister);
    printf("operationcode:          %02d\n", operationCode);
    printf("operand:                %02d\n\n", operand);

    printf("MEMORIA:\n");
    printf("%8d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

    for (int row = 0; row < 10; row++) {
        printf("%2d ", row * 10);
        for (int col = 0; col < 10; col++) {
            printf("%+05d ", memory[row * 10 + col]);
        }
        printf("\n");
    }
}