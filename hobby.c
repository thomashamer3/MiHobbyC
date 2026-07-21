/**
 * @file hobby.c
 * @brief Implementación del módulo de gestión de hobbies y categorías.
 * @details Contiene la lógica de negocio para operaciones CRUD sobre
 *          hobbies, selección aleatoria, gestión de categorías (crear,
 *          renombrar, eliminar), conversión de Nº a ID, y consultas
 *          aggregadas. La tabla de hobbies usa un esquema con categoría,
 *          id secuencial, nombre y fecha de creación.
 *
 * @author MiHobbyC
 * @version 2.0
 * @date 2026
 *
 * @see hobby.h para la interfaz pública y constantes.
 * @see database.c para la capa de persistencia subyacente.
 */

#include "hobby.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Inicializa la tabla de hobbies con migraciones.
 * @details Ejecuta CREATE TABLE IF NOT EXISTS y verifica la existencia
 *          de la columna fecha_creacion mediante PRAGMA table_info.
 *          Si falta, la agrega y rellena con fechas calculadas desde
 *          MIGRATION_EPOCH.
 *
 * @param[in,out] db Puntero a la estructura Database.
 *
 * @return 1 si la inicialización fue exitosa, 0 si hubo error.
 *
 * @pre db debe ser válido y haber sido abierto con db_abrir().
 * @post La tabla TABLE_HOBBIES existe con todas las columnas necesarias.
 *
 * @see TABLE_HOBBIES
 * @see COL_FECHA
 * @see MIGRATION_EPOCH
 */
int hobby_init(Database *db)
{
    if (!db) return 0;

    const char *sql =
        "CREATE TABLE IF NOT EXISTS " TABLE_HOBBIES " ("
        "  categoria TEXT NOT NULL,"
        "  id INTEGER NOT NULL,"
        "  nombre TEXT NOT NULL UNIQUE,"
        "  " COL_FECHA " TEXT NOT NULL DEFAULT '',"
        "  PRIMARY KEY (categoria, id)"
        ");";
    if (!db_ejecutar(db, sql)) return 0;

    sqlite3_stmt *stmt = db_preparar(db, "PRAGMA table_info(" TABLE_HOBBIES ");");
    int has_fecha = 0;
    if (stmt)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *col = (const char *)sqlite3_column_text(stmt, 1);
            if (col && strcmp(col, COL_FECHA) == 0)
            {
                has_fecha = 1;
                break;
            }
        }
        sqlite3_finalize(stmt);
    }

    if (!has_fecha)
    {
        db_ejecutar(db, "ALTER TABLE " TABLE_HOBBIES " ADD COLUMN " COL_FECHA " TEXT NOT NULL DEFAULT '';");
        db_ejecutar(db,
                    "UPDATE " TABLE_HOBBIES " SET " COL_FECHA " = "
                    "datetime('" MIGRATION_EPOCH "', '+' || (id - 1) || ' days') "
                    "WHERE " COL_FECHA " = '';");
    }

    return 1;
}

/**
 * @brief Cuenta los hobbies reales de una categoría (excluye placeholders).
 * @details Usa COUNT(*) con filtro NOT LIKE para excluir registros
 *          cuyo nombre termina en HOBBY_PLACEHOLDER.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría a consultar.
 *
 * @return Cantidad de hobbies reales (>= 0), o -1 si hubo error.
 *
 * @see HOBBY_PLACEHOLDER
 * @see MAX_POR_CATEGORIA
 */
int hobby_contar_por_categoria(Database *db, const char *categoria)
{
    if (!db || !categoria) return -1;

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT COUNT(*) FROM " TABLE_HOBBIES " WHERE categoria = ? AND nombre NOT LIKE ?;");
    if (!stmt) return -1;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "%" HOBBY_PLACEHOLDER, -1, SQLITE_STATIC);

    int count = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count;
}

