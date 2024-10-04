#ifndef WEB_CRAWLER_I_H
#define WEB_CRAWLER_I_H

/**
 * @brief      Function to allocate resources for the WebCrawlerApp.
 * @return     Pointer to the initialized WebCrawlerApp, or NULL on failure.
 */
static WebCrawlerApp *web_crawler_app_alloc()
{
    // Initialize the entire structure to zero to prevent undefined behavior
    WebCrawlerApp *app = (WebCrawlerApp *)malloc(sizeof(WebCrawlerApp));
    if (!app)
    {
        FURI_LOG_E(TAG, "Failed to allocate WebCrawlerApp");
        return NULL;
    }
    memset(app, 0, sizeof(WebCrawlerApp));

    // Open GUI
    Gui *gui = furi_record_open(RECORD_GUI);
    if (!gui)
    {
        FURI_LOG_E(TAG, "Failed to open GUI record");
        free_resources(app);
        return NULL;
    }

    // Allocate ViewDispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    if (!app->view_dispatcher)
    {
        FURI_LOG_E(TAG, "Failed to allocate ViewDispatcher");
        furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    // Attach ViewDispatcher to GUI
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    // Initialize UART with the correct callback
    if (!flipper_http_init(flipper_http_rx_callback, app))
    {
        FURI_LOG_E(TAG, "Failed to initialize UART");
        free_all(app, "Failed to initialize UART");
        return NULL;
    }

    // Allocate and initialize temp_buffer and path
    app->temp_buffer_size_path = 128;
    app->temp_buffer_path = malloc(app->temp_buffer_size_path);
    if (!app->temp_buffer_path)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_path");
        free_all(app, "Failed to allocate temp_buffer_path");
        return NULL;
    }
    app->temp_buffer_path[0] = '\0';

    // Allocate path
    app->path = malloc(app->temp_buffer_size_path);
    if (!app->path)
    {
        FURI_LOG_E(TAG, "Failed to allocate path");
        free_all(app, "Failed to allocate path");
        return NULL;
    }
    app->path[0] = '\0';

    // Allocate and initialize temp_buffer_ssid
    app->temp_buffer_size_ssid = 128;
    app->temp_buffer_ssid = malloc(app->temp_buffer_size_ssid);
    if (!app->temp_buffer_ssid)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_ssid");
        free_all(app, "Failed to allocate temp_buffer_ssid");
        return NULL;
    }
    app->temp_buffer_ssid[0] = '\0';

    // Allocate ssid
    app->ssid = malloc(app->temp_buffer_size_ssid);
    if (!app->ssid)
    {
        FURI_LOG_E(TAG, "Failed to allocate ssid");
        free_all(app, "Failed to allocate ssid");
        return NULL;
    }
    app->ssid[0] = '\0';

    // Allocate and initialize temp_buffer_password
    app->temp_buffer_size_password = 128;
    app->temp_buffer_password = malloc(app->temp_buffer_size_password);
    if (!app->temp_buffer_password)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_password");
        free_all(app, "Failed to allocate temp_buffer_password");
        return NULL;
    }
    app->temp_buffer_password[0] = '\0';

    // Allocate password
    app->password = malloc(app->temp_buffer_size_password);
    if (!app->password)
    {
        FURI_LOG_E(TAG, "Failed to allocate password");
        free_all(app, "Failed to allocate password");
        return NULL;
    }
    app->password[0] = '\0';

    // Allocate TextInput views
    app->text_input_path = text_input_alloc();
    if (!app->text_input_path)
    {
        free_all(app, "Failed to allocate TextInput for Path");
        return NULL;
    }

    app->text_input_ssid = text_input_alloc();
    if (!app->text_input_ssid)
    {
        free_all(app, "Failed to allocate TextInput for SSID");
        return NULL;
    }

    app->text_input_password = text_input_alloc();
    if (!app->text_input_password)
    {
        free_all(app, "Failed to allocate TextInput for Password");
        return NULL;
    }

    // Add TextInput views with unique view IDs
    view_dispatcher_add_view(
        app->view_dispatcher,
        WebCrawlerViewTextInput,
        text_input_get_view(app->text_input_path));
    view_dispatcher_add_view(
        app->view_dispatcher,
        WebCrawlerViewTextInputSSID,
        text_input_get_view(app->text_input_ssid));
    view_dispatcher_add_view(
        app->view_dispatcher,
        WebCrawlerViewTextInputPassword,
        text_input_get_view(app->text_input_password));

    // Set previous callback for TextInput views to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_path),
        web_crawler_back_to_configure_callback);
    view_set_previous_callback(
        text_input_get_view(app->text_input_ssid),
        web_crawler_back_to_configure_callback);
    view_set_previous_callback(
        text_input_get_view(app->text_input_password),
        web_crawler_back_to_configure_callback);

    // Allocate Configuration screen
    app->variable_item_list_config = variable_item_list_alloc();
    if (!app->variable_item_list_config)
    {
        free_all(app, "Failed to allocate VariableItemList for Configuration");
        return NULL;
    }
    variable_item_list_reset(app->variable_item_list_config);

    // Add "Path" item to the configuration screen
    app->path_item = variable_item_list_add(
        app->variable_item_list_config,
        "Path",
        1,    // Number of possible values (1 for a single text value)
        NULL, // No change callback needed
        NULL  // No context needed
    );
    if (!app->path_item)
    {
        free_all(app, "Failed to add Path item to VariableItemList");
        return NULL;
    }
    variable_item_set_current_value_text(app->path_item, ""); // Initialize

    // Add "SSID" item to the configuration screen
    app->ssid_item = variable_item_list_add(
        app->variable_item_list_config,
        "SSID",
        1,    // Number of possible values (1 for a single text value)
        NULL, // No change callback needed
        NULL  // No context needed
    );
    if (!app->ssid_item)
    {
        free_all(app, "Failed to add SSID item to VariableItemList");
        return NULL;
    }
    variable_item_set_current_value_text(app->ssid_item, ""); // Initialize

    // Add "Password" item to the configuration screen
    app->password_item = variable_item_list_add(
        app->variable_item_list_config,
        "Password",
        1,    // Number of possible values (1 for a single text value)
        NULL, // No change callback needed
        NULL  // No context needed
    );
    if (!app->password_item)
    {
        free_all(app, "Failed to add Password item to VariableItemList");
        return NULL;
    }
    variable_item_set_current_value_text(app->password_item, ""); // Initialize

    // Set a single enter callback for all configuration items
    variable_item_list_set_enter_callback(
        app->variable_item_list_config,
        web_crawler_config_enter_callback,
        app);

    // Set previous callback for configuration screen
    view_set_previous_callback(
        variable_item_list_get_view(app->variable_item_list_config),
        web_crawler_back_to_main_callback);

    // Add Configuration view to ViewDispatcher
    view_dispatcher_add_view(
        app->view_dispatcher,
        WebCrawlerViewConfigure,
        variable_item_list_get_view(app->variable_item_list_config));

    // Allocate Submenu view
    app->submenu = submenu_alloc();
    if (!app->submenu)
    {
        FURI_LOG_E(TAG, "Failed to allocate Submenu");
        free_all(app, "Failed to allocate Submenu");
        return NULL;
    }

    submenu_set_header(app->submenu, "Web Crawler v0.2");

    // Add items to Submenu
    submenu_add_item(app->submenu, "Run", WebCrawlerSubmenuIndexRun, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu, "About", WebCrawlerSubmenuIndexAbout, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu, "Configure", WebCrawlerSubmenuIndexSetPath, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu, "Read File", WebCrawlerSubmenuIndexData, web_crawler_submenu_callback, app);

    // Set previous callback for Submenu
    view_set_previous_callback(submenu_get_view(app->submenu), web_crawler_exit_app_callback);

    // Add Submenu view to ViewDispatcher
    view_dispatcher_add_view(
        app->view_dispatcher,
        WebCrawlerViewSubmenu,
        submenu_get_view(app->submenu));

    // Allocate Main view
    app->view_main = view_alloc();
    if (!app->view_main)
    {
        free_all(app, "Failed to allocate Main view");
        return NULL;
    }

    view_set_draw_callback(app->view_main, web_crawler_view_draw_callback);
    view_set_previous_callback(app->view_main, web_crawler_back_to_main_callback);

    // Add Main view to ViewDispatcher
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewRun, app->view_main);

    //-- WIDGET ABOUT VIEW --

    // Allocate and add About view
    app->widget_about = widget_alloc();
    if (!app->widget_about)
    {
        FURI_LOG_E(TAG, "Failed to allocate About widget");
        free_all(app, "Failed to allocate About widget");
        return NULL;
    }

    // Reset the widget before adding elements
    widget_reset(app->widget_about);

    widget_add_text_scroll_element(
        app->widget_about,
        0,
        0,
        128,
        64,
        "Web Crawler App\n"
        "---\n"
        "This is a web crawler app for Flipper Zero.\n"
        "---\n"
        "Visit github.com/jblanked for more details.\n"
        "---\n"
        "Press BACK to return.");

    view_set_previous_callback(widget_get_view(app->widget_about), web_crawler_back_to_main_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewAbout, widget_get_view(app->widget_about));

    //-- TEXTBOX DATA VIEW --
    app->textbox = widget_alloc();
    if (!app->textbox)
    {
        FURI_LOG_E(TAG, "Failed to allocate Textbox");
        free_all(app, "Failed to allocate Textbox");
        return NULL;
    }

    widget_reset(app->textbox);

    widget_add_text_scroll_element(
        app->textbox,
        0,
        0,
        128,
        64, "Data will be displayed here.");

    view_set_previous_callback(widget_get_view(app->textbox), web_crawler_back_to_main_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewData, widget_get_view(app->textbox));

    // Load Settings and Update Views
    if (!load_settings(app->path, app->temp_buffer_size_path, app->ssid, app->temp_buffer_size_ssid, app->password, app->temp_buffer_size_password, app))
    {
        FURI_LOG_E(TAG, "Failed to load settings");
    }
    else
    {
        // Update the configuration items based on loaded settings
        if (app->path[0] != '\0')
        {
            variable_item_set_current_value_text(app->path_item, app->path);
        }
        else
        {
            variable_item_set_current_value_text(app->path_item, ""); // Initialize
        }

        if (app->ssid[0] != '\0')
        {
            variable_item_set_current_value_text(app->ssid_item, app->ssid);
        }
        else
        {
            variable_item_set_current_value_text(app->ssid_item, ""); // Initialize
        }

        // Password handling can be omitted for security or handled securely
    }

    app_instance = app;

    // Start with the Submenu view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewSubmenu);

    return app;
}

#endif // WEB_CRAWLER_I_H
