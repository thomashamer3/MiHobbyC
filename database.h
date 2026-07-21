/**
 * @file database.h
 * @brief Interfaz del módulo de acceso a datos SQLite.
 * @details Define la estructura Database, las macros de configuración
 *          de rutas y buffers, y los prototipos para abrir, cerrar,
 *          ejecutar consultas, preparar statements y escribir logs.
 *
 * @author MiHobbyC
 * @version 2.0
 * @date 2026
 *
 * @see sqlite3.h para la API de SQLite3 embebida.
 * @see hobby.h para la capa de negocio que consume esta API.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <stdio.h>

/** @brief Subcarpeta de datos dentro de LOCALAPPDATA o USERPROFILE. */
#define DB_SUBCARPETA "MiHobbyC\\data"

/** @brief Nombre del archivo de base de datos SQLite. */
#define DB_ARCHIVO    "hobbies.db"

/** @brief Nombre del archivo de log de la aplicación. */
#define DB_LOG        "mihobbyc.log"

/** @brief Tamaño de los buffers para rutas de archivo. */
#define PATH_BUF_SIZE   512

/** @brief Tamaño de los buffers para mensajes de log. */
#define MSG_BUF_SIZE    768

/** @brief Tamaño de los buffers para cadenas de fecha/hora. */
#define FECHA_BUF_SIZE  64

/** @brief Formato de fecha y hora para el log (strftime). */
#define LOG_DATE_FORMAT "%Y-%m-%d %H:%M:%S"

/**
 * @struct Database
 * @brief Estructura principal que encapsula la conexión a SQLite y el log.
 *
 * @var Database::db
 * Puntero a la conexión SQLite3. NULL si la BD no está abierta.
 *
 * @var Database::log
 * Puntero al archivo de log. NULL si el log no está disponible.
 */
typedef struct
{
    sqlite3 *db;   /**< Conexión a la base de datos SQLite3. */
    FILE *log;     /**< Archivo de log de la aplicación. */
} Database;

/**
 * @brief Abre o crea la base de datos y el archivo de log.
 * @details Construye la ruta en LOCALAPPDATA\\DB_SUBCARPETA (o USERPROFILE
 *          como fallback), crea los directorios necesarios, abre el log
 *          en modo append, configura WAL journal y foreign keys.
 *
 * @param[out] db Puntero a la estructura Database a inicializar.
 *
 * @return 1 si la apertura fue exitosa, 0 si hubo error.
 *
 * @pre db debe ser un puntero válido.
 * @post db->db y db->log están inicializados. Se escribe entrada de log
 *       indicando "Sistema iniciado".
 *
 * @see db_cerrar
 * @see db_log
 */
int db_abrir(Database *db);

/**
 * @brief Cierra la base de datos y el archivo de log.
 * @details Escribe una entrada de log indicando cierre, cierra la
 *          conexión SQLite y el FILE* del log.
 *
 * @param[in,out] db Puntero a la estructura Database.
 *
 * @pre db debe haber sido abierto con db_abrir().
 * @post db->db y db->log son NULL. Los recursos están liberados.
 *
 * @see db_abrir
 */
void db_cerrar(Database *db);

/**
 * @brief Ejecuta una sentencia SQL sin retornar resultados (INSERT, UPDATE, DELETE, DDL).
 * @details Registra errores en el log y en stderr.
 *
 * @param[in,out] db  Puntero a la estructura Database.
 * @param[in]     sql Sentencia SQL a ejecutar.
 *
 * @return 1 si éxito, 0 si error.
 *
 * @pre db debe estar abierto. sql debe ser una sentencia válida.
 * @post La sentencia se ejecuta sobre la base de datos.
 *
 * @see db_preparar
 */
int db_ejecutar(Database *db, const char *sql);

/**
 * @brief Prepara una sentencia SQL para ejecución parametrizada.
 * @details Usa sqlite3_prepare_v2. Registra errores en el log y en stderr.
 *          El llamador es responsable de llamar sqlite3_finalize() al stmt.
 *
 * @param[in] db  Puntero a la estructura Database.
 * @param[in] sql Sentencia SQL con parámetros tipo ?.
 *
 * @return Puntero al statement preparado, o NULL si error.
 *
 * @pre db debe estar abierto. sql debe ser una sentencia válida.
 * @post El statement está listo para bind + step. Debe ser finalizado.
 *
 * @see db_ejecutar
 * @see sqlite3_finalize
 */
sqlite3_stmt *db_preparar(Database *db, const char *sql);

/**
 * @brief Escribe una entrada en el archivo de log con timestamp.
 * @details Formato: [YYYY-MM-DD HH:MM:SS] mensaje.
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
void db_log(Database *db, const char *mensaje);

/**
 * @brief Ejecuta una consulta COUNT(*) con un parámetro text y retorna el resultado.
 * @details Prepara, vincula, ejecuta y finaliza el statement en una sola llamada.
 *          Útil para consultas del tipo SELECT COUNT(*) FROM ... WHERE campo = ?.
 *
 * @param[in] db       Puntero a la estructura Database.
 * @param[in] sql      Sentencia SQL con exactamente 1 parámetro ?.
 * @param[in] param    Valor del parámetro text.
 *
 * @return Resultado del COUNT (>= 0), o -1 si hubo error.
 *
 * @pre db debe estar abierto. sql debe tener exactamente 1 parámetro ?.
 *
 * @see db_preparar
 * @see sqlite3_bind_text
 */
int db_conteo(Database *db, const char *sql, const char *param);

/**
 * @brief Ejecuta una sentencia SQL y retorna la cantidad de filas modificadas.
 * @details Similar a db_ejecutar() pero retorna sqlite3_changes() en lugar de 0/1.
 *          Útil para UPDATE/DELETE donde se necesita saber cuántas filas se afectaron.
 *
 * @param[in,out] db  Puntero a la estructura Database.
 * @param[in]     sql Sentencia SQL a ejecutar.
 *
 * @return Cantidad de filas modificadas (>= 0), o -1 si hubo error.
 *
 * @pre db debe estar abierto. sql debe ser una sentencia válida.
 *
 * @see db_ejecutar
 * @see sqlite3_changes
 */
int db_ejecutar_cambios(Database *db, const char *sql);

#endif
