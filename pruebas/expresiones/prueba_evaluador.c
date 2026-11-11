#include <stdio.h>
#include <math.h>

#include "../../src/expresiones/expresiones.h"

static int pruebas_correctas = 0;
static int pruebas_fallidas = 0;

static void probar_resultado(const char *expresion, double esperado) {
    char postfija[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int cantidad = 0;
    double resultado = 0.0;
    int i;

    printf("\nExpresion: %s\n", expresion);

    if (!convertirPostfija(expresion, postfija, &cantidad)) {
        printf("FALLO: no se pudo convertir la expresion.\n");
        pruebas_fallidas++;
        return;
    }

    printf("Postfija: ");
    for (i = 0; i < cantidad; i++) {
        printf("%s ", postfija[i]);
    }
    printf("\n");

    if (!evaluarPostfija(postfija, cantidad, &resultado)) {
        printf("FALLO: no se pudo evaluar la expresion.\n");
        pruebas_fallidas++;
        return;
    }

    printf("Resultado: %.10g | Esperado: %.10g\n", resultado, esperado);

    if (fabs(resultado - esperado) < 0.0000001) {
        printf("PRUEBA CORRECTA\n");
        pruebas_correctas++;
    } else {
        printf("FALLO: el resultado no coincide.\n");
        pruebas_fallidas++;
    }
}

static void probar_sintaxis_invalida(const char *expresion) {
    char postfija[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int cantidad = 0;

    printf("\nExpresion invalida: %s\n", expresion);

    if (!convertirPostfija(expresion, postfija, &cantidad)) {
        printf("PRUEBA CORRECTA: se rechazo la expresion.\n");
        pruebas_correctas++;
    } else {
        printf("FALLO: se acepto una expresion invalida.\n");
        pruebas_fallidas++;
    }
}

static void probar_division_cero(void) {
    char postfija[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int cantidad = 0;
    double resultado = 0.0;

    printf("\nPrueba de division entre cero: 8 / 0\n");

    if (convertirPostfija("8 / 0", postfija, &cantidad) &&
        !evaluarPostfija(postfija, cantidad, &resultado)) {
        printf("PRUEBA CORRECTA: se detecto la division entre cero.\n");
        pruebas_correctas++;
    } else {
        printf("FALLO: no se detecto la division entre cero.\n");
        pruebas_fallidas++;
    }
}

int main(void) {
    printf("=== PRUEBAS AUTOMATIZADAS DEL EVALUADOR ===\n");

    probar_resultado("2 + 3 * 4", 14.0);
    probar_resultado("(2 + 3) * 4", 20.0);
    probar_resultado("2 ^ 3 ^ 2", 512.0);
    probar_resultado("15 % 4", 3.0);
    probar_resultado("1.5 + 2.5 * 2", 6.5);
    probar_resultado("{2 + [3 * (4 - 1)]}", 11.0);
    probar_resultado("-2 ^ 2", -4.0);

    probar_sintaxis_invalida("(2 + 3");
    probar_sintaxis_invalida("2 + * 3");
    probar_sintaxis_invalida("(2 + 3)]");
    probar_division_cero();

    printf("\n========================================\n");
    printf("Pruebas correctas: %d\n", pruebas_correctas);
    printf("Pruebas fallidas:  %d\n", pruebas_fallidas);

    return pruebas_fallidas == 0 ? 0 : 1;
}
