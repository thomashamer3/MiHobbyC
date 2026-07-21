# Guía de Compilación

## Requisitos

| Requisito | Versión Mínima | Versión Recomendada |
|-----------|---------------|---------------------|
| GCC | 13+ (C23) | MinGW64 latest |
| Code::Blocks | 20.03 | 20.03+ |
| SO | Windows 10 | Windows 11 |

**Nota:** El proyecto usa el estándar **C23** (`-std=c23`). Asegúrese de que su compilador lo soporte.

---

## Opción 1: Code::Blocks

### Configuración del Compilador

El proyecto está configurado para usar **MSYS2 MinGW64**:

1. Abrir Code::Blocks
2. Settings → Compiler
3. Seleccionar "GNU GCC Compiler for MinGW64" o "gcc-msys2-mingw64"

### Compilar

1. Abrir `MiHobbyC.cbp`
2. Seleccionar target:
   - **Debug** — Con símbolos de depuración
   - **Release** — Optimizado (-O3)
3. Build → Build (F9)
4. El ejecutable se genera en `bin/Debug/` o `bin/Release/`

### Configuración del Proyecto

El `.cbp` incluye:

```xml
<Compiler>
    <Add option="-fexpensive-optimizations" />
    <Add option="-std=c23" />
    <Add option="-m64" />
</Compiler>
<Linker>
    <Add option="-O3" />
    <Add option="-static-libgcc" />
    <Add option="-m64" />
</Linker>
```

---

## Opción 2: GCC Directo

### Debug (con símbolos)

```bash
gcc -std=c23 -Wall -Wextra -g -o mihobby.exe main.c database.c hobby.c sqlite3.c
```

### Release (optimizado)

```bash
gcc -std=c23 -O3 -Wall -Wextra -o mihobby.exe main.c database.c hobby.c sqlite3.c -static-libgcc
```

### Solo Compilar (sin enlazar)

```bash
gcc -std=c23 -Wall -Wextra -c main.c
gcc -std=c23 -Wall -Wextra -c database.c
gcc -std=c23 -Wall -Wextra -c hobby.c
gcc -std=c23 -Wall -Wextra -c sqlite3.c
```

---

## Opción 3: PowerShell Script

```powershell
# Compilar
gcc -std=c23 -Wall -Wextra -o mihobby.exe main.c database.c hobby.c sqlite3.c

# Ejecutar
.\mihobby.exe

# Compilar y ejecutar
gcc -std=c23 -Wall -Wextra -o mihobby.exe main.c database.c hobby.c sqlite3.c; .\mihobby.exe
```

---

## Flags de Compilación

| Flag | Descripción |
|------|-------------|
| `-std=c23` | Estándar C23 |
| `-Wall` | Todas las warnings comunes |
| `-Wextra` | Warnings adicionales |
| `-g` | Símbolos de depuración |
| `-O3` | Optimización máxima |
| `-static-libgcc` | Librería GCC estática (para distribución) |
| `-m64` | Compilación a 64 bits |
| `-fexpensive-optimizations` | Optimizaciones costosas |
| `-lm` | Librería matemática (si se usa `math.h`) |

---

## Estructura de Directorios

```
MiHobbyC/
├── bin/
│   ├── Debug/
│   │   └── MiHobbyC.exe
│   └── Release/
│       └── MiHobbyC.exe
├── obj/
│   ├── Debug/
│   │   └── *.o
│   └── Release/
│       └── *.o
└── *.c, *.h
```

---

## Errores Comunes

### `error: unknown type name 'bool'`

**Causa:** Compilador no soporta C23 o falta `#include <stdbool.h>`

**Solución:** Usar `-std=c23` o agregar `#include <stdbool.h>`

### `error: 'snprintf' undeclared`

**Causa:** Falta `#include <stdio.h>`

**Solución:** Ya está incluido en `database.c`. Verificar el include.

### `warning: implicit declaration of function '_mkdir'`

**Causa:** Falta `#include <direct.h>` (solo Windows)

**Solución:** Ya está incluido en `database.c`. Este warning no aplica en Unix.

### `error: sqlite3.h: No such file`

**Causa:** SQLite3 no está en el path

**Solución:** El archivo `sqlite3.c` y `sqlite3.h` deben estar en la misma carpeta que el resto.

---

## Distribución

Para distribuir el ejecutable:

```bash
# Compilar estático
gcc -std=c23 -O3 -static-libgcc -o MiHobbyC.exe main.c database.c hobby.c sqlite3.c

# El ejecutable es autocontenido (no necesita DLLs)
```

**Nota:** La base de datos se crea automáticamente en `%LOCALAPPDATA%\MiHobbyC\data\` en la primera ejecución.

---

## Generar Documentación Doxygen

```bash
# Instalar Doxygen (si no está)
# Windows: descargar de doxygen.nl
# Linux: sudo apt install doxygen graphviz

# Generar documentación
doxygen Doxyfile

# La salida estará en docs/html/index.html
```
