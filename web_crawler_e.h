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
    WebCrawlerSubmenuIndexRun,     // click to go to Run the GET request
    WebCrawlerSubmenuIndexAbout,   // click to go to About screen
    WebCrawlerSubmenuIndexConfig,  // click to go to Config submenu (Wifi, File)
    WebCrawlerSubmenuIndexRequest, // click to go to Request submenu (Set URL, HTTP Method, Headers)
    WebCrawlerSubmenuIndexWifi,    // click to go to Wifi submenu (SSID, Password)
    WebCrawlerSubmenuIndexFile,    // click to go to file submenu (Read, File Type, Rename, Delete)
} WebCrawlerSubmenuIndex;

typedef enum
{
    WebCrawlerViewRun,                     // Run the GET request
    WebCrawlerViewAbout,                   // About screen
    WebCrawlerViewSubmenuConfig,           // Submenu Config view for App (Wifi, File)
    WebCrawlerViewVariableItemListRequest, // Submenu for URL (Set URL, HTTP Method, Headers)
    WebCrawlerViewVariableItemListWifi,    // Wifi Configuration screen (Submenu for SSID, Password)
    WebCrawlerViewVariableItemListFile,    // Submenu for File (Read, File Type, Rename, Delete)
    WebCrawlerViewMain,                    // Main view for App
    WebCrawlerViewSubmenuMain,             // Submenu Main view for App (Run, About, Config)
    WebCrawlerViewTextInput,               // Text input for Path
    WebCrawlerViewTextInputSSID,           // Text input for SSID
    WebCrawlerViewTextInputPassword,       // Text input for Password
    WebCrawlerViewFileRead,                // Text input for File Read
    WebCrawlerViewTextInputFileType,       // Text input for File Type
    WebCrawlerViewTextInputFileRename,     // Text input for File Rename
    WebCrawlerViewFileDelete,              // File Delete
} WebCrawlerViewIndex;

// Define the application structure
typedef struct
{
    ViewDispatcher *view_dispatcher;
    View *view_main;
    View *view_run;
    Submenu *submenu_main;
    Submenu *submenu_config;
    Widget *widget_about;

    TextInput *text_input_path;
    TextInput *text_input_ssid;
    TextInput *text_input_password;
    TextInput *text_input_file_type;
    TextInput *text_input_file_rename;

    Widget *widget_file_read;
    Widget *widget_file_delete;

    VariableItemList *variable_item_list_wifi;
    VariableItemList *variable_item_list_file;
    VariableItemList *variable_item_list_request;

    VariableItem *path_item;
    VariableItem *ssid_item;
    VariableItem *password_item;
    VariableItem *file_type_item;
    VariableItem *file_rename_item;
    VariableItem *file_read_item;
    VariableItem *file_delete_item;

    char *path;
    char *ssid;
    char *password;
    char *file_type;
    char *file_rename;

    char *temp_buffer_path;
    uint32_t temp_buffer_size_path;

    char *temp_buffer_ssid;
    uint32_t temp_buffer_size_ssid;

    char *temp_buffer_password;
    uint32_t temp_buffer_size_password;

    char *temp_buffer_file_type;
    uint32_t temp_buffer_size_file_type;

    char *temp_buffer_file_rename;
    uint32_t temp_buffer_size_file_rename;
} WebCrawlerApp;
#endif // WEB_CRAWLER_E