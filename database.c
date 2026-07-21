/**
 * @file database.c
 * @brief Implementación del módulo de acceso a datos SQLite.
 * @details Gestiona la apertura/cierre de la base de datos, ejecución
 *          de sentencias SQL, preparación de statements parametrizados
 *          y escritura de log con timestamp. Los archivos se almacenan
 *          en LOCALAPPDATA\MiHobbyC\data/.
 *
 * @author MiHobbyC
 * @version 2.0
 * @date 2026
 *
 * @see database.h para la interfaz pública.
 * @see hobby.c para las consultas SQL que utilizan esta capa.
 */

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>

/**
 * @brief Obtiene la fecha y hora actual formateada.
 * @details Usa localtime() y strftime() con el formato LOG_DATE_FORMAT.
 *
 * @param[out] buffer Buffer donde escribir la cadena de fecha.
 * @param[in]  tam    Tamaño del buffer.
 *
 * @pre buffer y tam deben ser válidos.
 * @post buffer contiene la fecha/hora en formato "YYYY-MM-DD HH:MM:SS".
 *
 * @warning No es thread-safe (usa localtime() estático).
 *
 * @see LOG_DATE_FORMAT
 */
static void obtener_fecha_hora(char *buffer, int tam)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buffer, tam, LOG_DATE_FORMAT, tm);
}

/**
 * @brief Crea recursivamente los directorios de una ruta.
 * @details Itera sobre la ruta separando por separadores de ruta
 *          y llamando _mkdir() en cada nivel intermedio.
 *
 * @param[in] ruta Ruta completa del directorio a crear.
 *
 * @return 0 si éxito (o ya existía), -1 si error.
 *
 * @note Solo funciona en Windows (_mkdir de direct.h).
 *
 * @see construir_ruta_base
 */
static int crear_directorio(const char *ruta)
{
    char tmp[PATH_BUF_SIZE];
    strncpy(tmp, ruta, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '\\' || *p == '/')
        {
            *p = '\0';
            _mkdir(tmp);
            *p = '\\';
        }
    }
    return _mkdir(tmp);
}

/**
 * @brief Construye la ruta base donde se almacenan los archivos de la aplicación.
 * @details Intenta usar LOCALAPPDATA; si no está disponible, usa USERPROFILE
 *          con AppData\Local como prefijo. La ruta resultante es
 *          <BASE>\MiHobbyC\data\.
 *
 * @param[out] buffer Buffer donde escribir la ruta construida.
 * @param[in]  tam    Tamaño del buffer.
 *
 * @return 1 si la ruta se construyó, 0 si no se pudo determinar el HOME.
 *
 * @see DB_SUBCARPETA
 * @see crear_directorio
 */
static int construir_ruta_base(char *buffer, int tam)
{
    const char *local = getenv("LOCALAPPDATA");
    if (!local || local[0] == '\0')
    {
        local = getenv("USERPROFILE");
        if (!local || local[0] == '\0')
        {
            fprintf(stderr, "No se pudo determinar la ruta del usuario.\n");
            return 0;
        }
        snprintf(buffer, tam, "%s\\AppData\\Local\\%s", local, DB_SUBCARPETA);
    }
    else
    {
        snprintf(buffer, tam, "%s\\%s", local, DB_SUBCARPETA);
    }
    return 1;
}

/**
 * @brief Abre o crea la base de datos y el archivo de log.
 * @details Secuencia de inicialización:
 *          1. Construir ruta base en LOCALAPPDATA
 *          2. Crear directorios
 *          3. Abrir archivo de log en modo append
 *          4. Escribir entrada de inicio
 *          5. Abrir/crear base de datos SQLite
 *          6. Configurar WAL journal y foreign keys
 *          7. Registrar ruta de BD en el log
 *
 * @param[out] db Puntero a la estructura Database a inicializar.
 *
 * @return 1 si la apertura fue exitosa, 0 si hubo error.
 *
 * @pre db debe ser un puntero válido.
 * @post db->db y db->log están inicializados.
 *
 * @see db_cerrar
 * @see db_log
 */
