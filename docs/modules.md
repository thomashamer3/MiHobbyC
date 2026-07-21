# Módulos

## Visión General

El proyecto está organizado en 4 módulos con responsabilidades claras:

```
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   main.c     │  │  hobby.c/h   │  │ database.c/h │  │  sqlite3.c/h │
│  Interfaz    │→ │  Negocio     │→ │  Datos       │→ │  Motor BD    │
└──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
```

---

## Módulo: Interfaz (`main.c`)

### Responsabilidad

Manejar toda la interacción con el usuario: menús, entrada de datos y visualización de resultados.

### Funciones Públicas

Ninguna (todo es `static`).

### Funciones Static

| Función | Descripción |
|---------|-------------|
| `limpiar_pantalla()` | Limpia la consola (cls/clear) |
| `mostrar_banner()` | Muestra el encabezado de la app |
| `mostrar_menu_principal()` | Muestra el menú principal |
| `mostrar_menu_categorias()` | Muestra el menú de categorías |
| `mostrar_menu_hobbies()` | Muestra el menú de hobbies de una categoría |
| `limpiar_buffer()` | Descarta caracteres pendientes en stdin |
| `pausar()` | Espera que el usuario presione Enter |
| `leer_texto()` | Lee una línea de texto con prompt |
| `resolver_categoria()` | Convierte Nº de categoría al nombre real |
| `mostrar_hobby_seleccionado()` | Muestra el resultado del sorteo |
| `mostrar_mensaje_exito()` | Muestra mensaje [OK] |
| `mostrar_mensaje_error()` | Muestra mensaje [ERROR] |
| `mostrar_mensaje_info()` | Muestra mensaje [INFO] |
| `menu_hobbies()` | Bucle del submenú de hobbies |
| `menu_categorias()` | Bucle del submenú de categorías |

### Flujo

```
main()
  ├── db_abrir()
  ├── hobby_init()
  └── do {
        mostrar_banner()
        mostrar_menu_principal()
        switch (opcion)
          ├── [1] hobby_seleccionar_aleatorio() → mostrar_hobby_seleccionado()
          ├── [2] menu_categorias()
          │     ├── [1] hobby_mostrar_categorias()
          │     ├── [2] hobby_crear_categoria()
          │     ├── [3] resolver_categoria() → hobby_renombrar_categoria()
          │     ├── [4] resolver_categoria() → hobby_eliminar_categoria()
          │     └── [5] resolver_categoria() → menu_hobbies()
          │           ├── [1] hobby_listar_por_categoria()
          │           ├── [2] hobby_crear()
          │           ├── [3] hobby_nro_a_id() → hobby_actualizar()
          │           └── [4] hobby_nro_a_id() → hobby_eliminar()
          └── [0] Salir
      } while (opcion != 0)
```

---

## Módulo: Negocio (`hobby.c` / `hobby.h`)

### Responsabilidad

Implementar la lógica del negocio: CRUD de hobbies y categorías, selección aleatoria, validaciones.

### Macros

| Macro | Valor | Descripción |
|-------|-------|-------------|
| `MAX_HOBBY_LEN` | 100 | Longitud máxima del nombre de hobby |
| `MAX_CATEGORIA_LEN` | 50 | Longitud máxima del nombre de categoría |
| `MAX_POR_CATEGORIA` | 5 | Máximo de hobbies por categoría |
| `TABLE_HOBBIES` | "hobbies" | Nombre de la tabla |
| `COL_FECHA` | "fecha_creacion" | Nombre de la columna de fecha |
| `HOBBY_PLACEHOLDER` | " [nuevo]" | Sufijo de registros placeholder |
| `MIGRATION_EPOCH` | "2020-01-01" | Fecha base para migración |

### Funciones Públicas

| Función | Retorno | Descripción |
|---------|---------|-------------|
| `hobby_init()` | int | Inicializa la tabla con migraciones |
| `hobby_crear()` | int | Crea un hobby nuevo |
| `hobby_actualizar()` | int | Actualiza nombre/categoría de un hobby |
| `hobby_eliminar()` | int | Elimina un hobby por ID y categoría |
| `hobby_seleccionar_aleatorio()` | int | Selecciona un hobby al azar |
| `hobby_contar_por_categoria()` | int | Cuenta hobbies reales de una categoría |
| `hobby_mostrar_categorias()` | void | Lista categorías con conteo |
| `hobby_categoria_existe()` | int | Verifica si una categoría existe |
| `hobby_listar_por_categoria()` | int | Lista hobbies de una categoría |
| `hobby_nro_a_id()` | int | Convierte Nº secuencial a ID real |
| `hobby_crear_categoria()` | int | Crea categoría con placeholder |
| `hobby_renombrar_categoria()` | int | Renombra categoría y placeholder |
| `hobby_eliminar_categoria()` | int | Elimina categoría y todos sus hobbies |

