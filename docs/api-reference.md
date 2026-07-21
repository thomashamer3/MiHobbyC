# Referencia de la API

## Módulo: database.h

### Struct Database

```c
typedef struct {
    sqlite3 *db;   // Conexión SQLite
    FILE *log;     // Archivo de log
} Database;
```

Estructura principal que encapsula la conexión a la base de datos y el archivo de log.

---

### db_abrir

```c
int db_abrir(Database *db);
```

Abre o crea la base de datos y el archivo de log.

| Parámetro | Descripción |
|-----------|-------------|
| `db` | Puntero a la estructura Database a inicializar |

**Retorna:** 1 si éxito, 0 si error.

**Efectos secundarios:**
- Crea directorios en LOCALAPPDATA
- Abre archivo de log en modo append
- Configura WAL journal y foreign keys
- Escribe entrada de log "Sistema iniciado"

---

### db_cerrar

```c
void db_cerrar(Database *db);
```

Cierra la base de datos y el archivo de log.

**Efectos secundarios:**
- Escribe entrada de log "Sistema detenido"
- Cierra conexión SQLite y FILE* del log
- Pone punteros a NULL

---

### db_ejecutar

```c
int db_ejecutar(Database *db, const char *sql);
```

Ejecuta una sentencia SQL sin retornar resultados.

| Parámetro | Descripción |
|-----------|-------------|
| `db` | Puntero a Database abierto |
| `sql` | Sentencia SQL (INSERT, UPDATE, DELETE, DDL) |

**Retorna:** 1 si éxito, 0 si error.

**Nota:** Registra errores en el log y stderr.

---

### db_preparar

```c
sqlite3_stmt *db_preparar(Database *db, const char *sql);
```

Prepara una sentencia SQL para ejecución parametrizada.

| Parámetro | Descripción |
|-----------|-------------|
| `db` | Puntero a Database abierto |
| `sql` | Sentencia SQL con parámetros tipo `?` |

**Retorna:** Puntero al statement, o NULL si error.

**Nota:** El llamador debe llamar `sqlite3_finalize()` al statement.

---

### db_log

```c
void db_log(Database *db, const char *mensaje);
```

Escribe una entrada en el archivo de log con timestamp.

| Parámetro | Descripción |
|-----------|-------------|
| `db` | Puntero a Database con log abierto |
| `mensaje` | Texto de la entrada |

**Formato:** `[YYYY-MM-DD HH:MM:SS] mensaje`

---

## Módulo: hobby.h

### hobby_init

```c
int hobby_init(Database *db);
```

Inicializa la tabla de hobbies con migraciones automáticas.

**Retorna:** 1 si éxito, 0 si error.

**Efectos secundarios:**
- Crea tabla `hobbies` si no existe
- Agrega columna `fecha_creacion` si falta
- Rellena fechas para registros existentes

---

### hobby_crear

```c
int hobby_crear(Database *db, const char *nombre, const char *categoria);
```

Crea un nuevo hobby en la base de datos.

| Parámetro | Descripción |
|-----------|-------------|
| `nombre` | Nombre del hobby (máx. 100 chars) |
| `categoria` | Nombre de la categoría destino |

**Retorna:** ID del hobby creado (>= 1), o -1 si error.

**Validaciones:**
- Verifica límite de 5 hobbies por categoría
- Nombre único en toda la BD (UNIQUE constraint)

---

### hobby_actualizar

```c
int hobby_actualizar(Database *db, int id, const char *categoria_actual,
                     const char *nombre, const char *categoria_nueva);
```

Actualiza el nombre y/o categoría de un hobby existente.

| Parámetro | Descripción |
|-----------|-------------|
| `id` | ID del hobby a actualizar |
| `categoria_actual` | Categoría actual del hobby |
| `nombre` | Nuevo nombre (NULL para mantener) |
| `categoria_nueva` | Nueva categoría (NULL para mantener) |

**Retorna:** 1 si modificó, 0 si no encontrado o error.

---

### hobby_eliminar

```c
int hobby_eliminar(Database *db, int id, const char *categoria);
```

Elimina un hobby por ID y categoría.

**Retorna:** 1 si eliminó, 0 si no encontrado o error.

---

### hobby_seleccionar_aleatorio

```c
int hobby_seleccionar_aleatorio(Database *db, char *buffer_cat, int tam_cat,
                                char *buffer_hobby, int tam_hobby);
```

Selecciona un hobby aleatorio de la base de datos.

| Parámetro | Descripción |
|-----------|-------------|
| `buffer_cat` | Buffer para la categoría seleccionada |
| `tam_cat` | Tamaño del buffer de categoría |
| `buffer_hobby` | Buffer para el nombre del hobby |
| `tam_hobby` | Tamaño del buffer de hobby |

**Retorna:** 1 si seleccionó, 0 si tabla vacía o error.

---

### hobby_contar_por_categoria

```c
int hobby_contar_por_categoria(Database *db, const char *categoria);
```

Cuenta los hobbies reales (excluyendo placeholders) de una categoría.

