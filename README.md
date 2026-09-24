# Simulador de Computadora Simpletron (SML)

Este repositorio contiene un simulador funcional de la computadora **Simpletron** y su lenguaje de programación **SML (Simpletron Machine Language)** hecho en lenguaje C. 

El proyecto muestra la evolución del código desde la versión inicial básica hasta una versión con funciones extra agregadas.

---

## Características del Programa

### Versión Básica (v1.0)
- **Memoria:** 100 espacios en memoria (`00` al `99`).
- **Formato de datos:** Instrucciones de 4 dígitos (números entre `-9999` y `+9999`).
- **Fin de carga:** Termina de pedir datos cuando metes el número `9999`.
- **Registros:**
  - `accumulator`: Guarda el resultado de las operaciones.
  - `instructionCounter`: Indica en qué línea de memoria vamos.
  - `instructionRegister`: Guarda la instrucción que se va a procesar.
  - `operationCode`: Los primeros 2 dígitos que dicen qué hacer.
  - `operand`: Los últimos 2 dígitos con la posición de memoria a usar.

### Versión Mejorada (v2.0)
- **Más Memoria:** Se aumentó a 1000 espacios (`000` al `999`).
- **Formato extendido:** Se usan 5 dígitos para aceptar las posiciones de 3 dígitos.
- **Lectura desde archivo:** Lee automáticamente el archivo `programa.simp`. Si no existe, pide ingresar las instrucciones por teclado.
- **Punto flotante:** Ya acepta y calcula números con decimales.
- **Nuevas instrucciones:** Módulo/Residuo (`34`), Exponenciación (`35`), Salto de línea (`12`), Leer texto (`13`) y Mostrar texto (`14`).

---

## Cómo Compilar y Ejecutar

### Requisitos
- Compilador de C estándar instalado (como `gcc`).

### Compilacion
Abre la consola en la carpeta del proyecto y corre:

```bash
gcc simpletron.c -o simpletron -lm


Ejecucion
En Windows:

simpletron.exe


SML

Entrada y Salida
10 READ: Pide un numero por teclado y lo guarda en la direccion elegida.

11 WRITE: Muestra en pantalla el valor que hay en esa direccion.

12 NEWLINE: Imprime un salto de linea.

13 READ_STR: Pide un texto por teclado y lo guarda en memoria.

14 WRITE_STR: Muestra el texto guardado en memoria.

Carga y Almacenamiento
20 LOAD: Pasa el dato de la memoria al acumulador.

21 STORE: Guarda el valor del acumulador en la memoria.

Operaciones Matematicas
30 ADD: Suma el valor de la memoria al acumulador.

31 SUBTRACT: Resta el valor de memoria al acumulador.

32 DIVIDE: Divide el acumulador entre el valor de la memoria.

33 MULTIPLY: Multiplica el acumulador por el valor de memoria.

34 MODULUS: Saca el residuo de la division.

35 EXPONENT: Eleva el acumulador a la potencia elegida.

Saltos y Control
40 BRANCH: Salta directo a la linea de memoria seleccionada.

41 BRANCHNEG: Salta solo si el acumulador es negativo.

42 BRANCHZERO: Salta solo si el acumulador es igual a 0.

43 HALT: Termina la ejecucion del programa.


Errores Fatales y Vaciado de Memoria (Dump)
El simulador detecta si algo sale mal mientras ejecuta el código y se detiene llamando a la función dumpMemory(). Detecta fallas como:

Querer dividir entre cero (DIVIDE o MODULUS).

Desbordamiento (si los números se salen del límite soportado).

Usar un código de operación que no existe.

Intentar leer posiciones fuera del límite de la memoria.

Cuando el programa termina, se imprimen en pantalla los valores finales de todos los registros y la tabla con el contenido completo de la memoria.
