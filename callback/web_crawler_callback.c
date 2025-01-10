#include <callback/web_crawler_callback.h>

// Below added by Derek Jamison
// FURI_LOG_DEV will log only during app development. Be sure that Settings/System/Log Device is "LPUART"; so we dont use serial port.
#ifdef DEVELOPMENT
#define FURI_LOG_DEV(tag, format, ...) furi_log_print_format(FuriLogLevelInfo, tag, format, ##__VA_ARGS__)
#define DEV_CRASH() furi_crash()
#else
#define FURI_LOG_DEV(tag, format, ...)
#define DEV_CRASH()
#endif

static void web_crawler_draw_error(Canvas *canvas, DataLoaderModel *model)
{
    furi_check(model, "DataLoaderModel is NULL");
    furi_check(model->fhttp, "FlipperHTTP is NULL");
    furi_check(canvas, "Canvas is NULL");
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    if (model->fhttp->state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    if (model->fhttp->last_response)
    {
        if (strstr(model->fhttp->last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
        {
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }
        if (strstr(model->fhttp->last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
        {
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }
        if (strstr(model->fhttp->last_response, "[ERROR] GET request failed with error: connection refused") != NULL)
        {
            canvas_draw_str(canvas, 0, 10, "[ERROR] Connection refused.");
            canvas_draw_str(canvas, 0, 50, "Choose another URL.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }
        if (strstr(model->fhttp->last_response, "[PONG]") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[STATUS]Connecting to AP...");
            return;
        }

        canvas_draw_str(canvas, 0, 10, "[ERROR] Failed to sync data.");
        canvas_draw_str(canvas, 0, 30, "If this is your third attempt,");
        canvas_draw_str(canvas, 0, 40, "it's likely your URL is not");
        canvas_draw_str(canvas, 0, 50, "compabilbe or correct.");
        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        return;
    }

    canvas_draw_str(canvas, 0, 10, "HTTP request failed.");
    canvas_draw_str(canvas, 0, 20, "Press BACK to return.");
}

void web_crawler_http_method_change(VariableItem *item)
{
    WebCrawlerApp *app = (WebCrawlerApp *)variable_item_get_context(item);
    furi_check(app, "WebCrawlerApp is NULL");
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, http_method_names[index]);
    variable_item_set_current_value_index(item, index);

    // save the http method
    if (app)
    {
        strncpy(app->http_method, http_method_names[index], strlen(http_method_names[index]) + 1);

        // save the settings
        save_settings(
            app->path,
            app->ssid,
            app->password,
            app->file_rename,
            app->file_type,
            app->http_method,
            app->headers,
            app->payload);
    }
}

static bool web_crawler_fetch(DataLoaderModel *model)
{
    WebCrawlerApp *app = (WebCrawlerApp *)model->parser_context;
    furi_check(app, "WebCrawlerApp is NULL");
    furi_check(model->fhttp, "FlipperHTTP is NULL");
    if (app->file_type && app->file_rename)
    {
        snprintf(
            model->fhttp->file_path,
            sizeof(model->fhttp->file_path),
            STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler/%s%s",
            app->file_rename,
            app->file_type);
    }
    else
    {
        snprintf(
            model->fhttp->file_path,
            sizeof(model->fhttp->file_path),
            STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler/received_data.txt");
    }

    if (strstr(app->http_method, "GET") != NULL)
    {
        model->fhttp->save_received_data = true;
        model->fhttp->is_bytes_request = false;

        // Perform GET request and handle the response
        if (app->headers == NULL || app->headers[0] == '\0' || strstr(app->headers, " ") == NULL)
        {
            return flipper_http_get_request(model->fhttp, app->path);
        }
        else
        {
            return flipper_http_get_request_with_headers(model->fhttp, app->path, app->headers);
        }
    }
    else if (strstr(app->http_method, "POST") != NULL)
    {
        model->fhttp->save_received_data = true;
        model->fhttp->is_bytes_request = false;

        // Perform POST request and handle the response
        return flipper_http_post_request_with_headers(model->fhttp, app->path, app->headers, app->payload);
    }
    else if (strstr(app->http_method, "PUT") != NULL)
    {
        model->fhttp->save_received_data = true;
        model->fhttp->is_bytes_request = false;

        // Perform PUT request and handle the response
        return flipper_http_put_request_with_headers(model->fhttp, app->path, app->headers, app->payload);
    }
    else if (strstr(app->http_method, "DELETE") != NULL)
    {
        model->fhttp->save_received_data = true;
        model->fhttp->is_bytes_request = false;

        // Perform DELETE request and handle the response
        return flipper_http_delete_request_with_headers(model->fhttp, app->path, app->headers, app->payload);
    }
    else
    {
        model->fhttp->save_received_data = false;
        model->fhttp->is_bytes_request = true;

        // Perform GET request and handle the response
        return flipper_http_get_request_bytes(model->fhttp, app->path, app->headers);
    }
    return false;
}

static char *web_crawler_parse(DataLoaderModel *model)
{
    UNUSED(model);
    // there is no parsing since everything is saved to file
    return "Data saved to file.\nPress BACK to return.";
}

static void web_crawler_data_switch_to_view(WebCrawlerApp *app)
{
    furi_check(app, "WebCrawlerApp is NULL");
    char *title = "GET Request";
    if (strstr(app->http_method, "GET") != NULL)
    {
        title = "GET Request";
    }
    else if (strstr(app->http_method, "POST") != NULL)
    {
        title = "POST Request";
    }
    else if (strstr(app->http_method, "PUT") != NULL)
    {
        title = "PUT Request";
    }
    else if (strstr(app->http_method, "DELETE") != NULL)
    {
        title = "DELETE Request";
    }
    else
    {
        title = "File Download";
    }
    web_crawler_generic_switch_to_view(app, title, web_crawler_fetch, web_crawler_parse, 1, web_crawler_back_to_main_callback, WebCrawlerViewLoader);
}

/**
 * @brief      Navigation callback to handle exiting from other views to the submenu.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
uint32_t web_crawler_back_to_configure_callback(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    return WebCrawlerViewSubmenuConfig; // Return to the configure screen
}

/**
 * @brief      Navigation callback to handle returning to the Wifi Settings screen.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
uint32_t web_crawler_back_to_main_callback(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    return WebCrawlerViewSubmenuMain; // Return to the main submenu
}

uint32_t web_crawler_back_to_file_callback(void *context)
{
    UNUSED(context);
    return WebCrawlerViewVariableItemListFile; // Return to the file submenu
}

uint32_t web_crawler_back_to_wifi_callback(void *context)
{
    UNUSED(context);
    return WebCrawlerViewVariableItemListWifi; // Return to the wifi submenu
}

uint32_t web_crawler_back_to_request_callback(void *context)
{
    UNUSED(context);
    return WebCrawlerViewVariableItemListRequest; // Return to the request submenu
}

/**
 * @brief      Navigation callback to handle exiting the app from the main submenu.
 * @param      context   The context - unused
 * @return     VIEW_NONE to exit the app
 */
uint32_t web_crawler_exit_app_callback(void *context)
{
    UNUSED(context);
    return VIEW_NONE;
}

/**
 * @brief      Handle submenu item selection.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The WebCrawlerSubmenuIndex item that was clicked.
 */
void web_crawler_submenu_callback(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (app->view_dispatcher)
    {
        switch (index)
        {
        case WebCrawlerSubmenuIndexRun:
            web_crawler_data_switch_to_view(app);
            break;
        case WebCrawlerSubmenuIndexAbout:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewAbout);
            break;
        case WebCrawlerSubmenuIndexConfig:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewSubmenuConfig);
            break;
        case WebCrawlerSubmenuIndexWifi:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
            break;
        case WebCrawlerSubmenuIndexRequest:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
            break;
        case WebCrawlerSubmenuIndexFile:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile);
            break;
        default:
            FURI_LOG_E(TAG, "Unknown submenu index");
            break;
        }
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
void web_crawler_wifi_enter_callback(void *context, uint32_t index)
{
    switch (index)
    {
    case 0: // SSID
        web_crawler_setting_item_ssid_clicked(context, index);
        break;
    case 1: // Password
        web_crawler_setting_item_password_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
void web_crawler_file_enter_callback(void *context, uint32_t index)
{
    switch (index)
    {
    case 0: // File Read
        web_crawler_setting_item_file_read_clicked(context, index);
        break;
    case 1: // FIle Type
        web_crawler_setting_item_file_type_clicked(context, index);
        break;
    case 2: // File Rename
        web_crawler_setting_item_file_rename_clicked(context, index);
        break;
    case 3: // File Delete
        web_crawler_setting_item_file_delete_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
void web_crawler_request_enter_callback(void *context, uint32_t index)
{
    switch (index)
    {
    case 0: // URL
        web_crawler_setting_item_path_clicked(context, index);
        break;
    case 1:
        // HTTP Method
        break;
    case 2:
        // Headers
        web_crawler_setting_item_headers_clicked(context, index);
        break;
    case 3:
        // Payload
        web_crawler_setting_item_payload_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Callback for when the user finishes entering the URL.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_path_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->path || !app->temp_buffer_path || !app->temp_buffer_size_path || !app->path_item)
    {
        FURI_LOG_E(TAG, "Invalid path buffer");
        return;
    }
    // Store the entered URL from temp_buffer_path to path
    strncpy(app->path, app->temp_buffer_path, app->temp_buffer_size_path - 1);

    if (app->path_item)
    {
        variable_item_set_current_value_text(app->path_item, app->path);

        // Save the URL to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
}

/**
 * @brief      Callback for when the user finishes entering the headers
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_headers_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->temp_buffer_headers || !app->temp_buffer_size_headers || !app->headers_item)
    {
        FURI_LOG_E(TAG, "Invalid headers buffer");
        return;
    }
    // Store the entered headers from temp_buffer_headers to headers
    strncpy(app->headers, app->temp_buffer_headers, app->temp_buffer_size_headers - 1);

    if (app->headers_item)
    {
        variable_item_set_current_value_text(app->headers_item, app->headers);

        // Save the headers to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
}

/**
 * @brief      Callback for when the user finishes entering the payload.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_payload_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->temp_buffer_payload || !app->temp_buffer_size_payload || !app->payload_item)
    {
        FURI_LOG_E(TAG, "Invalid payload buffer");
        return;
    }
    // Store the entered payload from temp_buffer_payload to payload
    strncpy(app->payload, app->temp_buffer_payload, app->temp_buffer_size_payload - 1);

    if (app->payload_item)
    {
        variable_item_set_current_value_text(app->payload_item, app->payload);

        // Save the payload to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
}

/**
 * @brief      Callback for when the user finishes entering the SSID.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_ssid_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    FlipperHTTP *fhttp = flipper_http_alloc();
    furi_check(fhttp, "FlipperHTTP is NULL");
    if (!app->temp_buffer_ssid || !app->temp_buffer_size_ssid || !app->ssid || !app->ssid_item)
    {
        FURI_LOG_E(TAG, "Invalid SSID buffer");
        return;
    }
    // Store the entered SSID from temp_buffer_ssid to ssid
    strncpy(app->ssid, app->temp_buffer_ssid, app->temp_buffer_size_ssid - 1);

    if (app->ssid_item)
    {
        variable_item_set_current_value_text(app->ssid_item, app->ssid);

        // Save the SSID to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);

        // send to UART
        if (!flipper_http_save_wifi(fhttp, app->ssid, app->password))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
    }
    flipper_http_free(fhttp);
    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
}

/**
 * @brief      Callback for when the user finishes entering the Password.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_password_update(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    FlipperHTTP *fhttp = flipper_http_alloc();
    furi_check(fhttp, "FlipperHTTP is NULL");
    if (!app->temp_buffer_password || !app->temp_buffer_size_password || !app->password || !app->password_item)
    {
        FURI_LOG_E(TAG, "Invalid password buffer");
        return;
    }
    // Store the entered Password from temp_buffer_password to password
    strncpy(app->password, app->temp_buffer_password, app->temp_buffer_size_password - 1);

    if (app->password_item)
    {
        variable_item_set_current_value_text(app->password_item, app->password);

        // Save the Password to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);

        // send to UART
        if (!flipper_http_save_wifi(fhttp, app->ssid, app->password))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
    }
    flipper_http_free(fhttp);
    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
}

/**
 * @brief      Callback for when the user finishes entering the File Type.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_file_type_update(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->temp_buffer_file_type || !app->temp_buffer_size_file_type || !app->file_type || !app->file_type_item)
    {
        FURI_LOG_E(TAG, "Invalid file type buffer");
        return;
    }
    // Temporary buffer to store the old name
    char old_file_type[256];

    strncpy(old_file_type, app->file_type, sizeof(old_file_type) - 1);
    old_file_type[sizeof(old_file_type) - 1] = '\0'; // Null-terminate
    strncpy(app->file_type, app->temp_buffer_file_type, app->temp_buffer_size_file_type - 1);

    if (app->file_type_item)
    {
        variable_item_set_current_value_text(app->file_type_item, app->file_type);

        // Save the File Type to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);
    }

    rename_received_data(app->file_rename, app->file_rename, app->file_type, old_file_type);

    // set the file path for fhttp->file_path
    if (app->file_rename && app->file_type)
    {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s%s%s", RECEIVED_DATA_PATH, app->file_rename, app->file_type);
        file_path[sizeof(file_path) - 1] = '\0'; // Null-terminate
        // strncpy(fhttp->file_path, file_path, sizeof(fhttp->file_path) - 1);
        // fhttp->file_path[sizeof(fhttp->file_path) - 1] = '\0'; // Null-terminate
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile);
}

/**
 * @brief      Callback for when the user finishes entering the File Rename.
 * @param      context   The context - WebCrawlerApp object.
 */
void web_crawler_set_file_rename_update(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->temp_buffer_file_rename || !app->temp_buffer_size_file_rename || !app->file_rename || !app->file_rename_item)
    {
        FURI_LOG_E(TAG, "Invalid file rename buffer");
        return;
    }

    // Temporary buffer to store the old name
    char old_name[256];

    // Ensure that app->file_rename is null-terminated
    strncpy(old_name, app->file_rename, sizeof(old_name) - 1);
    old_name[sizeof(old_name) - 1] = '\0'; // Null-terminate

    // Store the entered File Rename from temp_buffer_file_rename to file_rename
    strncpy(app->file_rename, app->temp_buffer_file_rename, app->temp_buffer_size_file_rename - 1);

    if (app->file_rename_item)
    {
        variable_item_set_current_value_text(app->file_rename_item, app->file_rename);

        // Save the File Rename to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type, app->http_method, app->headers, app->payload);
    }

    rename_received_data(old_name, app->file_rename, app->file_type, app->file_type);

    // set the file path for fhttp->file_path
    if (app->file_rename && app->file_type)
    {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s%s%s", RECEIVED_DATA_PATH, app->file_rename, app->file_type);
        file_path[sizeof(file_path) - 1] = '\0'; // Null-terminate
        // strncpy(fhttp->file_path, file_path, sizeof(fhttp->file_path) - 1);
        // fhttp->file_path[sizeof(fhttp->file_path) - 1] = '\0'; // Null-terminate
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile);
}

/**
 * @brief      Handler for Path configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_path_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    if (!app->text_input_path)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    UNUSED(index);

    // Initialize temp_buffer with existing path
    if (app->path && strlen(app->path) > 0)
    {
        strncpy(app->temp_buffer_path, app->path, app->temp_buffer_size_path - 1);
    }
    else
    {
        strncpy(app->temp_buffer_path, "https://httpbin.org/get", app->temp_buffer_size_path - 1);
    }

    app->temp_buffer_path[app->temp_buffer_size_path - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_path,
        web_crawler_set_path_updated,
        app,
        app->temp_buffer_path,
        app->temp_buffer_size_path,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_path),
        web_crawler_back_to_request_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInput);
}

/**
 * @brief      Handler for headers configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_headers_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_headers)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }
    if (!app->headers)
    {
        FURI_LOG_E(TAG, "Headers is NULL");
        return;
    }
    if (!app->temp_buffer_headers)
    {
        FURI_LOG_E(TAG, "Temp buffer headers is NULL");
        return;
    }

    // Initialize temp_buffer with existing headers
    if (app->headers && strlen(app->headers) > 0)
    {
        strncpy(app->temp_buffer_headers, app->headers, app->temp_buffer_size_headers - 1);
    }
    else
    {
        strncpy(app->temp_buffer_headers, "{\"Content-Type\":\"application/json\",\"key\":\"value\"}", app->temp_buffer_size_headers - 1);
    }

    app->temp_buffer_headers[app->temp_buffer_size_headers - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_headers,
        web_crawler_set_headers_updated,
        app,
        app->temp_buffer_headers,
        app->temp_buffer_size_headers,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_headers),
        web_crawler_back_to_request_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputHeaders);
}

/**
 * @brief      Handler for payload configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_payload_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_payload)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    // Initialize temp_buffer with existing payload
    if (app->payload && strlen(app->payload) > 0)
    {
        strncpy(app->temp_buffer_payload, app->payload, app->temp_buffer_size_payload - 1);
    }
    else
    {
        strncpy(app->temp_buffer_payload, "{\"key\":\"value\"}", app->temp_buffer_size_payload - 1);
    }

    app->temp_buffer_payload[app->temp_buffer_size_payload - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_payload,
        web_crawler_set_payload_updated,
        app,
        app->temp_buffer_payload,
        app->temp_buffer_size_payload,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_payload),
        web_crawler_back_to_request_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputPayload);
}

/**
 * @brief      Handler for SSID configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_ssid_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_ssid)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    // Initialize temp_buffer with existing SSID
    if (app->ssid && strlen(app->ssid) > 0)
    {
        strncpy(app->temp_buffer_ssid, app->ssid, app->temp_buffer_size_ssid - 1);
    }
    else
    {
        strncpy(app->temp_buffer_ssid, "", app->temp_buffer_size_ssid - 1);
    }

    app->temp_buffer_ssid[app->temp_buffer_size_ssid - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_ssid,
        web_crawler_set_ssid_updated,
        app,
        app->temp_buffer_ssid,
        app->temp_buffer_size_ssid,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_ssid),
        web_crawler_back_to_wifi_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputSSID);
}

/**
 * @brief      Handler for Password configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_password_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_password)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    // Initialize temp_buffer with existing password
    strncpy(app->temp_buffer_password, app->password, app->temp_buffer_size_password - 1);
    app->temp_buffer_password[app->temp_buffer_size_password - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_password,
        web_crawler_set_password_update,
        app,
        app->temp_buffer_password,
        app->temp_buffer_size_password,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_password),
        web_crawler_back_to_wifi_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputPassword);
}

/**
 * @brief      Handler for File Type configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_file_type_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_file_type)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    // Initialize temp_buffer with existing file_type
    if (app->file_type && strlen(app->file_type) > 0)
    {
        strncpy(app->temp_buffer_file_type, app->file_type, app->temp_buffer_size_file_type - 1);
    }
    else
    {
        strncpy(app->temp_buffer_file_type, ".txt", app->temp_buffer_size_file_type - 1);
    }

    app->temp_buffer_file_type[app->temp_buffer_size_file_type - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_file_type,
        web_crawler_set_file_type_update,
        app,
        app->temp_buffer_file_type,
        app->temp_buffer_size_file_type,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_file_type),
        web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputFileType);
}

/**
 * @brief      Handler for File Rename configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_file_rename_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    if (!app->text_input_file_rename)
    {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    // Initialize temp_buffer with existing file_rename
    if (app->file_rename && strlen(app->file_rename) > 0)
    {
        strncpy(app->temp_buffer_file_rename, app->file_rename, app->temp_buffer_size_file_rename - 1);
    }
    else
    {
        strncpy(app->temp_buffer_file_rename, "received_data", app->temp_buffer_size_file_rename - 1);
    }

    app->temp_buffer_file_rename[app->temp_buffer_size_file_rename - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    uart_text_input_set_result_callback(
        app->text_input_file_rename,
        web_crawler_set_file_rename_update,
        app,
        app->temp_buffer_file_rename,
        app->temp_buffer_size_file_rename,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        uart_text_input_get_view(app->text_input_file_rename),
        web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputFileRename);
}

/**
 * @brief      Handler for File Delete configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
void web_crawler_setting_item_file_delete_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);

    if (!delete_received_data(app))
    {
        FURI_LOG_E(TAG, "Failed to delete file");
    }

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        widget_get_view(app->widget_file_delete),
        web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewFileDelete);
}

void web_crawler_setting_item_file_read_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app, "WebCrawlerApp is NULL");
    UNUSED(index);
    widget_reset(app->widget_file_read);
    char file_path[256];
    if (app->file_rename && app->file_type)
    {
        snprintf(file_path, sizeof(file_path), "%s%s%s", RECEIVED_DATA_PATH, app->file_rename, app->file_type);
    }
    else
    {
        snprintf(file_path, sizeof(file_path), "%s%s%s", RECEIVED_DATA_PATH, "received_data", ".txt");
    }

    // load the received data from the saved file
    FuriString *received_data = flipper_http_load_from_file(file_path);
    if (received_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        if (app->widget_file_read)
        {
            widget_add_text_scroll_element(
                app->widget_file_read,
                0,
                0,
                128,
                64, "File is empty.");
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewFileRead);
        }
        return;
    }
    widget_add_text_scroll_element(app->widget_file_read, 0, 0, 128, 64, furi_string_get_cstr(received_data));
    furi_string_free(received_data);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        widget_get_view(app->widget_file_read),
        web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewFileRead);
}

static void web_crawler_widget_set_text(char *message, Widget **widget)
{
    if (widget == NULL)
    {
        FURI_LOG_E(TAG, "flip_weather_set_widget_text - widget is NULL");
        DEV_CRASH();
        return;
    }
    if (message == NULL)
    {
        FURI_LOG_E(TAG, "flip_weather_set_widget_text - message is NULL");
        DEV_CRASH();
        return;
    }
    widget_reset(*widget);

    uint32_t message_length = strlen(message); // Length of the message
    uint32_t i = 0;                            // Index tracker
    uint32_t formatted_index = 0;              // Tracker for where we are in the formatted message
    char *formatted_message;                   // Buffer to hold the final formatted message

    // Allocate buffer with double the message length plus one for safety
    if (!easy_flipper_set_buffer(&formatted_message, message_length * 2 + 1))
    {
        return;
    }

    while (i < message_length)
    {
        uint32_t max_line_length = 31;                  // Maximum characters per line
        uint32_t remaining_length = message_length - i; // Remaining characters
        uint32_t line_length = (remaining_length < max_line_length) ? remaining_length : max_line_length;

        // Check for newline character within the current segment
        uint32_t newline_pos = i;
        bool found_newline = false;
        for (; newline_pos < i + line_length && newline_pos < message_length; newline_pos++)
        {
            if (message[newline_pos] == '\n')
            {
                found_newline = true;
                break;
            }
        }

        if (found_newline)
        {
            // If newline found, set line_length up to the newline
            line_length = newline_pos - i;
        }

        // Temporary buffer to hold the current line
        char line[32];
        strncpy(line, message + i, line_length);
        line[line_length] = '\0';

        // If newline was found, skip it for the next iteration
        if (found_newline)
        {
            i += line_length + 1; // +1 to skip the '\n' character
        }
        else
        {
            // Check if the line ends in the middle of a word and adjust accordingly
            if (line_length == max_line_length && message[i + line_length] != '\0' && message[i + line_length] != ' ')
            {
                // Find the last space within the current line to avoid breaking a word
                char *last_space = strrchr(line, ' ');
                if (last_space != NULL)
                {
                    // Adjust the line_length to avoid cutting the word
                    line_length = last_space - line;
                    line[line_length] = '\0'; // Null-terminate at the space
                }
            }

            // Move the index forward by the determined line_length
            i += line_length;

            // Skip any spaces at the beginning of the next line
            while (i < message_length && message[i] == ' ')
            {
                i++;
            }
        }

        // Manually copy the fixed line into the formatted_message buffer
        for (uint32_t j = 0; j < line_length; j++)
        {
            formatted_message[formatted_index++] = line[j];
        }

        // Add a newline character for line spacing
        formatted_message[formatted_index++] = '\n';
    }

    // Null-terminate the formatted_message
    formatted_message[formatted_index] = '\0';

    // Add the formatted message to the widget
    widget_add_text_scroll_element(*widget, 0, 0, 128, 64, formatted_message);
}

void web_crawler_loader_draw_callback(Canvas *canvas, void *model)
{
    if (!canvas || !model)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_draw_callback - canvas or model is NULL");
        return;
    }

    DataLoaderModel *data_loader_model = (DataLoaderModel *)model;
    SerialState http_state = data_loader_model->fhttp->state;
    DataState data_state = data_loader_model->data_state;
    char *title = data_loader_model->title;

    canvas_set_font(canvas, FontSecondary);

    if (http_state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    if (data_state == DataStateError || data_state == DataStateParseError)
    {
        web_crawler_draw_error(canvas, data_loader_model);
        return;
    }

    canvas_draw_str(canvas, 0, 7, title);
    canvas_draw_str(canvas, 0, 17, "Loading...");

    if (data_state == DataStateInitial)
    {
        return;
    }

    if (http_state == SENDING)
    {
        canvas_draw_str(canvas, 0, 27, "Fetching...");
        return;
    }

    if (http_state == RECEIVING || data_state == DataStateRequested)
    {
        canvas_draw_str(canvas, 0, 27, "Receiving...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateReceived)
    {
        canvas_draw_str(canvas, 0, 27, "Processing...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateParsed)
    {
        canvas_draw_str(canvas, 0, 27, "Processed...");
        return;
    }
}

static void web_crawler_loader_process_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_process_callback - context is NULL");
        DEV_CRASH();
        return;
    }

    WebCrawlerApp *app = (WebCrawlerApp *)context;
    View *view = app->view_loader;

    DataState current_data_state;
    DataLoaderModel *loader_model = NULL;
    with_view_model(view, DataLoaderModel * model, { current_data_state = model->data_state; loader_model = model; }, false);
    if (!loader_model || !loader_model->fhttp)
    {
        FURI_LOG_E(TAG, "Model or fhttp is NULL");
        DEV_CRASH();
        return;
    }

    if (current_data_state == DataStateInitial)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                model->data_state = DataStateRequested;
                DataLoaderFetch fetch = model->fetcher;
                if (fetch == NULL)
                {
                    FURI_LOG_E(TAG, "Model doesn't have Fetch function assigned.");
                    model->data_state = DataStateError;
                    return;
                }

                // Clear any previous responses
                strncpy(model->fhttp->last_response, "", 1);
                bool request_status = fetch(model);
                if (!request_status)
                {
                    model->data_state = DataStateError;
                }
            },
            true);
    }
    else if (current_data_state == DataStateRequested || current_data_state == DataStateError)
    {
        if (loader_model->fhttp->state == IDLE && loader_model->fhttp->last_response != NULL)
        {
            if (strstr(loader_model->fhttp->last_response, "[PONG]") != NULL)
            {
                FURI_LOG_DEV(TAG, "PONG received.");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[SUCCESS]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "SUCCESS received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[ERROR]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "ERROR received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strlen(loader_model->fhttp->last_response) == 0)
            {
                // Still waiting on response
            }
            else
            {
                with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateReceived; }, true);
            }
        }
        else if (loader_model->fhttp->state == SENDING || loader_model->fhttp->state == RECEIVING)
        {
            // continue waiting
        }
        else if (loader_model->fhttp->state == INACTIVE)
        {
            // inactive. try again
        }
        else if (loader_model->fhttp->state == ISSUE)
        {
            with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateError; }, true);
        }
        else
        {
            FURI_LOG_DEV(TAG, "Unexpected state: %d lastresp: %s", loader_model->fhttp->state, loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            DEV_CRASH();
        }
    }
    else if (current_data_state == DataStateReceived)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                char *data_text;
                if (model->parser == NULL)
                {
                    data_text = NULL;
                    FURI_LOG_DEV(TAG, "Parser is NULL");
                    DEV_CRASH();
                }
                else
                {
                    data_text = model->parser(model);
                }
                FURI_LOG_DEV(TAG, "Parsed data: %s\r\ntext: %s", model->fhttp->last_response ? model->fhttp->last_response : "NULL", data_text ? data_text : "NULL");
                model->data_text = data_text;
                if (data_text == NULL)
                {
                    model->data_state = DataStateParseError;
                }
                else
                {
                    model->data_state = DataStateParsed;
                }
            },
            true);
    }
    else if (current_data_state == DataStateParsed)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                if (++model->request_index < model->request_count)
                {
                    model->data_state = DataStateInitial;
                }
                else
                {
                    web_crawler_widget_set_text(model->data_text != NULL ? model->data_text : "", &app->widget_result);
                    if (model->data_text != NULL)
                    {
                        free(model->data_text);
                        model->data_text = NULL;
                    }
                    view_set_previous_callback(widget_get_view(app->widget_result), model->back_callback);
                    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewWidgetResult);
                }
            },
            true);
    }
}

static void web_crawler_loader_timer_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_timer_callback - context is NULL");
        DEV_CRASH();
        return;
    }
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    view_dispatcher_send_custom_event(app->view_dispatcher, WebCrawlerCustomEventProcess);
}

static void web_crawler_loader_on_enter(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_on_enter - context is NULL");
        DEV_CRASH();
        return;
    }
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            view_set_previous_callback(view, model->back_callback);
            if (model->timer == NULL)
            {
                model->timer = furi_timer_alloc(web_crawler_loader_timer_callback, FuriTimerTypePeriodic, app);
            }
            furi_timer_start(model->timer, 250);
        },
        true);
}

static void web_crawler_loader_on_exit(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_on_exit - context is NULL");
        DEV_CRASH();
        return;
    }
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_stop(model->timer);
            }
        },
        false);
}

void web_crawler_loader_init(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_init - view is NULL");
        DEV_CRASH();
        return;
    }
    view_allocate_model(view, ViewModelTypeLocking, sizeof(DataLoaderModel));
    view_set_enter_callback(view, web_crawler_loader_on_enter);
    view_set_exit_callback(view, web_crawler_loader_on_exit);
}

void web_crawler_loader_free_model(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_loader_free_model - view is NULL");
        DEV_CRASH();
        return;
    }
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_free(model->timer);
                model->timer = NULL;
            }
            if (model->parser_context)
            {
                // do not free the context here, it is the app context
                // free(model->parser_context);
                // model->parser_context = NULL;
            }
            if (model->fhttp)
            {
                flipper_http_free(model->fhttp);
                model->fhttp = NULL;
            }
        },
        false);
}

bool web_crawler_custom_event_callback(void *context, uint32_t index)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_custom_event_callback - context is NULL");
        DEV_CRASH();
        return false;
    }

    switch (index)
    {
    case WebCrawlerCustomEventProcess:
        web_crawler_loader_process_callback(context);
        return true;
    default:
        FURI_LOG_DEV(TAG, "web_crawler_custom_event_callback. Unknown index: %ld", index);
        return false;
    }
}

void web_crawler_generic_switch_to_view(WebCrawlerApp *app, char *title, DataLoaderFetch fetcher, DataLoaderParser parser, size_t request_count, ViewNavigationCallback back, uint32_t view_id)
{
    if (app == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_generic_switch_to_view - app is NULL");
        DEV_CRASH();
        return;
    }

    View *view = app->view_loader;
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "web_crawler_generic_switch_to_view - view is NULL");
        DEV_CRASH();
        return;
    }

    with_view_model(
        view,
        DataLoaderModel * model,
        {
            model->title = title;
            model->fetcher = fetcher;
            model->parser = parser;
            model->request_index = 0;
            model->request_count = request_count;
            model->back_callback = back;
            model->data_state = DataStateInitial;
            model->data_text = NULL;
            //
            model->parser_context = app;
            if (!model->fhttp)
            {
                model->fhttp = flipper_http_alloc();
            }
        },
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, view_id);
}