/**
 * @brief Verifica si un nombre de hobby ya existe en la base de datos.
 * @details Consulta la restricción UNIQUE global de la tabla hobbies.
 *
 * @param[in] db     Puntero a la estructura Database.
 * @param[in] nombre Nombre del hobby a verificar.
 *
 * @return 1 si existe, 0 si no existe o error.
 *
 * @note Función estática, solo se usa internamente.
 *
 * @see hobby_crear
 */
static int hobby_nombre_existe(Database *db, const char *nombre)
{
    if (!db || !nombre) return 0;

    return db_conteo(db,
                     "SELECT COUNT(*) FROM " TABLE_HOBBIES " WHERE nombre = ?;",
                     nombre) > 0;
}

/**
 * @brief Calcula el siguiente ID disponible para una categoría.
 * @details Busca el primer hueco en la secuencia de IDs (1, 2, 3, ...)
 *          usando UNION ALL de IDs existentes + 1 con filtro NOT IN.
 *          Si no hay huecos, retorna el siguiente al máximo.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría.
 *
 * @return Siguiente ID disponible (>= 1).
 *
 * @note Función estática, solo se usa internamente.
 * @note Siempre retorna >= 1 (nunca 0).
 *
 * @see hobby_crear
 * @see hobby_crear_categoria
 */
static int hobby_siguiente_id(Database *db, const char *categoria)
{
    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT COALESCE(MIN(a.id), 1) FROM ("
                                     "  SELECT t.id + 1 AS id FROM " TABLE_HOBBIES " t WHERE t.categoria = ?"
                                     "  UNION ALL"
                                     "  SELECT 1"
                                     ") a"
                                     " WHERE a.id NOT IN (SELECT id FROM " TABLE_HOBBIES " WHERE categoria = ?);");
    if (!stmt) return 1;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, categoria, -1, SQLITE_STATIC);

    int id = 1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return id;
}

/**
 * @brief Crea un nuevo hobby en la base de datos.
 * @details Verifica el límite MAX_POR_CATEGORIA, calcula el siguiente ID
 *          con hobby_siguiente_id() y asigna la fecha/hora actual.
 *
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     nombre    Nombre del hobby (máximo MAX_HOBBY_LEN).
 * @param[in]     categoria Nombre de la categoría destino.
 *
 * @return ID del hobby creado (>= 1), o -1 si hubo error o límite alcanzado.
 *
 * @pre db debe ser válido. categoria debe existir en la BD.
 * @post Se inserta un nuevo registro en TABLE_HOBBIES.
 *
 * @see hobby_crear_categoria
 * @see hobby_siguiente_id
 * @see hobby_contar_por_categoria
 */
