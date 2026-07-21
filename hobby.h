/**
 * @file hobby.h
 * @brief Interfaz del módulo de gestión de hobbies y categorías.
 * @details Define las constantes, macros y prototipos públicos para
 *          operaciones CRUD sobre la tabla de hobbies, selección
 *          aleatoria, gestión de categorías y conversión de Nº a ID.
 *
 * @author MiHobbyC
 * @version 2.0
 * @date 2026
 *
 * @see database.h para la capa de persistencia SQLite.
 * @see main.c para la interfaz de usuario que consume esta API.
 */

#ifndef HOBBY_H
#define HOBBY_H

#include "database.h"

/** @brief Longitud máxima del nombre de un hobby. */
#define MAX_HOBBY_LEN 100

/** @brief Longitud máxima del nombre de una categoría. */
#define MAX_CATEGORIA_LEN 50

/** @brief Cantidad máxima de hobbies por categoría. */
#define MAX_POR_CATEGORIA 5

/** @brief Nombre de la tabla de hobbies en la base de datos. */
#define TABLE_HOBBIES        "hobbies"

/** @brief Nombre de la columna de fecha de creación. */
#define COL_FECHA            "fecha_creacion"

/** @brief Sufijo de los registros placeholder usados para crear categorías vacías. */
#define HOBBY_PLACEHOLDER    " [nuevo]"

/** @brief Fecha epoch base para la migración de registros sin fecha. */
#define MIGRATION_EPOCH      "2020-01-01"

/**
 * @brief Inicializa la tabla de hobbies en la base de datos.
 * @details Crea la tabla si no existe y ejecuta la migración
 *          de la columna fecha_creacion si falta.
 *
 * @param[in,out] db Puntero a la estructura Database inicializada.
 *
 * @return 1 si la inicialización fue exitosa, 0 si hubo error.
 *
 * @pre db debe ser válido y haber sido abierto con db_abrir().
 * @post La tabla TABLE_HOBBIES existe con la columna COL_FECHA.
 *
 * @see db_abrir
 */
int hobby_init(Database *db);

/**
 * @brief Crea un nuevo hobby en la base de datos.
 * @details Verifica que la categoría no exceda MAX_POR_CATEGORIA,
 *          calcula el siguiente ID disponible y asigna la fecha actual.
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
 * @see MAX_POR_CATEGORIA
 */
int hobby_crear(Database *db, const char *nombre, const char *categoria);

/**
 * @brief Actualiza el nombre y/o categoría de un hobby existente.
 * @details Si se proporcionan ambos parámetros, actualiza nombre y categoría.
 *          Si solo se proporciona uno, actualiza solo ese campo.
 *          Verifica que la categoría destino no exceda MAX_POR_CATEGORIA.
 *
 * @param[in,out] db               Puntero a la estructura Database.
 * @param[in]     id               ID del hobby a actualizar.
 * @param[in]     categoria_actual Nombre de la categoría actual del hobby.
 * @param[in]     nombre           Nuevo nombre (NULL para mantener el actual).
 * @param[in]     categoria_nueva  Nueva categoría (NULL para mantener la actual).
 *
 * @return 1 si se modificó al menos 1 fila, 0 si no encontrado o error.
 *
 * @pre id debe existir en la categoría categoria_actual.
 * @post El registro se actualiza en TABLE_HOBBIES.
 *
 * @see hobby_eliminar
 * @see hobby_nro_a_id
 */
int hobby_actualizar(Database *db, int id, const char *categoria_actual, const char *nombre, const char *categoria_nueva);

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
 */
int hobby_eliminar(Database *db, int id, const char *categoria);

