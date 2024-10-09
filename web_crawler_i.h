#ifndef WEB_CRAWLER_I_H
#define WEB_CRAWLER_I_H

/**
 * @brief      Function to allocate resources for the WebCrawlerApp.
 * @return     Pointer to the initialized WebCrawlerApp, or NULL on failure.
 */
WebCrawlerApp *web_crawler_app_alloc()
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

    // Allocate and initialize temp_buffer_file_type
    app->temp_buffer_size_file_type = 128;
    app->temp_buffer_file_type = malloc(app->temp_buffer_size_file_type);
    if (!app->temp_buffer_file_type)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_file_type");
        free_all(app, "Failed to allocate temp_buffer_file_type");
        return NULL;
    }

    // Allocate file_type
    app->file_type = malloc(app->temp_buffer_size_file_type);
    if (!app->file_type)
    {
        FURI_LOG_E(TAG, "Failed to allocate file_type");
        free_all(app, "Failed to allocate file_type");
        return NULL;
    }
    app->file_type[0] = '\0';

    // Allocate and intialize temp_buffer_file_rename
    app->temp_buffer_size_file_rename = 128;
    app->temp_buffer_file_rename = malloc(app->temp_buffer_size_file_rename);
    if (!app->temp_buffer_file_rename)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_file_rename");
        free_all(app, "Failed to allocate temp_buffer_file_rename");
        return NULL;
    }

    // Allocate file_rename
    app->file_rename = malloc(app->temp_buffer_size_file_rename);
    if (!app->file_rename)
    {
        FURI_LOG_E(TAG, "Failed to allocate file_rename");
        free_all(app, "Failed to allocate file_rename");
        return NULL;
    }
    app->file_rename[0] = '\0';

    // Allocate and initialize temp_buffer_http_method
    app->temp_buffer_size_http_method = 128;
    app->temp_buffer_http_method = malloc(app->temp_buffer_size_http_method);
    if (!app->temp_buffer_http_method)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_http_method");
        free_all(app, "Failed to allocate temp_buffer_http_method");
        return NULL;
    }

    // Allocate http_method
    app->http_method = malloc(app->temp_buffer_size_http_method);
    if (!app->http_method)
    {
        FURI_LOG_E(TAG, "Failed to allocate http_method");
        free_all(app, "Failed to allocate http_method");
        return NULL;
    }

    // Allocate and initialize temp_buffer_headers
    app->temp_buffer_size_headers = 256;
    app->temp_buffer_headers = malloc(app->temp_buffer_size_headers);
    if (!app->temp_buffer_headers)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_headers");
        free_all(app, "Failed to allocate temp_buffer_headers");
        return NULL;
    }

    // Allocate headers
    app->headers = malloc(app->temp_buffer_size_headers);
    if (!app->headers)
    {
        FURI_LOG_E(TAG, "Failed to allocate headers");
        free_all(app, "Failed to allocate headers");
        return NULL;
    }

    // Allocate and initialize temp_buffer_payload
    app->temp_buffer_size_payload = 128;
    app->temp_buffer_payload = malloc(app->temp_buffer_size_payload);
    if (!app->temp_buffer_payload)
    {
        FURI_LOG_E(TAG, "Failed to allocate temp_buffer_payload");
        free_all(app, "Failed to allocate temp_buffer_payload");
        return NULL;
    }

    // Allocate payload
    app->payload = malloc(app->temp_buffer_size_payload);
    if (!app->payload)
    {
        FURI_LOG_E(TAG, "Failed to allocate payload");
        free_all(app, "Failed to allocate payload");
        return NULL;
    }

    // Allocate TextInput views
    app->text_input_path = uart_text_input_alloc();
    app->text_input_ssid = uart_text_input_alloc();
    app->text_input_password = uart_text_input_alloc();
    app->text_input_file_type = uart_text_input_alloc();
    app->text_input_file_rename = uart_text_input_alloc();
    app->text_input_headers = uart_text_input_alloc();
    app->text_input_payload = uart_text_input_alloc();
    if (!app->text_input_path || !app->text_input_ssid || !app->text_input_password || !app->text_input_file_type || !app->text_input_file_rename || !app->text_input_headers || !app->text_input_payload)
    {
        FURI_LOG_E(TAG, "Failed to allocate TextInput");
        free_all(app, "Failed to allocate TextInput");
        return NULL;
    }

    // Add TextInput views with unique view IDs
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInput, uart_text_input_get_view(app->text_input_path));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputSSID, uart_text_input_get_view(app->text_input_ssid));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputPassword, uart_text_input_get_view(app->text_input_password));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputFileType, uart_text_input_get_view(app->text_input_file_type));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputFileRename, uart_text_input_get_view(app->text_input_file_rename));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputHeaders, uart_text_input_get_view(app->text_input_headers));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewTextInputPayload, uart_text_input_get_view(app->text_input_payload));

    // Set previous callback for TextInput views to return to Configure screen
    view_set_previous_callback(uart_text_input_get_view(app->text_input_path), web_crawler_back_to_request_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_headers), web_crawler_back_to_request_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_payload), web_crawler_back_to_request_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_ssid), web_crawler_back_to_wifi_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_password), web_crawler_back_to_wifi_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_file_type), web_crawler_back_to_file_callback);
    view_set_previous_callback(uart_text_input_get_view(app->text_input_file_rename), web_crawler_back_to_file_callback);

    // Allocate Configuration screen
    app->variable_item_list_wifi = variable_item_list_alloc();
    app->variable_item_list_file = variable_item_list_alloc();
    app->variable_item_list_request = variable_item_list_alloc();
    if (!app->variable_item_list_wifi || !app->variable_item_list_file || !app->variable_item_list_request)
    {
        FURI_LOG_E(TAG, "Failed to allocate VariableItemList");
        free_all(app, "Failed to allocate VariableItemList");
        return NULL;
    }
    variable_item_list_reset(app->variable_item_list_wifi);
    variable_item_list_reset(app->variable_item_list_file);
    variable_item_list_reset(app->variable_item_list_request);

    // Add item to the configuration screen
    app->path_item = variable_item_list_add(app->variable_item_list_request, "Path", 0, NULL, NULL);
    app->http_method_item = variable_item_list_add(app->variable_item_list_request, "HTTP Method", 4, web_crawler_http_method_change, app);
    app->headers_item = variable_item_list_add(app->variable_item_list_request, "Headers", 0, NULL, NULL);
    app->payload_item = variable_item_list_add(app->variable_item_list_request, "Payload", 0, NULL, NULL);
    //
    app->ssid_item = variable_item_list_add(app->variable_item_list_wifi, "SSID", 0, NULL, NULL);         // index 0
    app->password_item = variable_item_list_add(app->variable_item_list_wifi, "Password", 0, NULL, NULL); // index 1
    //
    app->file_read_item = variable_item_list_add(app->variable_item_list_file, "Read File", 0, NULL, NULL);     // index 0
    app->file_type_item = variable_item_list_add(app->variable_item_list_file, "Set File Type", 0, NULL, NULL); // index 1
    app->file_rename_item = variable_item_list_add(app->variable_item_list_file, "Rename File", 0, NULL, NULL); // index 2
    app->file_delete_item = variable_item_list_add(app->variable_item_list_file, "Delete File", 0, NULL, NULL); // index 3

    if (!app->ssid_item || !app->password_item || !app->file_type_item || !app->file_rename_item || !app->path_item || !app->file_read_item || !app->file_delete_item || !app->http_method_item || !app->headers_item || !app->payload_item)
    {
        free_all(app, "Failed to add items to VariableItemList");
        return NULL;
    }

    variable_item_set_current_value_text(app->path_item, "");        // Initialize
    variable_item_set_current_value_text(app->http_method_item, ""); // Initialize
    variable_item_set_current_value_text(app->headers_item, "");     // Initialize
    variable_item_set_current_value_text(app->payload_item, "");     // Initialize
    variable_item_set_current_value_text(app->ssid_item, "");        // Initialize
    variable_item_set_current_value_text(app->password_item, "");    // Initialize
    variable_item_set_current_value_text(app->file_type_item, "");   // Initialize
    variable_item_set_current_value_text(app->file_rename_item, ""); // Initialize
    variable_item_set_current_value_text(app->file_read_item, "");   // Initialize
    variable_item_set_current_value_text(app->file_delete_item, ""); // Initialize

    // Set a single callback for all items
    variable_item_list_set_enter_callback(app->variable_item_list_wifi, web_crawler_wifi_enter_callback, app);
    view_set_previous_callback(variable_item_list_get_view(app->variable_item_list_wifi), web_crawler_back_to_configure_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi, variable_item_list_get_view(app->variable_item_list_wifi));

    variable_item_list_set_enter_callback(app->variable_item_list_file, web_crawler_file_enter_callback, app);
    view_set_previous_callback(variable_item_list_get_view(app->variable_item_list_file), web_crawler_back_to_configure_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile, variable_item_list_get_view(app->variable_item_list_file));

    variable_item_list_set_enter_callback(app->variable_item_list_request, web_crawler_request_enter_callback, app);
    view_set_previous_callback(variable_item_list_get_view(app->variable_item_list_request), web_crawler_back_to_configure_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest, variable_item_list_get_view(app->variable_item_list_request));
    //------------------------------//
    //        SUBMENU VIEW          //
    //------------------------------//

    // Allocate
    app->submenu_main = submenu_alloc();
    app->submenu_config = submenu_alloc();
    if (!app->submenu_main || !app->submenu_config)
    {
        FURI_LOG_E(TAG, "Failed to allocate Submenu");
        free_all(app, "Failed to allocate Submenu");
        return NULL;
    }

    // Set header
    submenu_set_header(app->submenu_main, "Web Crawler v0.4");
    submenu_set_header(app->submenu_config, "Settings");

    // Add items
    submenu_add_item(app->submenu_main, "Run", WebCrawlerSubmenuIndexRun, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu_main, "About", WebCrawlerSubmenuIndexAbout, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu_main, "Settings", WebCrawlerSubmenuIndexConfig, web_crawler_submenu_callback, app);

    submenu_add_item(app->submenu_config, "WiFi", WebCrawlerSubmenuIndexWifi, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu_config, "File", WebCrawlerSubmenuIndexFile, web_crawler_submenu_callback, app);
    submenu_add_item(app->submenu_config, "Request", WebCrawlerSubmenuIndexRequest, web_crawler_submenu_callback, app);

    // Set previous callback for Submenu
    view_set_previous_callback(submenu_get_view(app->submenu_main), web_crawler_exit_app_callback);       // Exit App
    view_set_previous_callback(submenu_get_view(app->submenu_config), web_crawler_back_to_main_callback); // Back to Main

    // Add Submenu view to ViewDispatcher
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewSubmenuMain, submenu_get_view(app->submenu_main));
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewSubmenuConfig, submenu_get_view(app->submenu_config));

    //---------------------------------------------------------->

    // Allocate Main view
    app->view_main = view_alloc();
    app->view_run = view_alloc();
    if (!app->view_main || !app->view_run)
    {
        free_all(app, "Failed to allocate Views");
        return NULL;
    }

    // view_set_draw_callback(app->view_main, web_crawler_view_draw_callback);
    view_set_previous_callback(app->view_main, web_crawler_exit_app_callback);

    view_set_draw_callback(app->view_run, web_crawler_view_draw_callback);
    view_set_previous_callback(app->view_run, web_crawler_back_to_main_callback);

    // Add Main view to ViewDispatcher
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewMain, app->view_main);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewRun, app->view_run);

    //-- WIDGET ABOUT VIEW --

    // Allocate and add About view
    app->widget_about = widget_alloc();
    app->widget_file_read = widget_alloc();
    app->widget_file_delete = widget_alloc();
    if (!app->widget_about || !app->widget_file_read || !app->widget_file_delete)
    {
        FURI_LOG_E(TAG, "Failed to allocate Widget");
        free_all(app, "Failed to allocate Widget");
        return NULL;
    }

    // Reset the widget before adding elements
    widget_reset(app->widget_about);
    widget_reset(app->widget_file_read);
    widget_reset(app->widget_file_delete);

    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, "Web Crawler App\n"
                                                                     "---\n"
                                                                     "This is a web crawler app for Flipper Zero.\n"
                                                                     "---\n"
                                                                     "Visit github.com/jblanked for more details.\n"
                                                                     "---\n"
                                                                     "Press BACK to return.");

    widget_add_text_scroll_element(app->widget_file_read, 0, 0, 128, 64, "Data will be displayed here.");
    widget_add_text_scroll_element(app->widget_file_delete, 0, 0, 128, 64, "File deleted.");

    view_set_previous_callback(widget_get_view(app->widget_about), web_crawler_back_to_main_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewAbout, widget_get_view(app->widget_about));

    view_set_previous_callback(widget_get_view(app->widget_file_read), web_crawler_back_to_file_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewFileRead, widget_get_view(app->widget_file_read));

    view_set_previous_callback(widget_get_view(app->widget_file_delete), web_crawler_back_to_file_callback);
    view_dispatcher_add_view(app->view_dispatcher, WebCrawlerViewFileDelete, widget_get_view(app->widget_file_delete));

    // Load Settings and Update Views
    if (!load_settings(
            app->path,
            app->temp_buffer_size_path,
            app->ssid,
            app->temp_buffer_size_ssid,
            app->password,
            app->temp_buffer_size_password,
            app->file_rename,
            app->temp_buffer_size_file_rename,
            app->file_type,
            app->temp_buffer_size_file_type,
            app->http_method,
            app->temp_buffer_size_http_method,
            app->headers,
            app->temp_buffer_size_headers,
            app->payload,
            app->temp_buffer_size_payload,
            app))
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

        if (app->file_type[0] != '\0')
        {
            variable_item_set_current_value_text(app->file_type_item, app->file_type);
        }
        else
        {
            variable_item_set_current_value_text(app->file_type_item, ".txt"); // Initialize
        }

        if (app->file_rename[0] != '\0')
        {
            variable_item_set_current_value_text(app->file_rename_item, app->file_rename);
        }
        else
        {
            variable_item_set_current_value_text(app->file_rename_item, "received_data"); // Initialize
        }

        if (app->http_method[0] != '\0')
        {
            variable_item_set_current_value_text(app->http_method_item, app->http_method);
        }
        else
        {
            variable_item_set_current_value_text(app->http_method_item, "GET"); // Initialize
        }

        if (app->headers[0] != '\0')
        {
            variable_item_set_current_value_text(app->headers_item, app->headers);
        }
        else
        {
            variable_item_set_current_value_text(app->headers_item, "{\n\t\"Content-Type\": \"application/json\"\n}"); // Initialize
        }

        if (app->payload[0] != '\0')
        {
            variable_item_set_current_value_text(app->payload_item, app->payload);
        }
        else
        {
            variable_item_set_current_value_text(app->payload_item, "{\n\t\"key\": \"value\"\n}"); // Initialize
        }

        // Password handling can be omitted for security or handled securely
    }

    app_instance = app;

    // Start with the Submenu view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewSubmenuMain);

    return app;
}

#endif // WEB_CRAWLER_I_H
