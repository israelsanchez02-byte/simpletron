
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

/* Limites del evaluador */
#define MAX_EXPR 256
#define MAX_TOKENS 256
#define MAX_TOKEN_LENGTH 64
#define MAX_DEPTH 100

/* Determina la precedencia de los operadores */
int precedencia(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;

        case '*':
        case '/':
        case '%':
            return 2;

        case 'p':
        case 'n':
            return 3;

        case '^':
            return 4;

        default:
            return 0;
    }
}

/* Comprueba si un caracter es un operador aritmetico */
int esOperador(char c) {
    return c == '+' || c == '-' ||
           c == '*' || c == '/' ||
           c == '%' || c == '^';
}

/* Comprueba si un caracter abre un agrupador */
int esApertura(char c) {
    return c == '(' || c == '[' || c == '{';
}

/* Verifica que los agrupadores coincidan */
int coincide(char apertura, char cierre) {
    return (apertura == '(' && cierre == ')') ||
           (apertura == '[' && cierre == ']') ||
           (apertura == '{' && cierre == '}');
}

/* Determina si un operador es asociativo por la derecha */
int esAsociativoDerecha(char op) {
    return op == '^' || op == 'p' || op == 'n';
}

/*
    Convierte una expresion infija a postfija
    utilizando el algoritmo Shunting Yard.
*/
int convertirPostfija(
    const char *expresion,
    char salida[MAX_TOKENS][MAX_TOKEN_LENGTH],
    int *cantidad
) {
    char pila[MAX_TOKENS];

    int tope = -1;
    int profundidad = 0;
    int i = 0;
    int esperaOperando = 1;

    *cantidad = 0;

    while (expresion[i] != '\0') {

        /* Ignorar espacios */
        if (isspace((unsigned char)expresion[i])) {
            i++;
            continue;
        }

        /* Lectura de numeros enteros o decimales */
        if (isdigit((unsigned char)expresion[i]) ||
            expresion[i] == '.') {

            if (!esperaOperando) {
                printf("Error: falta un operador.\n");
                return 0;
            }

            char *fin;
            errno = 0;

            double numero = strtod(&expresion[i], &fin);

            if (fin == &expresion[i] ||
                errno == ERANGE ||
                !isfinite(numero)) {

                printf("Error: numero invalido.\n");
                return 0;
            }

            int longitud = (int)(fin - &expresion[i]);

            if (longitud >= MAX_TOKEN_LENGTH ||
                *cantidad >= MAX_TOKENS) {

                printf("Error: numero o expresion demasiado grande.\n");
                return 0;
            }

            strncpy(
                salida[*cantidad],
                &expresion[i],
                (size_t)longitud
            );

            salida[*cantidad][longitud] = '\0';

            (*cantidad)++;

            i += longitud;
            esperaOperando = 0;
        }

        /* Apertura de parentesis, corchetes o llaves */
        else if (esApertura(expresion[i])) {

            if (!esperaOperando) {
                printf("Error: falta un operador antes del agrupador.\n");
                return 0;
            }

            if (profundidad >= MAX_DEPTH) {
                printf("Error: se excedio la profundidad maxima de %d.\n",
                       MAX_DEPTH);
                return 0;
            }

            if (tope >= MAX_TOKENS - 1) {
                printf("Error: expresion demasiado compleja.\n");
                return 0;
            }

            pila[++tope] = expresion[i];

            profundidad++;

            i++;
            esperaOperando = 1;
        }

        /* Cierre de parentesis, corchetes o llaves */
        else if (expresion[i] == ')' ||
                 expresion[i] == ']' ||
                 expresion[i] == '}') {

            if (esperaOperando) {
                printf("Error: agrupador vacio o expresion incompleta.\n");
                return 0;
            }

            /* Sacar operadores hasta encontrar la apertura */
            while (tope >= 0 && !esApertura(pila[tope])) {

                if (*cantidad >= MAX_TOKENS) {
                    printf("Error: demasiados elementos.\n");
                    return 0;
                }

                salida[*cantidad][0] = pila[tope--];
                salida[*cantidad][1] = '\0';

                (*cantidad)++;
            }

            /* Comprobar que exista la apertura correspondiente */
            if (tope < 0 ||
                !coincide(pila[tope], expresion[i])) {

                printf("Error: agrupadores desbalanceados o incorrectos.\n");
                return 0;
            }

            tope--;

            profundidad--;

            i++;
            esperaOperando = 0;
        }

        /* Operadores aritmeticos */
        else if (esOperador(expresion[i])) {

            char operador = expresion[i];

            /* Manejo de signos unarios */
            if (esperaOperando) {

                if (operador != '+' && operador != '-') {
                    printf("Error: operador sin operando.\n");
                    return 0;
                }

                /* p representa positivo y n representa negativo */
                operador = (operador == '+') ? 'p' : 'n';

                if (tope >= MAX_TOKENS - 1) {
                    printf("Error: expresion demasiado compleja.\n");
                    return 0;
                }

                pila[++tope] = operador;

                i++;

                continue;
            }

            /*
                Sacar operadores de mayor precedencia.
                La potencia y los signos unarios se procesan
                con asociatividad por la derecha.
            */
            while (
                tope >= 0 &&
                !esApertura(pila[tope]) &&
                (
                    precedencia(pila[tope]) > precedencia(operador) ||
                    (
                        precedencia(pila[tope]) == precedencia(operador) &&
                        !esAsociativoDerecha(operador)
                    )
                )
            ) {

                if (*cantidad >= MAX_TOKENS) {
                    printf("Error: demasiados elementos.\n");
                    return 0;
                }

                salida[*cantidad][0] = pila[tope--];
                salida[*cantidad][1] = '\0';

                (*cantidad)++;
            }

            if (tope >= MAX_TOKENS - 1) {
                printf("Error: expresion demasiado compleja.\n");
                return 0;
            }

            pila[++tope] = operador;

            i++;
            esperaOperando = 1;
        }

        /* Caracter no reconocido */
        else {
            printf("Error: caracter no valido '%c'.\n",
                   expresion[i]);

            return 0;
        }

    }

    /* Verificar que la expresion no termine con un operador */
    if (esperaOperando) {
        printf("Error: la expresion esta incompleta.\n");
        return 0;
    }

    /* Vaciar la pila de operadores */
    while (tope >= 0) {

        /* Si queda una apertura, falta su cierre */
        if (esApertura(pila[tope])) {
            printf("Error: agrupador sin cierre.\n");
            return 0;
        }

        if (*cantidad >= MAX_TOKENS) {
            printf("Error: demasiados elementos.\n");
            return 0;
        }

        salida[*cantidad][0] = pila[tope--];
        salida[*cantidad][1] = '\0';

        (*cantidad)++;
    }

    if (profundidad != 0) {
        printf("Error: agrupadores desbalanceados.\n");
        return 0;
    }

    return 1;
}