/**
 * @brief Selecciona un hobby aleatorio de la base de datos con su categoría.
 * @details Usa ORDER BY RANDOM() LIMIT 1 para obtener un registro al azar.
 *
 * @param[in,out] db          Puntero a la estructura Database.
 * @param[out]    buffer_cat  Buffer donde copiar la categoría seleccionada.
 * @param[in]     tam_cat     Tamaño del buffer de categoría.
 * @param[out]    buffer_hobby Buffer donde copiar el nombre del hobby.
 * @param[in]     tam_hobby   Tamaño del buffer de hobby.
 *
 * @return 1 si se seleccionó un hobby, 0 si la tabla está vacía o error.
 *
 * @pre db debe ser válido. Los buffers deben tener tamaño suficiente.
 * @post Los buffers contienen la categoría y nombre del hobby seleccionado.
 *
 * @see mostrar_hobby_seleccionado
 */
int hobby_seleccionar_aleatorio(Database *db, char *buffer_cat, int tam_cat, char *buffer_hobby, int tam_hobby);

/**
 * @brief Cuenta los hobbies reales (excluyendo placeholders) de una categoría.
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría a consultar.
 *
 * @return Cantidad de hobbies (>= 0), o -1 si hubo error.
 *
 * @see MAX_POR_CATEGORIA
 */
int hobby_contar_por_categoria(Database *db, const char *categoria);

/**
 * @brief Muestra en pantalla las categorías disponibles con su conteo de hobbies.
 * @details Imprime una tabla formateada con Nº, nombre de categoría y
 *          proporción de hobbies (actual/máximo). Excluye categorías
 *          que solo contengan registros placeholder.
 *
 * @param[in] db Puntero a la estructura Database.
 *
 * @pre db debe ser válido.
 * @post Se imprime la tabla en stdout.
 *
 * @see hobby_categoria_existe
 * @see resolver_categoria
 */
void hobby_mostrar_categorias(Database *db);

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
 */
int hobby_categoria_existe(Database *db, const char *categoria);

/**
 * @brief Lista los hobbies de una categoría específica en pantalla.
 * @details Muestra una tabla con Nº secuencial (1-5) y nombre, ordenada
 *          por fecha de creación. Excluye registros placeholder.
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
 * @see hobby_listar_por_categoria
 */
int hobby_listar_por_categoria(Database *db, const char *categoria);

/**
 * @brief Convierte un Nº secuencial (1-5) al ID real de un hobby en una categoría.
 * @details El Nº corresponde al orden de visualización en hobby_listar_por_categoria
 *          (ordenado por COL_FECHA). Usa OFFSET para mapear.
 *
 * @param[in] db        Puntero a la estructura Database.
 * @param[in] categoria Nombre de la categoría.
 * @param[in] nro       Nº secuencial (1 a MAX_POR_CATEGORIA).
 *
 * @return ID real del hobby (>= 1), o -1 si el Nº es inválido o no encontrado.
 *
 * @pre nro debe ser >= 1.
 * @see hobby_listar_por_categoria
 * @see hobby_actualizar
 * @see hobby_eliminar
 */
int hobby_nro_a_id(Database *db, const char *categoria, int nro);

/**
 * @brief Crea una categoría insertando un registro placeholder.
 * @details Inserta un hobby temporal con nombre "categoría [nuevo]" para
 *          que la categoría exista en la BD sin tener hobbies reales.
 *
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     categoria Nombre de la nueva categoría.
 *
 * @return 1 si éxito, 0 si error.
 *
 * @pre La categoría no debe existir previamente.
 * @post Se inserta un registro placeholder en TABLE_HOBBIES.
 *
 * @see hobby_renombrar_categoria
 * @see hobby_eliminar_categoria
 * @see HOBBY_PLACEHOLDER
 */
int hobby_crear_categoria(Database *db, const char *categoria);

/**
 * @brief Renombra una categoría y actualiza su registro placeholder.
 * @details Primero actualiza el nombre del placeholder (vieja → nueva),
 *          luego actualiza la columna categoria de todos los registros.
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
int hobby_renombrar_categoria(Database *db, const char *vieja, const char *nueva);

/**
 * @brief Elimina una categoría y todos sus hobbies.
 * @details Elimina todos los registros de TABLE_HOBBIES que pertenezcan
 *          a la categoría especificada, incluyendo placeholders.
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
int hobby_eliminar_categoria(Database *db, const char *categoria);

#endif