int hobby_crear(Database *db, const char *nombre, const char *categoria)
{
    if (!db || !nombre || !categoria) return -1;

    int total_en_cat = hobby_contar_por_categoria(db, categoria);
    if (total_en_cat >= MAX_POR_CATEGORIA)
    {
        printf("  Error: la categoria '%s' ya tiene %d hobbies (maximo %d).\n",
               categoria, total_en_cat, MAX_POR_CATEGORIA);
        return -1;
    }
    if (hobby_nombre_existe(db, nombre))
    {
        printf("  Error: ya existe un hobby llamado '%s'.\n", nombre);
        return -1;
    }
    int nuevo_id = hobby_siguiente_id(db, categoria);

    sqlite3_stmt *stmt = db_preparar(db,
                                     "INSERT INTO " TABLE_HOBBIES " (id, nombre, categoria, " COL_FECHA ") "
                                     "VALUES (?, ?, ?, datetime('now', 'localtime'));");
    if (!stmt) return -1;

    sqlite3_bind_int(stmt, 1, nuevo_id);
    sqlite3_bind_text(stmt, 2, nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, categoria, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    int id = (rc == SQLITE_DONE) ? nuevo_id : -1;
    sqlite3_finalize(stmt);

    if (id > 0)
        printf("  Hobby creado correctamente.\n");
    else
        printf("  Error: no se pudo crear el hobby en la base de datos.\n");

    return id;
}

/**
 * @brief Actualiza el nombre y/o categoría de un hobby existente.
 * @details Maneja cuatro casos según los parámetros no-NULL:
 *          - Solo nombre: actualiza nombre
 *          - Solo categoría: actualiza categoría
 *          - Ambos: actualiza nombre y categoría
 *          - Ninguno: retorna 0 sin cambios
 *          Verifica el límite MAX_POR_CATEGORIA si se cambia de categoría.
 *
 * @param[in,out] db               Puntero a la estructura Database.
 * @param[in]     id               ID del hobby a actualizar.
 * @param[in]     categoria_actual Nombre de la categoría actual del hobby.
 * @param[in]     nombre           Nuevo nombre (NULL para mantener el actual).
 * @param[in]     categoria_nueva  Nueva categoría (NULL para mantener la actual).
 *
 * @return 1 si se modificó al menos 1 fila, 0 si no encontrado, error o sin cambios.
 *
 * @pre id debe existir en la categoría categoria_actual.
 * @post El registro se actualiza en TABLE_HOBBIES si hubo cambios.
 *
 * @see hobby_eliminar
 * @see hobby_nro_a_id
 */
int hobby_actualizar(Database *db, int id, const char *categoria_actual, const char *nombre, const char *categoria_nueva)
{
    if (!db || !categoria_actual) return 0;

    if (categoria_nueva)
    {
        int total_en_cat = hobby_contar_por_categoria(db, categoria_nueva);
        if (total_en_cat >= MAX_POR_CATEGORIA)
        {
            printf("  Error: la categoria '%s' ya tiene %d hobbies (maximo %d).\n",
                   categoria_nueva, total_en_cat, MAX_POR_CATEGORIA);
            return 0;
        }
    }

    sqlite3_stmt *stmt = NULL;

    if (nombre && categoria_nueva)
    {
        stmt = db_preparar(db,
                           "UPDATE " TABLE_HOBBIES " SET nombre = ?, categoria = ? WHERE categoria = ? AND id = ?;");
        if (!stmt) return 0;
        sqlite3_bind_text(stmt, 1, nombre, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, categoria_nueva, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, categoria_actual, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, id);
    }
    else if (nombre)
    {
        stmt = db_preparar(db,
                           "UPDATE " TABLE_HOBBIES " SET nombre = ? WHERE categoria = ? AND id = ?;");
        if (!stmt) return 0;
        sqlite3_bind_text(stmt, 1, nombre, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, categoria_actual, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, id);
    }
    else if (categoria_nueva)
    {
        stmt = db_preparar(db,
                           "UPDATE " TABLE_HOBBIES " SET categoria = ? WHERE categoria = ? AND id = ?;");
        if (!stmt) return 0;
        sqlite3_bind_text(stmt, 1, categoria_nueva, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, categoria_actual, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, id);
    }
    else
    {
        return 0;
    }

    sqlite3_step(stmt);
    int changed = sqlite3_changes(db->db);
    sqlite3_finalize(stmt);

    if (changed)
        printf("  Hobby %d en '%s' actualizado.\n", id, categoria_actual);
    else
        printf("  No se encontro hobby con ID %d en '%s'.\n", id, categoria_actual);

    return changed > 0;
}

/**
 * @brief Elimina un hobby por ID y categoría.
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     id        ID del hobby a eliminar.
 * @param[in]     categoria Categoría del hobby a eliminar.
 *
 * @return 1 si se eliminó, 0 si no encontrado o error.
 *
 * @pre id debe existir en la categoría especificada.
 * @post Se elimina el registro correspondiente de TABLE_HOBBIES.
 *
 * @see hobby_eliminar_categoria
 * @see hobby_actualizar
 */
int hobby_eliminar(Database *db, int id, const char *categoria)
{
    if (!db || !categoria) return 0;

    sqlite3_stmt *stmt = db_preparar(db, "DELETE FROM " TABLE_HOBBIES " WHERE categoria = ? AND id = ?;");
    if (!stmt) return 0;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);

    int changed = sqlite3_changes(db->db);
    sqlite3_finalize(stmt);

    if (changed)
        printf("  Hobby %d en '%s' eliminado.\n", id, categoria);
    else
        printf("  No se encontro hobby con ID %d en '%s'.\n", id, categoria);

    return changed > 0;
}

