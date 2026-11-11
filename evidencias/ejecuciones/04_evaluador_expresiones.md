# Evidencia del evaluador de expresiones

Este archivo indica cómo generar la evidencia de ejecución. La captura debe obtenerse al ejecutar el programa en el equipo local; este documento no sustituye una captura real.

## 1. Compilar y ejecutar las pruebas automatizadas

Desde la raíz del repositorio:

```bash
gcc pruebas/expresiones/prueba_evaluador.c src/expresiones/expresiones.c -DEXPRESIONES_LIB -o pruebas/expresiones/prueba_evaluador.exe -lm
./pruebas/expresiones/prueba_evaluador.exe
```

## 2. Resultado que debe aparecer

Al final de la ejecución:

```text
Pruebas correctas: 11
Pruebas fallidas:  0
```

Durante la ejecución también deben mostrarse las expresiones en notación postfija y los resultados esperados.

## 3. Captura solicitada

Tomar una captura de pantalla donde se vea:

- El comando de compilación o ejecución.
- Varias pruebas positivas y sus resultados.
- Los casos de error detectados.
- El resumen de 11 pruebas correctas y 0 fallidas.

Guardar la captura como:

`evidencias/ejecuciones/04_evaluador_expresiones.png`

No agregar una captura hasta ejecutar realmente las pruebas.
