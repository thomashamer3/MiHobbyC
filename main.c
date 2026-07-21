/**
 * @file main.c
 * @brief Punto de entrada y interfaz de usuario de MiHobbyC.
 * @details Implementa el sistema de menús interactivo para sortear
 *          hobbies aleatorios, administrar categorías (CRUD) y
 *          administrar hobbies dentro de cada categoría. Usa las
 *          capas database.h y hobby.h para persistencia y lógica.
 *
 * @author MiHobbyC
 * @version 2.0
 * @date 2026
 *
 * @see database.h para la capa de acceso a datos.
 * @see hobby.h para la lógica de negocio.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "hobby.h"

static void menu_hobbies(Database *db, const char *categoria);
static void hobby_menu_agregar(Database *db, const char *categoria);
static void hobby_menu_editar(Database *db, const char *categoria);
static void hobby_menu_eliminar(Database *db, const char *categoria);
static void menu_categorias(Database *db);
static void cat_menu_crear(Database *db);
static void cat_menu_editar(Database *db);
static void cat_menu_eliminar(Database *db);
static void cat_menu_seleccionar(Database *db);

/**
 * @brief Limpia la pantalla de la consola.
 * @details Usa system("cls") en Windows y system("clear") en Unix.
 *
 * @note Función estática, solo se usa en este archivo.
 */