### Funciones Static

| Función | Descripción |
|---------|-------------|
| `hobby_siguiente_id()` | Calcula el siguiente ID disponible |

### Validaciones

1. **Límite de hobbies:** `hobby_contar_por_categoria() >= MAX_POR_CATEGORIA`
2. **Nombre duplicado:** SQLite UNIQUE constraint
3. **Parámetros NULL:** Todas las funciones verifican punteros
4. **Nº inválido:** `hobby_nro_a_id()` retorna -1 si el Nº no existe

---

## Módulo: Datos (`database.c` / `database.h`)

### Responsabilidad

Gestionar la conexión a SQLite, ejecutar queries, preparar statements y escribir el log.

### Struct

```c
typedef struct {
    sqlite3 *db;   // Conexión SQLite
    FILE *log;     // Archivo de log
} Database;
```

### Macros

| Macro | Valor | Descripción |
|-------|-------|-------------|
| `DB_SUBCARPETA` | "MiHobbyC\\data" | Subcarpeta de datos |
| `DB_ARCHIVO` | "hobbies.db" | Nombre del archivo BD |
| `DB_LOG` | "mihobbyc.log" | Nombre del archivo de log |
| `PATH_BUF_SIZE` | 512 | Tamaño de buffers de ruta |
| `MSG_BUF_SIZE` | 768 | Tamaño de buffers de mensaje |
| `FECHA_BUF_SIZE` | 64 | Tamaño de buffers de fecha |
| `LOG_DATE_FORMAT` | "%Y-%m-%d %H:%M:%S" | Formato de fecha para log |

### Funciones Públicas

| Función | Retorno | Descripción |
|---------|---------|-------------|
| `db_abrir()` | int | Abre BD y log, configura WAL |
| `db_cerrar()` | void | Cierra BD y log |
| `db_ejecutar()` | int | Ejecuta SQL sin retorno |
| `db_preparar()` | sqlite3_stmt* | Prepara statement parametrizado |
| `db_log()` | void | Escribe entrada en el log |

### Funciones Static

| Función | Descripción |
|---------|-------------|
| `obtener_fecha_hora()` | Obtiene fecha/hora actual formateada |
| `crear_directorio()` | Crea directorios recursivamente |
| `construir_ruta_base()` | Construye la ruta base de archivos |

### Flujo de Apertura

```
db_abrir()
  ├── construir_ruta_base()    # LOCALAPPDATA\MiHobbyC\data
  ├── crear_directorio()       # mkdir recursivo
  ├── fopen(log, "a")          # Abrir log
  ├── obtener_fecha_hora()     # Timestamp
  ├── fprintf(log, inicio)     # "Sistema iniciado"
  ├── sqlite3_open()           # Abrir/crear BD
  ├── sqlite3_exec(WAL)        # Configurar journal
  ├── sqlite3_exec(FK)         # Habilitar foreign keys
  └── db_log(apertura)         # "Base de datos abierta"
```

---

## Módulo: Persistencia (`sqlite3.c` / `sqlite3.h`)

### Responsabilidad

Motor de base de datos SQLite3 (dependencia externa embebida).

### Características

- **Amalgamation:** Un solo archivo `.c` con todo el motor
- **WAL Journal:** Escritura concurrente
- **UTF-8:** Codificación por defecto
- **Sin dependencias externas:** Todo autocontenido

### Uso en el Proyecto

Se usa exclusivamente a través de `database.c`:
- `sqlite3_open()` / `sqlite3_close()`
- `sqlite3_exec()`
- `sqlite3_prepare_v2()` / `sqlite3_step()` / `sqlite3_finalize()`
- `sqlite3_bind_text()` / `sqlite3_bind_int()`
- `sqlite3_column_text()` / `sqlite3_column_int()`
- `sqlite3_changes()`
- `sqlite3_errmsg()`

---

## Dependencias entre Módulos

```
main.c
  ├── #include "database.h"  (para Database, db_abrir, db_cerrar, db_log)
  └── #include "hobby.h"     (para todas las funciones de hobby)

hobby.c
  └── #include "hobby.h"
        └── #include "database.h"

database.c
  └── #include "database.h"
        └── #include "sqlite3.h"
```

**Regla:** Un módulo solo puede depender de módulos "debajo" en la jerarquía. Nunca hacia arriba.
