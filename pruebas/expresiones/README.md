# Pruebas del evaluador de expresiones

## Compilar

Desde la carpeta raíz del repositorio:

```bash
gcc pruebas/expresiones/prueba_evaluador.c src/expresiones/expresiones.c -DEXPRESIONES_LIB -o pruebas/expresiones/prueba_evaluador.exe -lm
```

## Ejecutar

En Git Bash:

```bash
./pruebas/expresiones/prueba_evaluador.exe
```

En Windows CMD:

```bat
pruebas\expresiones\prueba_evaluador.exe
```

El programa termina con código 0 cuando todas las pruebas pasan y con código 1 si encuentra algún fallo.

## Casos positivos

| Expresión | Resultado esperado | Qué comprueba |
|---|---:|---|
| `2 + 3 * 4` | 14 | Precedencia |
| `(2 + 3) * 4` | 20 | Paréntesis |
| `2 ^ 3 ^ 2` | 512 | Asociatividad derecha de potencia |
| `15 % 4` | 3 | Módulo |
| `1.5 + 2.5 * 2` | 6.5 | Números decimales |
| `{2 + [3 * (4 - 1)]}` | 11 | Llaves, corchetes y paréntesis |
| `-2 ^ 2` | -4 | Signo unario y potencia |

## Casos de error

- Agrupador sin cerrar: `(2 + 3`.
- Operadores consecutivos: `2 + * 3`.
- Agrupadores incorrectos: `(2 + 3)]`.
- División entre cero: `8 / 0`.

La salida final debe indicar 11 pruebas correctas y 0 fallidas.

