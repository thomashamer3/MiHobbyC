# Changelog

Todos los cambios notables en MiHobbyC documentados en este archivo.

El formato se basa en [Keep a Changelog](https://keepachangelog.com/es/1.1.0/).

## [2.0.0] - 2026-07-21

### Added

- **Selección aleatoria** — Opción de sortear un hobby al azar desde el menú principal
- **Nº secuencial** — Los hobbies se muestran por Nº (1-5) en lugar de IDs internos
- **Conversión Nº→ID** — Función `hobby_nro_a_id()` para mapear Nº visible al ID real
- **Fecha de creación** — Columna `fecha_creacion` con migración automática
- **Log de auditoría** — Registro de todas las operaciones CRUD en `mihobbyc.log`
- **Migraciones automáticas** — Verificación de esquema con `PRAGMA table_info`
- **WAL journal** — Modo de escritura concurrente para mejor rendimiento
- **Foreign keys** — Habilitación de claves foráneas en SQLite
- **Documentación Doxygen** — 100% de cobertura en funciones, structs y macros
- **Documentación técnica** — README, CHANGELOG, architecture, database, modules, build, api-reference

### Changed

- **Máximo de hobbies por categoría** — Ahora es 5 (constante `MAX_POR_CATEGORIA`)
- **Menú de categorías** — Opción `[5]` para administrar hobbies de una categoría
- **Entrada de usuarios** — Ahora se usan Nº secuenciales en lugar de IDs directos
- **Categorías sin hobbies** — Se muestran solo categorías con hobbies reales (excluye placeholders)
- **Query de categorías** — `GROUP BY` con `COUNT(*)` en una sola query (eliminada N+1)
- **Constantes centralizadas** — Todos los valores hardcodeados reemplazados por macros

### Fixed

- **Bug en resolver_categoria** — Ahora filtra placeholders para coincidir con el numbering del display
- **Bug en renombrar categoría** — Ahora también renombra el registro placeholder `[nuevo]`
- **Memoria** — Eliminada variable `inicializado` y array estático `CATEGORIAS[]`
- **Includes** — Eliminados includes no utilizados (`<time.h>`, `<stdlib.h>`, `<sys/stat.h>`)

### Removed

- **`hobby_listar()`** — Función no utilizada (reemplazada por `hobby_listar_por_categoria()`)
- **`hobby_total()`** — Función no utilizada
- **`struct Hobby`** — Estructura no utilizada
- **`MAX_CATEGORIA_LEN` hardcodeado** — Movido a macro en `hobby.h`
- **`CATEGORIAS[]`** — Array estático eliminado (las categorías son 100% dinámicas desde la BD)

## [1.0.0] - 2026-07-20

### Added

- Versión inicial del proyecto
- CRUD básico de hobbies y categorías
- Persistencia en SQLite3
- Interfaz de menú en consola
- Proyecto Code::Blocks
