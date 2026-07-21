# Arquitectura del Sistema

## Visión General

MiHobbyC sigue una arquitectura en capas con separación clara de responsabilidades:

```
┌─────────────────────────────────────────┐
│              Interfaz (main.c)          │  Menús, entrada de usuario, display
├─────────────────────────────────────────┤
│         Lógica de Negocio (hobby.c)     │  CRUD, validaciones, consultas
├─────────────────────────────────────────┤
│       Acceso a Datos (database.c)       │  Conexión BD, queries, log
├─────────────────────────────────────────┤
│         Persistencia (sqlite3.c)        │  SQLite3 embebido
└─────────────────────────────────────────┘
```

## Capas

### 1. Interfaz de Usuario (`main.c`)

**Responsabilidad:** Manejar la interacción con el usuario.

- Muestra menús en la consola
- Lee entrada del usuario (números, texto, confirmaciones)
- Formatea y muestra resultados
- Coordina el flujo entre menús

**Depende de:** `hobby.h`, `database.h`

**No contiene:** Lógica de negocio ni acceso a datos.

### 2. Lógica de Negocio (`hobby.c` / `hobby.h`)

**Responsabilidad:** Implementar las reglas del negocio.

- CRUD de hobbies (crear, actualizar, eliminar, listar)
- CRUD de categorías (crear, renombrar, eliminar, listar)
- Selección aleatoria
- Validaciones (límite de hobbies por categoría)
- Conversión Nº→ID

**Depende de:** `database.h`

**No contiene:** Código de UI ni manejo de archivos.

### 3. Acceso a Datos (`database.c` / `database.h`)

**Responsabilidad:** Gestionar la persistencia y el log.

- Abrir/cerrar conexión SQLite
- Ejecutar sentencias SQL
- Preparar statements parametrizados
- Escribir entradas de log con timestamp
- Construir rutas de archivos

**Depende de:** `sqlite3.h`, `<stdio.h>`, `<direct.h>`

**No contiene:** Lógica de negocio ni UI.

### 4. Persistencia (`sqlite3.c` / `sqlite3.h`)

**Responsabilidad:** Motor de base de datos SQLite3.

- Almacenamiento en disco
- Transacciones y WAL journal
- Consultas SQL optimizadas

**Es una dependencia externa** embebida en el proyecto.

## Flujo del Programa

### Inicialización

```
main()
  ├── db_abrir()          # Crear/abrir BD y log
  ├── hobby_init()        # Crear tabla, migrar si necesario
  └── while (opcion != 0) # Bucle principal del menú
```

### Sorteo Aleatorio

```
main() → caso 1
  ├── hobby_seleccionar_aleatorio()  # SELECT RANDOM() LIMIT 1
  └── mostrar_hobby_seleccionado()  # Display formateado
```

### CRUD de Categorías

```
menu_categorias()
  ├── [1] hobby_mostrar_categorias()
  ├── [2] hobby_crear_categoria()      # INSERT placeholder
  ├── [3] hobby_renombrar_categoria()  # UPDATE × 2
  ├── [4] hobby_eliminar_categoria()   # DELETE
  └── [5] menu_hobbies()
        ├── [1] hobby_listar_por_categoria()
        ├── [2] hobby_crear()
        ├── [3] hobby_nro_a_id() → hobby_actualizar()
        └── [4] hobby_nro_a_id() → hobby_eliminar()
```

## Modelo de Datos

### Tabla `hobbies`

| Columna | Tipo | Descripción |
|---------|------|-------------|
| `categoria` | TEXT | Nombre de la categoría (parte de PK) |
| `id` | INTEGER | ID secuencial dentro de la categoría (parte de PK) |
| `nombre` | TEXT | Nombre del hobby (UNIQUE) |
| `fecha_creacion` | TEXT | Fecha ISO 8601 de creación |

**Clave primaria compuesta:** `(categoria, id)`

### Registros Placeholder

Para que una categoría exista sin tener hobbies reales, se inserta un registro con:
- `nombre = "categoría [nuevo]"`
- Este registro se excluye de las consultas con `NOT LIKE '% [nuevo]'`

## Decisiones de Diseño

### ¿Por qué SQLite embebido?

- Sin instalación externa
- Persistencia local
- Rendimiento suficiente para uso individual
- Archivo único (`.db`)

### ¿Por qué Nº secuencial en lugar de IDs?

- Los IDs internos pueden tener huecos (tras eliminaciones)
- Los Nº son consecutivos y fáciles de recordar
- Experiencia de usuario más simple

### ¿Por qué placeholders para categorías vacías?

- SQLite no soporta categorías vacías (la PK requiere `id`)
- El placeholder permite que la categoría exista sin hobbies reales
- Se filtra en todas las consultas con `NOT LIKE`

### ¿Por qué WAL journal?

- Permite lecturas concurrentes mientras se escribe
- Mejor rendimiento en operaciones mixtas
- Sin pérdida de datos en crashes

## Dependencias

```
main.c
  ├── hobby.h (→ hobby.c)
  └── database.h (→ database.c)

hobby.c
  └── hobby.h
        └── database.h

database.c
  └── database.h
        └── sqlite3.h (→ sqlite3.c)
```

**No hay dependencias circulares.** La dependencia fluye en una sola dirección: `main → hobby → database → sqlite3`.
