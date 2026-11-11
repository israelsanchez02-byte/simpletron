# Proyecto Simpletron: Simulador, Evaluador y Compilador

Proyecto desarrollado en lenguaje C que integra un simulador de la computadora Simpletron, un evaluador de expresiones aritméticas y un compilador de un lenguaje sencillo llamado Simple hacia SML.

## Autor

**Israel Sánchez Trejo**

## 1. Componentes del proyecto

### Simulador Simpletron

Archivo principal: `simpletron.c`

Implementa una computadora Simpletron con:

- Memoria de 1000 posiciones.
- Registros del procesador.
- Ciclo de búsqueda, decodificación y ejecución.
- Lectura de programas SML desde archivos.
- Operaciones aritméticas con números decimales.
- Entrada y salida de números y cadenas.
- Saltos condicionales e incondicionales.
- Detección de errores fatales y desbordamientos.
- Volcado completo de memoria al terminar la ejecución.

### Evaluador de expresiones

Archivo: `src/expresiones/expresiones.c`

Sus funciones principales son:

- Conversión de expresiones infijas a postfijas mediante Shunting Yard.
- Evaluación de expresiones postfijas utilizando una pila.
- Manejo de precedencia y asociatividad.
- Soporte para números de varios dígitos y decimales.
- Operadores `+`, `-`, `*`, `/`, `%` y `^`.
- Validación de paréntesis, corchetes y llaves.
- Detección de errores sintácticos y división entre cero.

### Lenguaje Simple y compilador

Archivo: `src/compilador/compilador.c`

El compilador transforma programas escritos en Simple a instrucciones SML.

Comandos disponibles:

- `rem`: comentario.
- `input`: entrada de datos.
- `let`: asignación y operaciones aritméticas.
- `print`: mostrar resultados.
- `goto`: salto a una línea.
- `if`: condición con operador relacional y salto.
- `end`: terminar el programa.

El lenguaje utiliza números de línea ascendentes, variables de una letra y constantes enteras.

Operadores aritméticos: `+`, `-`, `*`, `/`, `^`.

Operadores relacionales: `<`, `<=`, `>`, `>=`, `==`, `!=`.

El compilador trabaja en dos pasadas: primero registra líneas, variables y referencias; después resuelve las direcciones pendientes y genera el archivo SML.

## 2. Instrucciones SML

| Código | Operación | Descripción |
|---|---|---|
| 10 | READ | Leer un número |
| 11 | WRITE | Mostrar un número |
| 12 | NEWLINE | Salto de línea |
| 13 | READ_STR | Leer una cadena |
| 14 | WRITE_STR | Mostrar una cadena |
| 20 | LOAD | Cargar en el acumulador |
| 21 | STORE | Guardar desde el acumulador |
| 30 | ADD | Suma |
| 31 | SUBTRACT | Resta |
| 32 | DIVIDE | División |
| 33 | MULTIPLY | Multiplicación |
| 34 | MODULUS | Residuo |
| 35 | EXPONENT | Exponenciación |
| 40 | BRANCH | Salto incondicional |
| 41 | BRANCHNEG | Salto si es negativo |
| 42 | BRANCHZERO | Salto si es cero |
| 43 | HALT | Terminar ejecución |

## 3. Compilación

Se requiere un compilador de C, por ejemplo GCC.

Desde la carpeta principal del proyecto:

```bash
gcc simpletron.c -o simpletron.exe -lm
gcc src/expresiones/expresiones.c -o src/expresiones/evaluador.exe -lm
gcc src/compilador/compilador.c src/expresiones/expresiones.c -DEXPRESIONES_LIB -o src/compilador/compilador.exe -lm
```

## 4. Ejecución

### Evaluador de expresiones

```bash
./src/expresiones/evaluador.exe
```

El programa solicita una expresión y muestra su conversión a postfija y el resultado.

### Compilador de Simple

Ejemplo:

```bash
./src/compilador/compilador.exe programas/simple/prueba_compilador.simple
```

El compilador genera el archivo correspondiente dentro de `programas/sml/`.

### Simulador Simpletron

Para ejecutar el SML generado:

```bash
./simpletron.exe programas/sml/prueba_compilador.sml
```

También se puede ejecutar otro programa cambiando la ruta del archivo SML.




## Ejemplo completo: programa Simple a SML

Archivo de entrada: `programas/simple/prueba_compilador.simple`

```text
10 rem prueba del compilador
20 input a
30 let b = a * 2
40 if b >= 10 goto 70
50 print b
60 goto 80
70 print b + 1
80 end
```

Compilar el programa:

```bash
./src/compilador/compilador.exe programas/simple/prueba_compilador.simple
```

Resultado de la compilación:

```text
Compilacion correcta.
Lineas fuente: 8
Instrucciones SML: 17
Celdas de datos: 31
Memoria utilizada: 48 / 1000
Archivo generado: programas/sml/prueba_compilador.sml
```

Ejecutar el archivo SML:

```bash
./simpletron.exe programas/sml/prueba_compilador.sml
```

Resultados obtenidos:

| Entrada | Salida |
|---|---|
| 5 | +11 |
| 3 | +6 |

El programa demuestra la lectura de datos, asignación, multiplicación, evaluación de condiciones y saltos.







## 5. Organización de carpetas

```text
simpletron/
│
├── simpletron.c
├── README.md
│
├── src/
│   ├── compilador/
│   │   └── compilador.c
│   └── expresiones/
│       └── expresiones.c
│       └── expresiones.h
│
├── programas/
│   ├── simple/
│   └── sml/
│
├── pruebas/
│   ├── entradas/
│   └── expresiones/
│
├── evidencias/
│   ├── ejecuciones/
│   └── git-branch/
│
└── docs/
    └── diseno.md
```

## 6. Pruebas y evidencias

El repositorio contiene programas de prueba para las operaciones aritméticas, lectura y escritura, cadenas, saltos, errores del simulador y evaluación de expresiones.

También incluye programas Simple de ejemplo, sus archivos SML generados y pruebas de los seis operadores relacionales.

Las evidencias de ejecución se encuentran en `evidencias/ejecuciones/` y las evidencias de control de versiones en `evidencias/git-branch/`.

La documentación de diseño se encuentra en `docs/diseno.md`.
