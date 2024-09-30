// Define the submenu items for our WebCrawler application
typedef enum
{
    WebCrawlerSubmenuIndexRun,    // The main screen
    WebCrawlerSubmenuIndexAbout,  // The about screen
    WebCrawlerSubmenuIndexSetPath // The configuration screen
} WebCrawlerSubmenuIndex;

// Define views for our WebCrawler application
typedef enum
{
    WebCrawlerViewMain,              // The main screen
    WebCrawlerViewSubmenu,           // The menu when the app starts
    WebCrawlerViewAbout,             // The about screen
    WebCrawlerViewConfigure,         // The configuration screen
    WebCrawlerViewTextInput,         // Text input screen for Path
    WebCrawlerViewTextInputSSID,     // Text input screen for SSID
    WebCrawlerViewTextInputPassword, // Text input screen for Password
} WebCrawlerView;

// Define a separate model for the main view
typedef struct
{
    char path[128];     // Store the entered website path
    char ssid[128];     // Store the entered SSID
    char password[128]; // Store the entered password
} WebCrawlerMainModel;

// Define the application structure
typedef struct
{
    ViewDispatcher *view_dispatcher;             // Switches between our views
    View *view_main;                             // The main screen that displays the main content
    Submenu *submenu;                            // The application submenu
    Widget *widget_about;                        // The about screen
    TextInput *text_input_path;                  // Text input screen for Path
    TextInput *text_input_ssid;                  // Text input screen for SSID
    TextInput *text_input_password;              // Text input screen for Password
    VariableItemList *variable_item_list_config; // The configuration screen

    char *path;                  // The path to the website
    char *ssid;                  // The SSID of the WiFi network
    char *password;              // The password of the WiFi network
    VariableItem *path_item;     // Reference to the path configuration item
    VariableItem *ssid_item;     // Reference to the SSID configuration item
    VariableItem *password_item; // Reference to the password configuration item

    char *temp_buffer_path;         // Temporary buffer for text input (Path)
    uint32_t temp_buffer_size_path; // Size of the temporary buffer

    char *temp_buffer_ssid;         // Temporary buffer for text input (SSID)
    uint32_t temp_buffer_size_ssid; // Size of the temporary buffer

    char *temp_buffer_password;         // Temporary buffer for text input (Password)
    uint32_t temp_buffer_size_password; // Size of the temporary buffer
} WebCrawlerApp;