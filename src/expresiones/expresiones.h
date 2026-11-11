#ifndef SIMPLETRON_EXPRESIONES_H
#define SIMPLETRON_EXPRESIONES_H

#define MAX_TOKENS 256
#define MAX_TOKEN_LENGTH 64

int convertirPostfija(
    const char *expresion,
    char salida[MAX_TOKENS][MAX_TOKEN_LENGTH],
    int *cantidad
);

#endif
