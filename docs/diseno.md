# Diseño del proyecto Simpletron

## 1. Organización general

El proyecto está dividido en tres módulos principales:

- `simpletron.c`: contiene el simulador de la computadora Simpletron y ejecuta programas SML.
- `src/expresiones/expresiones.c`: convierte expresiones infijas a postfijas y calcula su resultado.
- `src/compilador/compilador.c`: lee programas escritos en Simple y genera instrucciones SML.

## 2. Simulador Simpletron

El simulador utiliza una memoria de 1000 posiciones y los registros del procesador: acumulador, contador de instrucciones, registro de instrucción, código de operación y operando.

Su ciclo de ejecución sigue tres etapas:

1. Obtener la instrucción indicada por el contador.
2. Decodificar el código de operación y su operando.
3. Ejecutar la operación y actualizar los registros.

El programa también valida instrucciones, direcciones de memoria, división entre cero y desbordamientos.

## 3. Evaluador de expresiones

El evaluador utiliza el algoritmo Shunting Yard para convertir expresiones infijas a postfijas.

Después utiliza una pila para evaluar el resultado, respetando la precedencia y asociatividad de los operadores. También valida los agrupadores, los operadores y los errores durante la evaluación.

## 4. Compilador de Simple a SML

El compilador realiza dos pasadas:

### Primera pasada

- Lee y valida las líneas del programa Simple.
- Registra los números de línea y las variables.
- Genera instrucciones preliminares.
- Reserva espacios para datos y referencias pendientes.

### Segunda pasada

- Resuelve las referencias a números de línea.
- Completa las direcciones de memoria pendientes.
- Genera el programa SML final.

Las instrucciones `let` utilizan conversión de expresiones infijas a postfijas para generar las operaciones necesarias y almacenar resultados en posiciones temporales.

## 5. Control de flujo

El compilador transforma las instrucciones `goto` e `if` en instrucciones de salto de SML.

Las condiciones relacionales se implementan mediante operaciones de resta y saltos condicionales.

## 6. Archivos generados

Los programas fuente de Simple se guardan en `programas/simple/` y sus programas SML generados en `programas/sml/`.

Las pruebas del simulador y del evaluador se organizan dentro de `pruebas/`.
