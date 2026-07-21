# MiHobbyC

**Selector de hobbies aleatorios con persistencia en SQLite**

Descubre tu actividad del día con un sorteo aleatorio. Administra categorías y hobbies con operaciones CRUD completas, todo almacenado en una base de datos SQLite local.

---

## Características

- **Sorteo aleatorio** — Selecciona un hobby al azar de la base de datos
- **Gestión de categorías** — Crear, renombrar, eliminar y listar categorías
- **CRUD de hobbies** — Agregar, editar, eliminar y listar hobbies por categoría
- **Nº secuencial** — Los hobbies se identifican por Nº (1-5) en lugar de IDs internos
- **Persistencia SQLite** — Base de datos local con WAL journal
- **Log de auditoría** — Registro de todas las operaciones CRUD en archivo de log
- **Migraciones automáticas** — La BD se actualiza sin perder datos
- **Sin dependencias externas** — SQLite3 embebido en el proyecto

## Capturas

```
  +=========================================+
  |        *  MI HOBBY C - Selector  *      |
  |      Descubre tu actividad del dia      |
  +=========================================+

  +-----------------------------------------+
  |           MENU PRINCIPAL                |
  +-----------------------------------------+
  |  [1]  Sortear hobby aleatorio           |
  |  [2]  Administrar categorias            |
  |  [0]  Salir                             |
  +-----------------------------------------+

  +=========================================+
  |       *  TU HOBBY DE HOY ES  *         |
  |   |  Anime                             |
  |   |  >> Lectura                        |
  +=========================================+
```

## Tecnologías

| Tecnología | Uso |
|-----------|-----|
| C (C23) | Lenguaje principal |
| SQLite3 | Base de datos embebida |
| Code::Blocks | IDE de desarrollo |
| GCC (MinGW64) | Compilador |

## Arquitectura

El proyecto sigue una arquitectura en capas:

```
┌─────────────────────────────────┐
│           main.c                │  ← Interfaz de usuario (menús)
├─────────────────────────────────┤
│          hobby.c / hobby.h      │  ← Lógica de negocio (CRUD)
├─────────────────────────────────┤
│       database.c / database.h   │  ← Capa de acceso a datos
├─────────────────────────────────┤
│         sqlite3.c / sqlite3.h   │  ← SQLite3 embebido
└─────────────────────────────────┘
```

Ver [docs/architecture.md](docs/architecture.md) para detalles completos.

## Estructura

```
MiHobbyC/
├── main.c              # Punto de entrada y menús
├── hobby.c             # Lógica de negocio (CRUD hobbies/categorías)
├── hobby.h             # Interfaz del módulo de hobbies
├── database.c          # Acceso a datos SQLite
├── database.h          # Interfaz del módulo de BD
├── sqlite3.c           # SQLite3 amalgamation
├── sqlite3.h           # Header de SQLite3
├── MiHobbyC.cbp        # Proyecto Code::Blocks
├── recurso.rc          # Recursos Windows (ícono)
├── Doxyfile            # Configuración Doxygen
├── docs/               # Documentación técnica
│   ├── architecture.md
│   ├── database.md
│   ├── modules.md
│   ├── build.md
│   └── api-reference.md
├── bin/                # Binarios compilados
└── obj/                # Objetos intermedios
```

## Requisitos

- **Compilador:** GCC con soporte C23 (MinGW64 o similar)
- **IDE (opcional):** Code::Blocks con compilador MSYS2 MinGW64
- **SO:** Windows (usa `LOCALAPPDATA` para almacenamiento)
- **No requiere:** SQLite3 instalado (está embebido)

## Compilación

### Opción 1: Code::Blocks

1. Abrir `MiHobbyC.cbp`
2. Seleccionar target (Debug o Release)
3. Build → Build (F9)

### Opción 2: GCC directo

```bash
# Debug
gcc -std=c23 -Wall -Wextra -g -o mihobby.exe main.c database.c hobby.c sqlite3.c

# Release
gcc -std=c23 -O3 -o mihobby.exe main.c database.c hobby.c sqlite3.c -static-libgcc
```

### Opción 3: PowerShell (Windows)

```powershell
gcc -std=c23 -Wall -Wextra -o mihobby.exe main.c database.c hobby.c sqlite3.c
.\mihobby.exe
```

Ver [docs/build.md](docs/build.md) para opciones avanzadas.

## Uso

```bash
./mihobby.exe
```

### Menú Principal