**Retorna:** Cantidad (>= 0), o -1 si error.

---

### hobby_mostrar_categorias

```c
void hobby_mostrar_categorias(Database *db);
```

Muestra en pantalla las categorías disponibles con su conteo.

**Formato de salida:**
```
+-----------------------------------------+
|         CATEGORIAS DISPONIBLES          |
+-----------------------------------------+
|  [1] Anime              (3/5)          |
|  [2] Deportes           (2/5)          |
+-----------------------------------------+
```

---

### hobby_categoria_existe

```c
int hobby_categoria_existe(Database *db, const char *categoria);
```

Verifica si una categoría existe en la base de datos.

**Retorna:** 1 si existe, 0 si no.

---

### hobby_listar_por_categoria

```c
int hobby_listar_por_categoria(Database *db, const char *categoria);
```

Lista los hobbies de una categoría específica.

**Retorna:** Cantidad de hobbies listados, o -1 si error.

**Formato de salida:**
```
No    Hobby
---   ---
1     Lectura
2     Drawing
3     Gaming

Total: 3/5 hobbies
```

---

### hobby_nro_a_id

```c
int hobby_nro_a_id(Database *db, const char *categoria, int nro);
```

Convierte un Nº secuencial (1-5) al ID real de un hobby.

| Parámetro | Descripción |
|-----------|-------------|
| `categoria` | Nombre de la categoría |
| `nro` | Nº secuencial (1 a MAX_POR_CATEGORIA) |

**Retorna:** ID real (>= 1), o -1 si no encontrado.

---

### hobby_crear_categoria

```c
int hobby_crear_categoria(Database *db, const char *categoria);
```

Crea una categoría insertando un registro placeholder.

**Retorna:** 1 si éxito, 0 si error.

**Efectos secundarios:**
- Inserta registro con nombre "categoría [nuevo]"

---

### hobby_renombrar_categoria

```c
int hobby_renombrar_categoria(Database *db, const char *vieja, const char *nueva);
```

Renombra una categoría y actualiza su placeholder.

**Retorna:** 1 si modificó, 0 si no encontrado o error.

**Efectos secundarios:**
- Actualiza nombre del placeholder
- Actualiza columna `categoria` de todos los registros

---

### hobby_eliminar_categoria

```c
int hobby_eliminar_categoria(Database *db, const char *categoria);
```

Elimina una categoría y todos sus hobbies.

**Retorna:** 1 si eliminó, 0 si no encontrado o error.

**Efectos secundarios:**
- Elimina todos los registros de la categoría (incluyendo placeholders)

---

## Constantes

### hobby.h

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `MAX_HOBBY_LEN` | 100 | Longitud máxima del nombre de hobby |
| `MAX_CATEGORIA_LEN` | 50 | Longitud máxima del nombre de categoría |
| `MAX_POR_CATEGORIA` | 5 | Máximo de hobbies por categoría |
| `TABLE_HOBBIES` | "hobbies" | Nombre de la tabla |
| `COL_FECHA` | "fecha_creacion" | Nombre de la columna de fecha |
| `HOBBY_PLACEHOLDER` | " [nuevo]" | Sufijo de registros placeholder |
| `MIGRATION_EPOCH` | "2020-01-01" | Fecha base para migración |

### database.h

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `DB_SUBCARPETA` | "MiHobbyC\\data" | Subcarpeta de datos |
| `DB_ARCHIVO` | "hobbies.db" | Nombre del archivo BD |
| `DB_LOG` | "mihobbyc.log" | Nombre del archivo de log |
| `PATH_BUF_SIZE` | 512 | Tamaño de buffers de ruta |
| `MSG_BUF_SIZE` | 768 | Tamaño de buffers de mensaje |
| `FECHA_BUF_SIZE` | 64 | Tamaño de buffers de fecha |
| `LOG_DATE_FORMAT` | "%Y-%m-%d %H:%M:%S" | Formato de fecha para log |

---

## Ejemplo de Uso

### Crear una categoría y agregar hobbies

```c
Database db;
db_abrir(&db);
hobby_init(&db);

// Crear categoría
hobby_crear_categoria(&db, "Anime");

// Agregar hobbies
hobby_crear(&db, "Lectura", "Anime");
hobby_crear(&db, "Drawing", "Anime");
hobby_crear(&db, "Gaming", "Anime");

// Seleccionar uno al azar
char cat[50], hobby[100];
if (hobby_seleccionar_aleatorio(&db, cat, sizeof(cat), hobby, sizeof(hobby))) {
    printf("Tu hobby de hoy: %s en %s\n", hobby, cat);
}

db_cerrar(&db);
```

### Listar y editar

```c
// Listar hobbies de una categoría
hobby_listar_por_categoria(&db, "Anime");

// Convertir Nº 2 a ID real
int id = hobby_nro_a_id(&db, "Anime", 2);
if (id > 0) {
    // Editar el hobby
    hobby_actualizar(&db, id, "Anime", "Nuevo Nombre", NULL);
}
```