/**
 * @brief Selecciona un hobby aleatorio de la base de datos.
 * @details Usa ORDER BY RANDOM() LIMIT 1 para obtener un registro
 *          al azar de la tabla completa. Copia categoría y nombre
 *          a los buffers proporcionados.
 *
 * @param[in,out] db          Puntero a la estructura Database.
 * @param[out]    buffer_cat  Buffer donde copiar la categoría.
 * @param[in]     tam_cat     Tamaño del buffer de categoría.
 * @param[out]    buffer_hobby Buffer donde copiar el nombre del hobby.
 * @param[in]     tam_hobby   Tamaño del buffer de hobby.
 *
 * @return 1 si se seleccionó un hobby, 0 si la tabla está vacía o error.
 *
 * @pre db debe ser válido. Los buffers deben tener tamaño suficiente.
 * @post Los buffers contienen la categoría y nombre del hobby seleccionado.
 *
 * @see hobby_listar_por_categoria
 */
int hobby_seleccionar_aleatorio(Database *db, char *buffer_cat, int tam_cat, char *buffer_hobby, int tam_hobby)
{
    if (!db || !buffer_cat || !buffer_hobby || tam_cat <= 0 || tam_hobby <= 0) return 0;

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT categoria, nombre FROM " TABLE_HOBBIES " ORDER BY RANDOM() LIMIT 1;");
    if (!stmt) return 0;

    int ok = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *categoria = (const char *)sqlite3_column_text(stmt, 0);
        const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
        if (categoria && nombre)
        {
            strncpy(buffer_cat, categoria, tam_cat - 1);
            buffer_cat[tam_cat - 1] = '\0';
            strncpy(buffer_hobby, nombre, tam_hobby - 1);
            buffer_hobby[tam_hobby - 1] = '\0';
            ok = 1;
        }
    }
    sqlite3_finalize(stmt);
    return ok;
}

/**
 * @brief Muestra en pantalla las categorías disponibles con su conteo.
 * @details Imprime una tabla formateada con Nº secuencial, nombre de
 *          categoría y proporción de hobbies (actual/máximo). Usa GROUP BY
 *          con filtro NOT LIKE para excluir categorías sin hobbies reales.
 *
 * @param[in] db Puntero a la estructura Database.
 *
 * @pre db debe ser válido.
 * @post Se imprime la tabla en stdout.
 *
 * @see hobby_categoria_existe
 * @see hobby_contar_por_categoria
 */
void hobby_mostrar_categorias(Database *db)
{
    if (!db) return;

    printf("\n  +-----------------------------------------+\n");
    printf("  |         CATEGORIAS DISPONIBLES          |\n");
    printf("  +-----------------------------------------+\n");
    fflush(stdout);

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT categoria, COUNT(*) FROM " TABLE_HOBBIES " "
                                     "WHERE nombre NOT LIKE ? "
                                     "GROUP BY categoria ORDER BY categoria;");
    if (stmt)
    {
        sqlite3_bind_text(stmt, 1, "%" HOBBY_PLACEHOLDER, -1, SQLITE_STATIC);
        int i = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *cat = (const char *)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            printf("  |  [%d] %-15s (%d/%d)       |\n", i, cat, count, MAX_POR_CATEGORIA);
            i++;
        }
        sqlite3_finalize(stmt);
    }
    fflush(stdout);

    printf("  +-----------------------------------------+\n\n");
    fflush(stdout);
}