static void limpiar_pantalla(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Muestra el banner principal de la aplicación.
 * @details Imprime el encabezado decorativo con el nombre del programa.
 *
 * @note Función estática, solo se usa en main().
 */
static void mostrar_banner(void)
{
    printf("\n");
    printf("  +=========================================+\n");
    printf("  |                                         |\n");
    printf("  |        *  MI HOBBY C - Selector  *      |\n");
    printf("  |                                         |\n");
    printf("  |      Descubre tu actividad del dia      |\n");
    printf("  |                                         |\n");
    printf("  +=========================================+\n\n");
}

/**
 * @brief Muestra el menú principal de opciones.
 * @details Opciones: [1] Sortear hobby, [2] Administrar categorías, [0] Salir.
 *
 * @note Función estática, solo se usa en main().
 */
static void mostrar_menu_principal(void)
{
    printf("  +-----------------------------------------+\n");
    printf("  |           MENU PRINCIPAL                |\n");
    printf("  +-----------------------------------------+\n");
    printf("  |  [1]  Sortear hobby aleatorio           |\n");
    printf("  |  [2]  Administrar categorias            |\n");
    printf("  |  [0]  Salir                             |\n");
    printf("  +-----------------------------------------+\n\n");
}

/**
 * @brief Muestra el menú de administración de categorías.
 * @details Opciones: [1] Ver, [2] Agregar, [3] Editar, [4] Eliminar,
 *          [5] Administrar hobbies de una categoría, [0] Volver.
 *
 * @note Función estática, solo se usa en menu_categorias().
 */
static void mostrar_menu_categorias(void)
{
    printf("  +-----------------------------------------+\n");
    printf("  |       ADMINISTRAR CATEGORIAS            |\n");
    printf("  +-----------------------------------------+\n");
    printf("  |  [1]  Ver categorias                    |\n");
    printf("  |  [2]  Agregar categoria                 |\n");
    printf("  |  [3]  Editar categoria                  |\n");
    printf("  |  [4]  Eliminar categoria                |\n");
    printf("  |  [5]  Administrar hobbies de una cat.   |\n");
    printf("  |  [0]  Volver                            |\n");
    printf("  +-----------------------------------------+\n\n");
}

/**
 * @brief Muestra el menú de administración de hobbies de una categoría.
 * @param[in] categoria Nombre de la categoría seleccionada.
 *
 * @details Opciones: [1] Ver, [2] Agregar, [3] Editar, [4] Eliminar, [0] Volver.
 *
 * @note Función estática, solo se usa en menu_hobbies().
 */
static void mostrar_menu_hobbies(const char *categoria)
{
    printf("  +-----------------------------------------+\n");
    printf("  |  HOBBIES DE: %-24s|\n", categoria);
    printf("  +-----------------------------------------+\n");
    printf("  |  [1]  Ver hobbies                       |\n");
    printf("  |  [2]  Agregar hobby                     |\n");
    printf("  |  [3]  Editar hobby                      |\n");
    printf("  |  [4]  Eliminar hobby                    |\n");
    printf("  |  [0]  Volver                            |\n");
    printf("  +-----------------------------------------+\n\n");
}

/**
 * @brief Limpia el buffer de entrada estándar.
 * @details Descarta todos los caracteres pendientes hasta el newline o EOF.
 *
 * @note Función estática, se usa después de scanf() para evitar problemas.
 */
static void limpiar_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Pausa la ejecución esperando que el usuario presione Enter.
 *
 * @note Función estática, se usa al final de cada operación del menú.
 */
static void pausar(void)
{
    printf("\n  Presiona Enter para continuar...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Lee una línea de texto desde stdin con un prompt.
 * @details Muestra el prompt, lee con fgets() y elimina el newline final.
 *
 * @param[in]  prompt Mensaje a mostrar antes de la entrada.
 * @param[out] buffer Buffer donde almacenar el texto leído.
 * @param[in]  tam    Tamaño máximo del buffer.
 *
 * @note Función estática, se usa para entrada de nombres de hobbies/categorías.
 */
static void leer_texto(const char *prompt, char *buffer, int tam)
{
    printf("  %s", prompt);
    fflush(stdout);
    if (fgets(buffer, tam, stdin))
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        else if (len == (size_t)(tam - 1))
        {
            int c;
            while ((c = fgetc(stdin)) != '\n' && c != EOF)
                ;
        }
    }
}

/**
 * @brief Resuelve un Nº de categoría al nombre real.
 * @details Si el buffer contiene un número válido (> 0), consulta la BD
 *          y reemplaza el buffer con el nombre de la categoría correspondiente.
 *          Si contiene texto no numérico, retorna 0 (error).
 *
 * @param[in,out] db     Puntero a la estructura Database.
 * @param[in,out] buffer Buffer con el Nº de la categoría.
 * @param[in]     tam    Tamaño del buffer.
 *
 * @return 1 si se resolvió correctamente, 0 si el Nº es inválido o no encontrado.
 *
 * @note Función estática, usa la misma query que hobby_mostrar_categorias
 *       para garantizar consistencia en el numbering.
 *
 * @see hobby_mostrar_categorias
 * @see hobby_categoria_existe
 */
static int resolver_categoria(Database *db, char *buffer, int tam)
{
    char *endptr;
    long num = strtol(buffer, &endptr, 10);
    if (*endptr != '\0' || num <= 0)
        return 0;

    sqlite3_stmt *stmt = db_preparar(db,
                                     "SELECT DISTINCT categoria FROM " TABLE_HOBBIES " "
                                     "WHERE nombre NOT LIKE ? "
                                     "ORDER BY categoria;");
    if (!stmt) return 0;

    sqlite3_bind_text(stmt, 1, "%" HOBBY_PLACEHOLDER, -1, SQLITE_STATIC);

    int i = 1;
    int encontrado = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (i == num)
        {
            const char *cat = (const char *)sqlite3_column_text(stmt, 0);
            strncpy(buffer, cat, tam - 1);
            buffer[tam - 1] = '\0';
            encontrado = 1;
            break;
        }
        i++;
    }
    sqlite3_finalize(stmt);
    return encontrado;
}

/**
 * @brief Muestra el resultado del sorteo aleatorio formateado.
 * @details Imprime un recuadro decorativo con la categoría y el hobby seleccionado.
 *
 * @param[in] categoria Nombre de la categoría del hobby.
 * @param[in] hobby     Nombre del hobby seleccionado.
 *
 * @note Función estática, solo se usa en main() caso 1.
 *
 * @see hobby_seleccionar_aleatorio
 */
static void mostrar_hobby_seleccionado(const char *categoria, const char *hobby)
{
    printf("\n");
    printf("  +=========================================+\n");
    printf("  |                                         |\n");
    printf("  |       *  TU HOBBY DE HOY ES  *         |\n");
    printf("  |                                         |\n");
    printf("  |   +---------------------------------+   |\n");
    printf("  |   |  %-29s  |\n", categoria);
    printf("  |   |  >> %-27s <<   |\n", hobby);
    printf("  |   +---------------------------------+   |\n");
    printf("  |                                         |\n");
    printf("  +=========================================+\n");
}

/**
 * @brief Muestra un mensaje de éxito formateado.
 * @param[in] mensaje Texto del mensaje.
 *
 * @note Función estática, se usa para confirmar operaciones CRUD exitosas.
 */
static void mostrar_mensaje_exito(const char *mensaje)
{
    printf("\n  [OK] %s\n", mensaje);
}

/**
 * @brief Muestra un mensaje de error formateado.
 * @param[in] mensaje Texto del mensaje de error.
 *
 * @note Función estática, se usa para reportar errores al usuario.
 */
static void mostrar_mensaje_error(const char *mensaje)
{
    printf("\n  [ERROR] %s\n", mensaje);
}

/**
 * @brief Muestra un mensaje informativo formateado.
 * @param[in] mensaje Texto informativo.
 *
 * @note Función estática, se usa para mensajes de cancelación o info.
 */
static void mostrar_mensaje_info(const char *mensaje)
{
    printf("\n  [INFO] %s\n", mensaje);
}

static void hobby_menu_agregar(Database *db, const char *categoria)
{
    int total = hobby_contar_por_categoria(db, categoria);
    if (total >= MAX_POR_CATEGORIA)
    {
        char msg[MSG_BUF_SIZE];
        snprintf(msg, sizeof(msg), "Esta categoria ya tiene %d hobbies (maximo).", MAX_POR_CATEGORIA);
        mostrar_mensaje_error(msg);
        return;
    }

    printf("  --- Agregar Hobby a %s ---\n", categoria);
    printf("  (Espacios disponibles: %d)\n\n", MAX_POR_CATEGORIA - total);

    char nombre[MAX_HOBBY_LEN];
    leer_texto("Nombre del hobby: ", nombre, sizeof(nombre));
    if (strlen(nombre) == 0)
    {
        mostrar_mensaje_error("El nombre no puede estar vacio.");
        return;
    }

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Crear hobby '%s' en %s", nombre, categoria);
    db_log(db, msg);

    if (hobby_crear(db, nombre, categoria) > 0)
        mostrar_mensaje_exito("Hobby agregado correctamente.");
    else
        mostrar_mensaje_error("No se pudo agregar el hobby.");
}

static void hobby_menu_editar(Database *db, const char *categoria)
{
    printf("  --- Editar Hobby en %s ---\n\n", categoria);
    hobby_listar_por_categoria(db, categoria);

    int nro;
    printf("  Nº a editar: ");
    if (scanf("%d", &nro) != 1)
    {
        limpiar_buffer();
        mostrar_mensaje_error("Numero invalido.");
        return;
    }
    limpiar_buffer();

    int id = hobby_nro_a_id(db, categoria, nro);
    if (id < 0)
    {
        mostrar_mensaje_error("Numero de hobby invalido.");
        return;
    }

    printf("  Deja vacio para mantener el valor actual.\n\n");
    char nombre_act[MAX_HOBBY_LEN] = "";
    leer_texto("Nuevo nombre: ", nombre_act, sizeof(nombre_act));

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Actualizar hobby Nº %d (ID %d)", nro, id);
    db_log(db, msg);

    if (hobby_actualizar(db, id, categoria, strlen(nombre_act) > 0 ? nombre_act : NULL, NULL))
        mostrar_mensaje_exito("Hobby actualizado correctamente.");
    else
        mostrar_mensaje_error("No se pudo actualizar el hobby.");
}

static void hobby_menu_eliminar(Database *db, const char *categoria)
{
    printf("  --- Eliminar Hobby de %s ---\n\n", categoria);
    hobby_listar_por_categoria(db, categoria);

    int nro;
    printf("  Nº a eliminar: ");
    if (scanf("%d", &nro) != 1)
    {
        limpiar_buffer();
        mostrar_mensaje_error("Numero invalido.");
        return;
    }

    int id = hobby_nro_a_id(db, categoria, nro);
    if (id < 0)
    {
        mostrar_mensaje_error("Numero de hobby invalido.");
        return;
    }

    printf("  Confirmar eliminacion? (s/n): ");
    char resp;
    scanf(" %c", &resp);
    limpiar_buffer();

    if (resp != 's' && resp != 'S')
    {
        mostrar_mensaje_info("Eliminacion cancelada.");
        return;
    }

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Eliminar hobby Nº %d (ID %d)", nro, id);
    db_log(db, msg);

    if (hobby_eliminar(db, id, categoria))
        mostrar_mensaje_exito("Hobby eliminado correctamente.");
    else
        mostrar_mensaje_error("No se pudo eliminar el hobby.");
}

/**
 * @brief Bucle del submenú de administración de hobbies de una categoría.
 * @details Implementa las opciones: ver, agregar, editar y eliminar hobbies.
 *          Usa hobby_nro_a_id() para convertir el Nº visible al ID real.
 *
 * @param[in,out] db        Puntero a la estructura Database.
 * @param[in]     categoria Nombre de la categoría seleccionada.
 *
 * @pre categoria debe existir en la BD.
 * @post Retorna cuando el usuario selecciona [0] Volver.
 *
 * @note Función estática, solo se llama desde menu_categorias().
 *
 * @see hobby_listar_por_categoria
 * @see hobby_menu_agregar
 * @see hobby_menu_editar
 * @see hobby_menu_eliminar
 */
static void menu_hobbies(Database *db, const char *categoria)
{
    int opcion;

    do
    {
        limpiar_pantalla();
        mostrar_menu_hobbies(categoria);
        printf("  Selecciona una opcion: ");
        if (scanf("%d", &opcion) != 1)
            opcion = -1;
        limpiar_buffer();

        limpiar_pantalla();

        switch (opcion)
        {
        case 1:
            printf("  --- Hobbies de %s ---\n\n", categoria);
            hobby_listar_por_categoria(db, categoria);
            break;
        case 2:
            hobby_menu_agregar(db, categoria);
            break;
        case 3:
            hobby_menu_editar(db, categoria);
            break;
        case 4:
            hobby_menu_eliminar(db, categoria);
            break;
        case 0:
            break;
        default:
            mostrar_mensaje_error("Opcion no valida. Intenta de nuevo.");
            break;
        }

        if (opcion != 0)
            pausar();

    }
    while (opcion != 0);
}

static void cat_menu_crear(Database *db)
{
    printf("  --- Nueva Categoria ---\n\n");
    hobby_mostrar_categorias(db);
    printf("\n");

    char nombre[MAX_CATEGORIA_LEN];
    leer_texto("Nombre de la nueva categoria: ", nombre, sizeof(nombre));
    if (strlen(nombre) == 0)
    {
        mostrar_mensaje_error("El nombre no puede estar vacio.");
        return;
    }

    if (hobby_categoria_existe(db, nombre))
    {
        mostrar_mensaje_error("La categoria ya existe.");
        return;
    }

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Crear categoria '%s'", nombre);
    db_log(db, msg);

    if (hobby_crear_categoria(db, nombre))
        mostrar_mensaje_exito("Categoria creada. Ahora agrega hobbies a ella.");
    else
        mostrar_mensaje_error("No se pudo crear la categoria.");
}

static void cat_menu_editar(Database *db)
{
    printf("  --- Editar Categoria ---\n\n");
    hobby_mostrar_categorias(db);

    char cat_actual[MAX_CATEGORIA_LEN] = "";
    leer_texto("Categoria a editar: ", cat_actual, sizeof(cat_actual));
    if (strlen(cat_actual) == 0)
        return;

    if (!resolver_categoria(db, cat_actual, sizeof(cat_actual)))
    {
        mostrar_mensaje_error("Numero de categoria invalido.");
        return;
    }
    if (!hobby_categoria_existe(db, cat_actual))
    {
        mostrar_mensaje_error("La categoria no existe.");
        return;
    }

    char cat_nueva[MAX_CATEGORIA_LEN] = "";
    leer_texto("Nuevo nombre: ", cat_nueva, sizeof(cat_nueva));
    if (strlen(cat_nueva) == 0)
        return;

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Renombrar '%s' a '%s'", cat_actual, cat_nueva);
    db_log(db, msg);

    if (hobby_renombrar_categoria(db, cat_actual, cat_nueva))
        mostrar_mensaje_exito("Categoria renombrada correctamente.");
    else
        mostrar_mensaje_error("No se pudo renombrar la categoria.");
}

static void cat_menu_eliminar(Database *db)
{
    printf("  --- Eliminar Categoria ---\n\n");
    hobby_mostrar_categorias(db);

    char nombre[MAX_CATEGORIA_LEN];
    leer_texto("Categoria a eliminar: ", nombre, sizeof(nombre));
    if (strlen(nombre) == 0)
        return;

    if (!resolver_categoria(db, nombre, sizeof(nombre)))
    {
        mostrar_mensaje_error("Numero de categoria invalido.");
        return;
    }
    if (!hobby_categoria_existe(db, nombre))
    {
        mostrar_mensaje_error("La categoria no existe.");
        return;
    }

    int total = hobby_contar_por_categoria(db, nombre);
    printf("  La categoria '%s' tiene %d hobbies.\n", nombre, total);
    printf("  Confirmar eliminacion? (s/n): ");
    char resp;
    scanf(" %c", &resp);
    limpiar_buffer();

    if (resp != 's' && resp != 'S')
    {
        mostrar_mensaje_info("Eliminacion cancelada.");
        return;
    }

    char msg[MSG_BUF_SIZE];
    snprintf(msg, sizeof(msg), "CRUD: Eliminar categoria '%s' con %d hobbies", nombre, total);
    db_log(db, msg);

    if (hobby_eliminar_categoria(db, nombre))
        mostrar_mensaje_exito("Categoria y sus hobbies eliminados.");
    else
        mostrar_mensaje_error("No se pudo eliminar la categoria.");
}

static void cat_menu_seleccionar(Database *db)
{
    printf("  --- Seleccionar Categoria ---\n\n");
    hobby_mostrar_categorias(db);

    char cat_seleccionada[MAX_CATEGORIA_LEN] = "";
    leer_texto("Categoria (#): ", cat_seleccionada, sizeof(cat_seleccionada));
    if (strlen(cat_seleccionada) == 0)
        return;

    if (!resolver_categoria(db, cat_seleccionada, sizeof(cat_seleccionada)))
    {
        mostrar_mensaje_error("Numero de categoria invalido.");
        return;
    }
    if (!hobby_categoria_existe(db, cat_seleccionada))
    {
        mostrar_mensaje_error("La categoria no existe.");
        return;
    }

    menu_hobbies(db, cat_seleccionada);
}

/**
 * @brief Bucle del menú de administración de categorías.
 * @details Implementa las opciones: ver, agregar, editar, eliminar categorías
 *          y acceder al submenú de hobbies de una categoría.
 *
 * @param[in,out] db Puntero a la estructura Database.
 *
 * @post Retorna cuando el usuario selecciona [0] Volver.
 *
 * @note Función estática, solo se llama desde main() caso 2.
 *
 * @see hobby_mostrar_categorias
 * @see cat_menu_crear
 * @see cat_menu_editar
 * @see cat_menu_eliminar
 * @see cat_menu_seleccionar
 */
static void menu_categorias(Database *db)
{
    int opcion;

    do
    {
        limpiar_pantalla();
        mostrar_menu_categorias();
        printf("  Selecciona una opcion: ");
        if (scanf("%d", &opcion) != 1)
            opcion = -1;
        limpiar_buffer();

        limpiar_pantalla();

        switch (opcion)
        {
        case 1:
            printf("  --- Categorias Existentes ---\n");
            hobby_mostrar_categorias(db);
            break;
        case 2:
            cat_menu_crear(db);
            break;
        case 3:
            cat_menu_editar(db);
            break;
        case 4:
            cat_menu_eliminar(db);
            break;
        case 5:
            cat_menu_seleccionar(db);
            break;
        case 0:
            break;
        default:
            mostrar_mensaje_error("Opcion no valida. Intenta de nuevo.");
            break;
        }

        if (opcion != 0)
            pausar();

    }
    while (opcion != 0);
}

/**
 * @brief Función principal - Punto de entrada del programa.
 * @details Inicializa la base de datos, ejecuta hobby_init() y entra
 *          en el bucle principal del menú. Maneja las opciones:
 *          [1] Sorteo aleatorio, [2] Administrar categorías, [0] Salir.
 *
 * @return 0 si éxito, 1 si error al inicializar la BD.
 *
 * @pre El sistema de archivos debe permitir escritura en LOCALAPPDATA.
 * @post La BD y el log están cerrados. Los recursos están liberados.
 *
 * @see db_abrir
 * @see hobby_init
 * @see db_cerrar
 */
int main(void)
{
    Database db;
    if (!db_abrir(&db))
    {
        fprintf(stderr, "No se pudo abrir la base de datos.\n");
        return 1;
    }

    if (!hobby_init(&db))
    {
        fprintf(stderr, "No se pudo inicializar la tabla de hobbies.\n");
        db_cerrar(&db);
        return 1;
    }

    char buffer_cat[MAX_CATEGORIA_LEN];
    char buffer_hobby[MAX_HOBBY_LEN];
    int opcion;

    do
    {
        limpiar_pantalla();
        mostrar_banner();
        mostrar_menu_principal();
        printf("  Selecciona una opcion: ");

        if (scanf("%d", &opcion) != 1)
            opcion = -1;
        limpiar_buffer();

        limpiar_pantalla();

        switch (opcion)
        {
        case 1:
            db_log(&db, "Sorteo aleatorio solicitado");
            if (hobby_seleccionar_aleatorio(&db, buffer_cat, sizeof(buffer_cat), buffer_hobby, sizeof(buffer_hobby)))
            {
                mostrar_hobby_seleccionado(buffer_cat, buffer_hobby);
                char msg[MSG_BUF_SIZE];
                snprintf(msg, sizeof(msg), "Hobby seleccionado: %s: %s", buffer_cat, buffer_hobby);
                db_log(&db, msg);
            }
            else
            {
                mostrar_mensaje_error("No hay hobbies registrados. Agrega categorias y hobbies primero.");
                db_log(&db, "ADVERTENCIA: No hay hobbies para seleccionar");
            }
            pausar();
            break;
        case 2:
            menu_categorias(&db);
            break;
        case 0:
            limpiar_pantalla();
            printf("\n");
            printf("  +=========================================+\n");
            printf("  |                                         |\n");
            printf("  |    Que tengas un excelente dia!         |\n");
            printf("  |                                         |\n");
            printf("  +=========================================+\n\n");
            break;
        default:
            mostrar_mensaje_error("Opcion no valida. Intenta de nuevo.");
            pausar();
            break;
        }
    }
    while (opcion != 0);

    db_cerrar(&db);
    return 0;
}
