// Define the GPIO pins available on the Flipper Zero
GpioPin test_pins[9] = {
    {.port = GPIOA, .pin = LL_GPIO_PIN_7}, // PB7 - USART1_RX
    {.port = GPIOA, .pin = LL_GPIO_PIN_6}, // PB6 - USART1_TX
    {.port = GPIOA, .pin = LL_GPIO_PIN_5},
    {.port = GPIOA, .pin = LL_GPIO_PIN_4},
    {.port = GPIOB, .pin = LL_GPIO_PIN_3},
    {.port = GPIOB, .pin = LL_GPIO_PIN_2},
    {.port = GPIOC, .pin = LL_GPIO_PIN_3},
    {.port = GPIOC, .pin = LL_GPIO_PIN_1},
    {.port = GPIOC, .pin = LL_GPIO_PIN_0}};

// Forward declaration of callback functions
static void web_crawler_setting_item_path_clicked(void *context, uint32_t index);
static void web_crawler_setting_item_ssid_clicked(void *context, uint32_t index);
static void web_crawler_setting_item_password_clicked(void *context, uint32_t index);
/**
 * @brief      Navigation callback to handle exiting from other views to the submenu.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
static uint32_t web_crawler_back_to_main_callback(void *context)
{
    UNUSED(context);
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
    return VIEW_NONE; // Exit the app
}

/**
 * @brief      Handle submenu item selection.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The WebCrawlerSubmenuIndex item that was clicked.
 */
static void web_crawler_submenu_callback(void *context, uint32_t index)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;
    switch (index)
    {
    case WebCrawlerSubmenuIndexRun:
        // Switch to the main view where the saved path will be displayed
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewMain);
        break;
    case WebCrawlerSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewAbout);
        break;
    case WebCrawlerSubmenuIndexSetPath:
        view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewConfigure);
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

// At the top of your file, after includes and defines
static WebCrawlerApp *app_instance = NULL;

// Modify the draw callback function to match the expected signature
static void web_crawler_view_draw_callback(Canvas *canvas, void *model)
{
    WebCrawlerMainModel *main_model = (WebCrawlerMainModel *)model; // Cast model to WebCrawlerMainModel
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if (main_model->path[0] != '\0')
    {
        canvas_draw_str(canvas, 1, 10, "Sending GET request...");

        // Initialize the GPIO pin for output mode
        furi_hal_gpio_init_simple(&test_pins[1], GpioModeOutputPushPull);

        // Set GPIO pin high
        furi_hal_gpio_write(&test_pins[1], true);

        canvas_draw_str(canvas, 1, 20, "Sending Wifi settings..");

        // Send settings via UART
        send_settings_via_uart(main_model->path, main_model->ssid, main_model->password);

        furi_delay_ms(1000); // Delay for 1 second

        // Read data from UART sent by the dev board
        if (read_data_from_uart_and_save(canvas))
        {
            furi_hal_gpio_write(&test_pins[1], false); // Set GPIO pin low
            canvas_draw_str(canvas, 1, 80, "Data received and saved");

            // Switch back to submenu view
            if (app_instance)
            {
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, WebCrawlerViewSubmenu);
            }
        }
        else
        {
            furi_hal_gpio_write(&test_pins[1], false); // Set GPIO pin low
        }
    }
    else
    {
        canvas_draw_str(canvas, 1, 10, "No path saved.");
    }
}
/**
 * @brief      Input callback for the main screen.
 * @param      event    The input event.
 * @param      context  The context - WebCrawlerApp object.
 * @return     true if the event was handled, false otherwise.
 */
static bool web_crawler_view_input_callback(InputEvent *event, void *context)
{
    UNUSED(event);
    UNUSED(context);
    return false;
}

/**
 * @brief      Callback for when the user finishes entering the URL.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_path_updated(void *context)
{
    WebCrawlerApp *app = (WebCrawlerApp *)context;

    // Store the entered URL from temp_buffer_path to path
    strncpy(app->path, app->temp_buffer_path, app->temp_buffer_size_path - 1);

    // Ensure null-termination
    app->path[app->temp_buffer_size_path - 1] = '\0';

    if (app->path_item)
    {
        variable_item_set_current_value_text(app->path_item, app->path);

        // Save the URL to the settings
        save_settings(app->path, app->ssid, app->password);

        FURI_LOG_D(TAG, "URL saved: %s", app->path);
    }

    // Update the main view's model
    WebCrawlerMainModel *main_model = (WebCrawlerMainModel *)view_get_model(app->view_main);
    if (main_model)
    {
        strncpy(main_model->path, app->path, sizeof(main_model->path) - 1);
        main_model->path[sizeof(main_model->path) - 1] = '\0';
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

    // Store the entered SSID from temp_buffer_ssid to ssid
    strncpy(app->ssid, app->temp_buffer_ssid, app->temp_buffer_size_ssid - 1);

    // Ensure null-termination
    app->ssid[app->temp_buffer_size_ssid - 1] = '\0';

    if (app->ssid_item)
    {
        variable_item_set_current_value_text(app->ssid_item, app->ssid);

        // Save the SSID to the settings
        save_settings(app->path, app->ssid, app->password);

        FURI_LOG_D(TAG, "SSID saved: %s", app->ssid);
    }

    // Update the main view's model
    WebCrawlerMainModel *main_model = (WebCrawlerMainModel *)view_get_model(app->view_main);
    if (main_model)
    {
        strncpy(main_model->ssid, app->ssid, sizeof(main_model->ssid) - 1);
        main_model->ssid[sizeof(main_model->ssid) - 1] = '\0';
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

    // Store the entered Password from temp_buffer_password to password
    strncpy(app->password, app->temp_buffer_password, app->temp_buffer_size_password - 1);

    // Ensure null-termination
    app->password[app->temp_buffer_size_password - 1] = '\0';

    if (app->password_item)
    {
        variable_item_set_current_value_text(app->password_item, app->password);

        // Save the Password to the settings
        save_settings(app->path, app->ssid, app->password);

        FURI_LOG_D(TAG, "Password saved: %s", app->password);
    }

    // Update the main view's model
    WebCrawlerMainModel *main_model = (WebCrawlerMainModel *)view_get_model(app->view_main);
    if (main_model)
    {
        strncpy(main_model->password, app->password, sizeof(main_model->password) - 1);
        main_model->password[sizeof(main_model->password) - 1] = '\0';
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
    UNUSED(index);

    // Set up the text input
    text_input_set_header_text(app->text_input_path, "Enter URL");

    // Initialize temp_buffer with existing path
    strncpy(app->temp_buffer_path, "https://www.x.com/", app->temp_buffer_size_path - 1);
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
    UNUSED(index);

    // Set up the text input
    text_input_set_header_text(app->text_input_ssid, "Enter SSID");

    // Initialize temp_buffer with existing SSID
    strncpy(app->temp_buffer_ssid, app->ssid, app->temp_buffer_size_ssid - 1);
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