/*
    Evalua una expresion postfija utilizando
    una pila de valores numericos.
*/
int evaluarPostfija(
    char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH],
    int cantidad,
    double *resultado
) {
    double pila[MAX_TOKENS];

    int tope = -1;

    for (int i = 0; i < cantidad; i++) {

        char *fin;
        errno = 0;

        double numero = strtod(tokens[i], &fin);

        /* Si el token es un numero */
        if (fin != tokens[i] && *fin == '\0') {

            if (errno == ERANGE || !isfinite(numero)) {
                printf("Error: numero fuera de rango.\n");
                return 0;
            }

            if (tope >= MAX_TOKENS - 1) {
                printf("Error: pila llena.\n");
                return 0;
            }

            pila[++tope] = numero;
        }

        /* Operador unario positivo o negativo */
        else if (
            strcmp(tokens[i], "p") == 0 ||
            strcmp(tokens[i], "n") == 0
        ) {

            if (tope < 0) {
                printf("Error: falta un operando.\n");
                return 0;
            }

            if (tokens[i][0] == 'n') {
                pila[tope] = -pila[tope];
            }
        }

        /* Operadores binarios */
        else {

            if (tope < 1) {
                printf("Error: faltan operandos.\n");
                return 0;
            }

            double b = pila[tope--];
            double a = pila[tope];

            switch (tokens[i][0]) {

                case '+':
                    pila[tope] = a + b;
                    break;

                case '-':
                    pila[tope] = a - b;
                    break;

                case '*':
                    pila[tope] = a * b;
                    break;

                case '/':
                    if (b == 0.0) {
                        printf("Error: division entre cero.\n");
                        return 0;
                    }

                    pila[tope] = a / b;
                    break;

                case '%':
                    if (b == 0.0) {
                        printf("Error: modulo entre cero.\n");
                        return 0;
                    }

                    pila[tope] = fmod(a, b);
                    break;

                case '^':
                    pila[tope] = pow(a, b);
                    break;

                default:
                    printf("Error: operador desconocido.\n");
                    return 0;
            }

            if (!isfinite(pila[tope])) {
                printf("Error: resultado fuera de rango.\n");
                return 0;
            }
        }
    }

    /* Debe quedar exactamente un resultado */
    if (tope != 0) {
        printf("Error: expresion postfija incorrecta.\n");
        return 0;
    }

    *resultado = pila[0];

    return 1;
}

/* Programa principal */
int main(void) {

    char expresion[MAX_EXPR];

    char postfija[MAX_TOKENS][MAX_TOKEN_LENGTH];

    int cantidad;

    double resultado;

    printf("=== EVALUADOR DE EXPRESIONES ===\n");
    printf("Ingrese una expresion: ");

    if (fgets(expresion, sizeof(expresion), stdin) == NULL) {
        printf("Error al leer la expresion.\n");
        return 1;
    }

    /* Verificar que la entrada no exceda el limite */
    if (strchr(expresion, '\n') == NULL && !feof(stdin)) {

        int c;

        while ((c = getchar()) != '\n' && c != EOF) {
            /* Limpiar el resto de la entrada */
        }

        printf("Error: expresion demasiado larga.\n");

        return 1;
    }

    expresion[strcspn(expresion, "\n")] = '\0';

    /* Primera etapa: conversion infija a postfija */
    if (!convertirPostfija(expresion, postfija, &cantidad)) {
        return 1;
    }

    printf("\nExpresion postfija: ");

    for (int i = 0; i < cantidad; i++) {
        printf("%s ", postfija[i]);
    }

    printf("\n");

    /* Segunda etapa: evaluacion de la expresion */
    if (!evaluarPostfija(postfija, cantidad, &resultado)) {
        return 1;
    }

    printf("Resultado: %.10g\n", resultado);

    return 0;
}