/**
 * @brief Verifica si una categoría existe en la base de datos.
 * @details Cuenta todos los registros (incluyendo placeholders) de la
 *          categoría en TABLE_HOBBIES.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría a verificar.
 *
 * @return 1 si existe al menos 1 registro, 0 si no existe o error.
 *
 * @see hobby_crear_categoria
 * @see hobby_eliminar_categoria
 */
int hobby_categoria_existe(Database *db, const char *categoria)
{
    if (!db || !categoria) return 0;

    return db_conteo(db,
                     "SELECT COUNT(*) FROM " TABLE_HOBBIES " WHERE categoria = ?;",
                     categoria) > 0;
}

/**
 * @brief Lista los hobbies de una categoría específica en pantalla.
 * @details Muestra una tabla con Nº secuencial (1-5) y nombre, ordenada
 *          por COL_FECHA. Excluye registros placeholder. Imprime el
 *          total de hobbies al final.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría a listar.
 *
 * @return Cantidad de hobbies listados (>= 0), o -1 si hubo error.
 *
 * @pre db y categoria deben ser válidos.
 * @post Se imprime la tabla en stdout con el total al final.
 *
 * @see hobby_nro_a_id
 * @see HOBBY_PLACEHOLDER
 */
int hobby_listar_por_categoria(Database *db, const char *categoria)
{
    if (!db || !categoria) return -1;

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT nombre FROM " TABLE_HOBBIES " WHERE categoria = ? AND nombre NOT LIKE ? ORDER BY " COL_FECHA ";");
    if (!stmt) return -1;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "%" HOBBY_PLACEHOLDER, -1, SQLITE_STATIC);

    printf("\n  %-5s %-35s\n", "No", "Hobby");
    printf("  %-5s %-35s\n", "---", "---");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
        count++;
        printf("  %-5d %-35s\n", count, nombre ? nombre : "");
    }
    sqlite3_finalize(stmt);

    printf("\n  Total: %d/%d hobbies\n\n", count, MAX_POR_CATEGORIA);
    return count;
}

/**
 * @brief Convierte un Nº secuencial al ID real de un hobby.
 * @details El Nº corresponde al orden de visualización (por COL_FECHA),
 *          excluyendo placeholders. Usa OFFSET para mapear Nº → ID.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría.
 * @param[in] nro       Nº secuencial (1 a MAX_POR_CATEGORIA).
 *
 * @return ID real del hobby (>= 1), o -1 si el Nº es inválido o no encontrado.
 *
 * @pre nro debe ser >= 1.
 *
 * @see hobby_listar_por_categoria
 * @see hobby_actualizar
 * @see hobby_eliminar
 */
int hobby_nro_a_id(Database *db, const char *categoria, int nro)
{
    if (!db || !categoria || nro < 1) return -1;

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT id FROM " TABLE_HOBBIES " "
                                     "WHERE categoria = ? AND nombre NOT LIKE ? "
                                     "ORDER BY " COL_FECHA " "
                                     "LIMIT 1 OFFSET ?;");
    if (!stmt) return -1;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "%" HOBBY_PLACEHOLDER, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, nro - 1);

    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return id;
}

/**
 * @brief Crea una categoría insertando un registro placeholder.
 * @details Inserta un hobby temporal con nombre "categoría [nuevo]" para
 *          que la categoría exista en la BD. La categoría se considera
 *          "vacía" hasta que se agreguen hobbies reales.
 *
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     categoria Nombre de la nueva categoría.
 *
 * @return 1 si éxito, 0 si error.
 *
 * @pre La categoría no debe existir previamente (verificar con hobby_categoria_existe).
 * @post Se inserta un registro placeholder en TABLE_HOBBIES.
 *
 * @see hobby_renombrar_categoria
 * @see hobby_eliminar_categoria
 * @see HOBBY_PLACEHOLDER
 */