int db_abrir(Database *db)
{
    if (!db) return 0;

    db->db = NULL;
    db->log = NULL;

    char ruta_base[PATH_BUF_SIZE];
    if (!construir_ruta_base(ruta_base, sizeof(ruta_base)))
        return 0;

    crear_directorio(ruta_base);

    size_t len_base = strlen(ruta_base);

    char ruta_log[PATH_BUF_SIZE];
    if (len_base + strlen(DB_LOG) + 2 > sizeof(ruta_log))
    {
        fprintf(stderr, "Ruta de log demasiado larga.\n");
        return 0;
    }
    snprintf(ruta_log, sizeof(ruta_log), "%s\\%s", ruta_base, DB_LOG);
    db->log = fopen(ruta_log, "a");
    if (!db->log)
    {
        fprintf(stderr, "No se pudo crear el archivo de log.\n");
        return 0;
    }

    char fecha[FECHA_BUF_SIZE];
    obtener_fecha_hora(fecha, sizeof(fecha));
    fprintf(db->log, "[%s] === Sistema iniciado ===\n", fecha);
    fflush(db->log);

    char ruta_db[PATH_BUF_SIZE];
    if (len_base + strlen(DB_ARCHIVO) + 2 > sizeof(ruta_db))
    {
        fprintf(stderr, "Ruta de base de datos demasiado larga.\n");
        db_log(db, "ERROR: Ruta de BD demasiado larga");
        return 0;
    }
    snprintf(ruta_db, sizeof(ruta_db), "%s\\%s", ruta_base, DB_ARCHIVO);

    int rc = sqlite3_open(ruta_db, &db->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error al abrir BD: %s\n", sqlite3_errmsg(db->db));
        db_log(db, "ERROR: No se pudo abrir la base de datos");
        return 0;
    }

    sqlite3_exec(db->db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_exec(db->db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "Base de datos abierta: %s", ruta_db);
    db_log(db, msg);

    return 1;
}

/**
 * @brief Cierra la base de datos y el archivo de log.
 * @details Escribe entrada de cierre en el log, cierra la conexión
 *          SQLite y el FILE* del log.
 *
 * @param[in,out] db Puntero a la estructura Database.
 *
 * @pre db debe haber sido abierto con db_abrir().
 * @post db->db y db->log son NULL.
 *
 * @see db_abrir
 */
void db_cerrar(Database *db)
{
    if (!db) return;

    if (db->db)
    {
        db_log(db, "Base de datos cerrada");
        sqlite3_close(db->db);
        db->db = NULL;
    }

    if (db->log)
    {
        char fecha[FECHA_BUF_SIZE];
        obtener_fecha_hora(fecha, sizeof(fecha));
        fprintf(db->log, "[%s] === Sistema detenido ===\n\n", fecha);
        fclose(db->log);
        db->log = NULL;
    }
}

/**
 * @brief Ejecuta una sentencia SQL sin retornar resultados.
 * @details Usa sqlite3_exec(). Registra errores tanto en el log
 *          como en stderr, y libera el mensaje de error de SQLite.
 *
 * @param[in,out] db  Puntero a la estructura Database.
 * @param[in]     sql Sentencia SQL (INSERT, UPDATE, DELETE, DDL).
 *
 * @return 1 si éxito, 0 si error.
 *
 * @pre db debe estar abierto. sql debe ser una sentencia válida.
 * @post La sentencia se ejecuta sobre la base de datos.
 *
 * @see db_preparar
 * @see sqlite3_exec
 */
int db_ejecutar(Database *db, const char *sql)
{
    if (!db || !db->db || !sql) return 0;

    char *err = NULL;
    int rc = sqlite3_exec(db->db, sql, NULL, NULL, &err);

    if (rc != SQLITE_OK)
    {
        char msg[MSG_BUF_SIZE];
        snprintf(msg, sizeof(msg), "ERROR SQL: %s", err ? err : "desconocido");
        db_log(db, msg);
        fprintf(stderr, "Error SQL: %s\n", err ? err : "desconocido");
        sqlite3_free(err);
        return 0;
    }
    return 1;
}

/**
 * @brief Prepara una sentencia SQL para ejecución parametrizada.
 * @details Usa sqlite3_prepare_v2(). Registra errores en el log y stderr.
 *          El llamador debe llamar sqlite3_finalize() al statement devuelto.
 *
 * @param[in] db  Puntero a la estructura Database.
 * @param[in] sql Sentencia SQL con parámetros tipo ?.
 *
 * @return Puntero al statement preparado, o NULL si error.
 *
 * @pre db debe estar abierto. sql debe ser una sentencia válida.
 * @post El statement está listo para sqlite3_bind_*() + sqlite3_step().
 *
 * @see db_ejecutar
 * @see sqlite3_bind_text
 * @see sqlite3_bind_int
 * @see sqlite3_finalize
 */
sqlite3_stmt *db_preparar(Database *db, const char *sql)
{
    if (!db || !db->db || !sql) return NULL;

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        char msg[MSG_BUF_SIZE];
        snprintf(msg, sizeof(msg), "ERROR al preparar: %s", sqlite3_errmsg(db->db));
        db_log(db, msg);
        fprintf(stderr, "Error al preparar: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }
    return stmt;
}

/**
 * @brief Escribe una entrada en el archivo de log con timestamp.
 * @details Formato: [YYYY-MM-DD HH:MM:SS] mensaje seguido de newline.
 *          Hace flush inmediato para garantizar persistencia.
 *
 * @param[in] db       Puntero a la estructura Database.
 * @param[in] mensaje  Texto de la entrada de log.
 *
 * @pre db->log debe estar abierto. mensaje debe ser válido.
 * @post La entrada se escribe y se hace flush.
 *
 * @see LOG_DATE_FORMAT
 * @see db_abrir
 */
void db_log(Database *db, const char *mensaje)
{
    if (!db || !db->log || !mensaje) return;

    char fecha[FECHA_BUF_SIZE];
    obtener_fecha_hora(fecha, sizeof(fecha));
    fprintf(db->log, "[%s] %s\n", fecha, mensaje);
    fflush(db->log);
}

int db_conteo(Database *db, const char *sql, const char *param)
{
    if (!db || !sql || !param) return -1;

    sqlite3_stmt *stmt = db_preparar(db, sql);
    if (!stmt) return -1;

    sqlite3_bind_text(stmt, 1, param, -1, SQLITE_STATIC);

    int count = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count;
}

int db_ejecutar_cambios(Database *db, const char *sql)
{
    if (!db || !db->db || !sql) return -1;

    char *err = NULL;
    int rc = sqlite3_exec(db->db, sql, NULL, NULL, &err);

    if (rc != SQLITE_OK)
    {
        char msg[MSG_BUF_SIZE];
        snprintf(msg, sizeof(msg), "ERROR SQL: %s", err ? err : "desconocido");
        db_log(db, msg);
        fprintf(stderr, "Error SQL: %s\n", err ? err : "desconocido");
        sqlite3_free(err);
        return -1;
    }
    return sqlite3_changes(db->db);
}