| Opción | Descripción |
|--------|-------------|
| `[1]` Sortear hobby aleatorio | Selecciona un hobby al azar y lo muestra |
| `[2]` Administrar categorías | Entra al submenú de categorías |
| `[0]` Salir | Cierra la aplicación |

### Administrar Categorías

| Opción | Descripción |
|--------|-------------|
| `[1]` Ver categorías | Lista todas las categorías con conteo |
| `[2]` Agregar categoría | Crea una categoría nueva |
| `[3]` Editar categoría | Renombra una categoría existente |
| `[4]` Eliminar categoría | Elimina una categoría y sus hobbies |
| `[5]` Administrar hobbies | Entra al submenú de hobbies de una categoría |

### Administrar Hobbies

| Opción | Descripción |
|--------|-------------|
| `[1]` Ver hobbies | Lista los hobbies por Nº |
| `[2]` Agregar hobby | Agrega un hobby a la categoría |
| `[3]` Editar hobby | Modifica el nombre de un hobby |
| `[4]` Eliminar hobby | Elimina un hobby específico |

### Entrada por Nº

Los usuarios ingresan **Nº secuenciales** (1-5) en lugar de IDs internos. El sistema mapea automáticamente el Nº al ID real de la base de datos.

## Base de Datos

- **Ubicación:** `%LOCALAPPDATA%\MiHobbyC\data\hobbies.db`
- **Log:** `%LOCALAPPDATA%\MiHobbyC\data\mihobbyc.log`
- **Motor:** SQLite3 con WAL journal
- **Tabla principal:** `hobbies` (categoria, id, nombre, fecha_creacion)

Ver [docs/database.md](docs/database.md) para el esquema completo.

## Configuración

Las constantes del sistema están definidas en los headers:

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `MAX_HOBBY_LEN` | 100 | Longitud máxima del nombre de hobby |
| `MAX_CATEGORIA_LEN` | 50 | Longitud máxima del nombre de categoría |
| `MAX_POR_CATEGORIA` | 5 | Máximo de hobbies por categoría |
| `PATH_BUF_SIZE` | 512 | Tamaño de buffers de ruta |
| `MSG_BUF_SIZE` | 768 | Tamaño de buffers de mensaje |

## Roadmap

- [x] CRUD de hobbies y categorías
- [x] Selección aleatoria
- [x] Persistencia SQLite
- [x] Log de auditoría
- [x] Migraciones automáticas
- [x] Documentación Doxygen (100% cobertura)
- [ ] Filtros por categoría en sorteo
- [ ] Estadísticas de uso
- [ ] Exportar/importar datos
- [ ] Soporte multiplataforma (Linux/macOS)

## Documentación

| Documento | Descripción |
|-----------|-------------|
| [README.md](README.md) | Este archivo |
| [docs/architecture.md](docs/architecture.md) | Arquitectura del sistema |
| [docs/database.md](docs/database.md) | Esquema de la base de datos |
| [docs/modules.md](docs/modules.md) | Descripción de módulos |
| [docs/build.md](docs/build.md) | Guía de compilación |
| [docs/api-reference.md](docs/api-reference.md) | Referencia de la API |
| [CHANGELOG.md](CHANGELOG.md) | Historial de cambios |
| Doxyfile | Configuración para generar documentación HTML |

Para generar la documentación Doxygen:

```bash
doxygen Doxyfile
# La salida estará en docs/html/
```

## Licencia

Proyecto educativo. Usa SQLite3 en dominio público.

## Autor

**MiHobbyC** — Proyecto de aprendizaje en C con SQLite

## Contribuciones

1. Forjar el proyecto
2. Crear una rama (`git checkout -b feature/nueva-funcionalidad`)
3. Hacer commit (`git commit -m 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abrir un Pull Request

## Preguntas Frecuentes

### ¿Dónde se almacenan los datos?

En `%LOCALAPPDATA%\MiHobbyC\data\`. La carpeta se crea automáticamente.

### ¿Puedo usar el programa en Linux?

Actualmente usa `_mkdir` de Windows. Para Linux se necesitaría reemplazar por `mkdir()` de POSIX.

### ¿Cómo elimino todos los datos?

Eliminar el archivo `hobbies.db` en la carpeta de datos. Se recreará al iniciar.

### ¿Por qué los hobbies se muestran por Nº y no por ID?

Por experiencia de usuario. Los Nº son secuenciales (1-5) y fáciles de recordar, mientras que los IDs internos pueden tener huecos.
