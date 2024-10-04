// web_crawler_e.h
#ifndef WEB_CRAWLER_E
#define WEB_CRAWLER_E

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

#define TAG "WebCrawler"

// Define the submenu items for our WebCrawler application
typedef enum
{
    WebCrawlerSubmenuIndexRun,
    WebCrawlerSubmenuIndexAbout,
    WebCrawlerSubmenuIndexSetPath,
    WebCrawlerSubmenuIndexData,
} WebCrawlerSubmenuIndex;

// Define views for our WebCrawler application
typedef enum
{
    WebCrawlerViewSubmenu,           // Submenu
    WebCrawlerViewAbout,             // About screen
    WebCrawlerViewConfigure,         // Configuration screen
    WebCrawlerViewTextInput,         // Text input for Path
    WebCrawlerViewTextInputSSID,     // Text input for SSID
    WebCrawlerViewTextInputPassword, // Text input for Password
    WebCrawlerViewRun,               // Main run view
    WebCrawlerViewData,              // Data view
} WebCrawlerView;

// Define the application structure
typedef struct
{
    ViewDispatcher *view_dispatcher;
    View *view_main;
    Submenu *submenu;
    Widget *widget_about;
    TextInput *text_input_path;
    TextInput *text_input_ssid;
    TextInput *text_input_password;
    Widget *textbox;
    VariableItemList *variable_item_list_config;

    char *path;
    char *ssid;
    char *password;
    VariableItem *path_item;
    VariableItem *ssid_item;
    VariableItem *password_item;

    char *temp_buffer_path;
    uint32_t temp_buffer_size_path;

    char *temp_buffer_ssid;
    uint32_t temp_buffer_size_ssid;

    char *temp_buffer_password;
    uint32_t temp_buffer_size_password;
} WebCrawlerApp;

// forware declaration of storage functions
static void save_settings(const char *path, const char *ssid, const char *password);
static bool load_settings(char *path, size_t path_size, char *ssid, size_t ssid_size, char *password, size_t password_size, WebCrawlerApp *app);
static bool load_received_data();

#endif // WEB_CRAWLER_E