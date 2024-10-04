// web_crawler_callback.h
static bool sent_get_request = false;
static bool get_success = false;
static bool already_success = false;
static WebCrawlerApp *app_instance = NULL;

// Forward declaration of callback functions
static void web_crawler_setting_item_path_clicked(void *context, uint32_t index);
static void web_crawler_setting_item_ssid_clicked(void *context, uint32_t index);
static void web_crawler_setting_item_password_clicked(void *context, uint32_t index);

static void web_crawler_view_draw_callback(Canvas *canvas, void *context)
{
    UNUSED(context);

    WebCrawlerApp *app = app_instance;
    if (!app)
    {
        FURI_LOG_E(TAG, "App is NULL");
        return;
    }

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If you board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your Dev Board with the");
        canvas_draw_str(canvas, 0, 62, "FlipperHTTP firmware.");
        return;
    }

    if (app->path)
    {
        if (!sent_get_request)
        {

            canvas_draw_str(canvas, 0, 10, "Sending GET request...");

            // Perform GET request and handle the response
            get_success = flipper_http_get_request(app->path);

            canvas_draw_str(canvas, 0, 20, "Sent!");

            if (get_success)
            {
                canvas_draw_str(canvas, 0, 30, "Receiving data...");
                get_success = true;
            }
            else
            {
                canvas_draw_str(canvas, 0, 30, "Failed.");
            }

            sent_get_request = true;
        }
        else
        {
            if (get_success && fhttp.state == RECEIVING)
            {
                canvas_draw_str(canvas, 0, 10, "Receiving and parsing data...");
                already_success = true;
            }
            else if (get_success && fhttp.state == IDLE)
            {
                already_success = true;
                canvas_draw_str(canvas, 0, 10, "Data saved to file.");
                canvas_draw_str(canvas, 0, 20, "Press BACK to return.");
            }
            else
            {
                if (fhttp.state == ISSUE)
                {
                    if (strstr(fhttp.last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
                    {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                        canvas_draw_str(canvas, 0, 50, "Update your config settings.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    }
                    else if (strstr(fhttp.last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
                    {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                        canvas_draw_str(canvas, 0, 50, "Update your config settings.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    }
                    else
                    {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Failed to sync data.");
                        canvas_draw_str(canvas, 0, 30, "If this is your third attempt,");
                        canvas_draw_str(canvas, 0, 40, "it's likely your URL is not");
                        canvas_draw_str(canvas, 0, 50, "compabilbe or correct.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    }
                }
                else
                {
                    canvas_draw_str(canvas, 0, 10, "GET request failed.");
                    canvas_draw_str(canvas, 0, 20, "Press BACK to return.");
                }
                get_success = false;
            }
        }
    }
    else
    {
        canvas_draw_str(canvas, 0, 10, "URL not set.");
    }
}

/**
 * @brief      Navigation callback to handle exiting from other views to the submenu.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
static uint32_t web_crawler_back_to_main_callback(void *context)
{
    UNUSED(context);
    sent_get_request = false;
    get_success = false;
    already_success = false;
    if (app_instance && app_instance->textbox)
    {
        widget_reset(app_instance->textbox);
    }
    return WebCrawlerViewSubmenu; // Return to the main submenu view
}

/**
 * @brief      Navigation callback to handle returning to the Configure screen.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewConfigure
 */
static uint32_t web_crawler_back_to_configure_callback(void *context)
{
    UNUSED(context);
    return WebCrawlerViewConfigure; // Return to the Configure screen
}

/**
 * @brief      Navigation callback to handle exiting the app from the main submenu.
 * @param      context   The context - unused
 * @return     VIEW_NONE to exit the app
 */
static uint32_t web_crawler_exit_app_callback(void *context)
{
    UNUSED(context);
    return VIEW_NONE;
}

/**
 * @brief      Handle submenu item selection.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The WebCrawlerSubmenuIndex item that was clicked.
 */
static void web_crawler_submenu_callback(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    switch (index)
    {
    case WebCrawlerSubmenuIndexRun:
        sent_get_request = false; // Reset the flag
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewRun);
        break;
    case WebCrawlerSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewAbout);
        break;
    case WebCrawlerSubmenuIndexSetPath:
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewConfigure);
        break;
    case WebCrawlerSubmenuIndexData:
        if (!load_received_data())
        {
            if (app_instance->textbox)
            {
                widget_reset(app_instance->textbox);
                widget_add_text_scroll_element(
                    app_instance->textbox,
                    0,
                    0,
                    128,
                    64, "File is empty.");
            }
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewData);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown submenu index");
        break;
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
static void web_crawler_config_enter_callback(void *context, uint32_t index)
{
    switch (index)
    {
    case 0:
        web_crawler_setting_item_path_clicked(context, index);
        break;
    case 1:
        web_crawler_setting_item_ssid_clicked(context, index);
        break;
    case 2:
        web_crawler_setting_item_password_clicked(context, index);
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
static void web_crawler_set_path_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;

    furi_check(app);

    // Store the entered URL from temp_buffer_path to path
    strncpy(app->path, app->temp_buffer_path, app->temp_buffer_size_path - 1);

    if (app->path_item)
    {
        variable_item_set_current_value_text(app->path_item, app->path);

        // Save the URL to the settings
        save_settings(app->path, app->ssid, app->password);

        // send to UART
        if (!flipper_http_save_wifi(app->ssid, app->password))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "URL saved: %s", app->path);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewConfigure);
}

/**
 * @brief      Callback for when the user finishes entering the SSID.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_ssid_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    // Store the entered SSID from temp_buffer_ssid to ssid
    strncpy(app->ssid, app->temp_buffer_ssid, app->temp_buffer_size_ssid - 1);

    if (app->ssid_item)
    {
        variable_item_set_current_value_text(app->ssid_item, app->ssid);

        // Save the SSID to the settings
        save_settings(app->path, app->ssid, app->password);

        // send to UART
        if (!flipper_http_save_wifi(app->ssid, app->password))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "SSID saved: %s", app->ssid);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewConfigure);
}

/**
 * @brief      Callback for when the user finishes entering the Password.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_password_update(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    // Store the entered Password from temp_buffer_password to password
    strncpy(app->password, app->temp_buffer_password, app->temp_buffer_size_password - 1);

    if (app->password_item)
    {
        variable_item_set_current_value_text(app->password_item, app->password);

        // Save the Password to the settings
        save_settings(app->path, app->ssid, app->password);

        // send to UART
        if (!flipper_http_save_wifi(app->ssid, app->password))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "Password saved: %s", app->password);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewConfigure);
}

/**
 * @brief      Handler for Path configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_path_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    UNUSED(index);
    // Set up the text input
    text_input_set_header_text(app->text_input_path, "Enter URL");

    // Initialize temp_buffer with existing path
    if (app->path)
    {
        strncpy(app->temp_buffer_path, app->path, app->temp_buffer_size_path - 1);
    }
    else
    {
        strncpy(app->temp_buffer_path, "https://www.google.com/", app->temp_buffer_size_path - 1);
    }

    app->temp_buffer_path[app->temp_buffer_size_path - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_path,
        web_crawler_set_path_updated,
        app,
        app->temp_buffer_path,
        app->temp_buffer_size_path,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_path),
        web_crawler_back_to_configure_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInput);
}

/**
 * @brief      Handler for SSID configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_ssid_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    UNUSED(index);
    // Set up the text input
    text_input_set_header_text(app->text_input_ssid, "Enter SSID");

    // Initialize temp_buffer with existing SSID
    if (app->ssid)
    {
        strncpy(app->temp_buffer_ssid, app->ssid, app->temp_buffer_size_ssid - 1);
    }
    else
    {
        strncpy(app->temp_buffer_ssid, "SSID-2G-", app->temp_buffer_size_ssid - 1);
    }

    app->temp_buffer_ssid[app->temp_buffer_size_ssid - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_ssid,
        web_crawler_set_ssid_updated,
        app,
        app->temp_buffer_ssid,
        app->temp_buffer_size_ssid,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_ssid),
        web_crawler_back_to_configure_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputSSID);
}

/**
 * @brief      Handler for Password configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_password_clicked(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    furi_check(app);
    UNUSED(index);
    // Set up the text input
    text_input_set_header_text(app->text_input_password, "Enter Password");

    // Initialize temp_buffer with existing password
    strncpy(app->temp_buffer_password, app->password, app->temp_buffer_size_password - 1);
    app->temp_buffer_password[app->temp_buffer_size_password - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_password,
        web_crawler_set_password_update,
        app,
        app->temp_buffer_password,
        app->temp_buffer_size_password,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_password),
        web_crawler_back_to_configure_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputPassword);
}