int hobby_crear_categoria(Database *db, const char *categoria)
{
    if (!db || !categoria) return 0;

    int nuevo_id = hobby_siguiente_id(db, categoria);
    char nombre_dummy[MAX_HOBBY_LEN];
    snprintf(nombre_dummy, sizeof(nombre_dummy), "%s" HOBBY_PLACEHOLDER, categoria);

    sqlite3_stmt *stmt = db_preparar(db,
                                     "INSERT INTO " TABLE_HOBBIES " (id, nombre, categoria, " COL_FECHA ") "
                                     "VALUES (?, ?, ?, datetime('now', 'localtime'));");
    if (!stmt) return 0;

    sqlite3_bind_int(stmt, 1, nuevo_id);
    sqlite3_bind_text(stmt, 2, nombre_dummy, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, categoria, -1, SQLITE_STATIC);

    int ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

/**
 * @brief Renombra una categoría y actualiza su registro placeholder.
 * @details Ejecuta dos UPDATEs:
 *          1. Actualiza el nombre del placeholder (vieja[nuevo] → nueva[nuevo])
 *          2. Actualiza la columna categoria de todos los registros
 *
 * @param[in,out] db    Puntero a la estructura Database.
 * @param[in]     vieja Nombre actual de la categoría.
 * @param[in]     nueva Nuevo nombre de la categoría.
 *
 * @return 1 si se modificó al menos 1 fila, 0 si no encontrado o error.
 *
 * @pre La categoría vieja debe existir.
 * @post Todos los registros de TABLE_HOBBIES con esa categoría se actualizan.
 *
 * @see hobby_crear_categoria
 * @see hobby_eliminar_categoria
 */
int hobby_renombrar_categoria(Database *db, const char *vieja, const char *nueva)
{
    if (!db || !vieja || !nueva) return 0;

    char viejo_dummy[MAX_HOBBY_LEN];
    char nuevo_dummy[MAX_HOBBY_LEN];
    snprintf(viejo_dummy, sizeof(viejo_dummy), "%s" HOBBY_PLACEHOLDER, vieja);
    snprintf(nuevo_dummy, sizeof(nuevo_dummy), "%s" HOBBY_PLACEHOLDER, nueva);

    sqlite3_stmt *stmt = db_preparar(db,
                                     "UPDATE " TABLE_HOBBIES " SET nombre = ? "
                                     "WHERE categoria = ? AND nombre = ?;");
    if (!stmt) return 0;

    sqlite3_bind_text(stmt, 1, nuevo_dummy, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, vieja, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, viejo_dummy, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    stmt = db_preparar(db,
                       "UPDATE " TABLE_HOBBIES " SET categoria = ? WHERE categoria = ?;");
    if (!stmt) return 0;

    sqlite3_bind_text(stmt, 1, nueva, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, vieja, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int changed = sqlite3_changes(db->db);
    sqlite3_finalize(stmt);

    return changed > 0;
}

/**
 * @brief Elimina una categoría y todos sus hobbies.
 * @details Ejecuta DELETE de todos los registros de TABLE_HOBBIES
 *          que pertenezcan a la categoría especificada, incluyendo
 *          placeholders.
 *
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     categoria Nombre de la categoría a eliminar.
 *
 * @return 1 si se eliminó al menos 1 registro, 0 si no encontrado o error.
 *
 * @pre La categoría debe existir.
 * @post Se eliminan todos los registros de la categoría de TABLE_HOBBIES.
 *
 * @see hobby_crear_categoria
 * @see hobby_renombrar_categoria
 */
int hobby_eliminar_categoria(Database *db, const char *categoria)
{
    if (!db || !categoria) return 0;

    sqlite3_stmt *stmt = db_preparar(db, "DELETE FROM " TABLE_HOBBIES " WHERE categoria = ?;");
    if (!stmt) return 0;

    sqlite3_bind_text(stmt, 1, categoria, -1, SQLITE_STATIC);
    sqlite3_step(stmt);

    int changed = sqlite3_changes(db->db);
    sqlite3_finalize(stmt);

    return changed > 0;
}
