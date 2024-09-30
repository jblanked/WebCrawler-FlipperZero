// Function to free allocated buffers
static void free_buffers(WebCrawlerApp *app)
{
    free(app->path);
    free(app->temp_buffer_path);
    free(app->ssid);
    free(app->temp_buffer_ssid);
    free(app->password);
    free(app->temp_buffer_password);
}

static void free_resources(WebCrawlerApp *app)
{
    free_buffers(app);
    free(app);
}

static void free_inputs(WebCrawlerApp *app)
{
    free(app->temp_buffer_path);
    free(app->temp_buffer_ssid);
    free(app->temp_buffer_password);
}

static void free_all(WebCrawlerApp *app, char *reason)
{
    FURI_LOG_E(TAG, reason);
    view_free(app->view_main);
    submenu_free(app->submenu);
    variable_item_list_free(app->variable_item_list_config);
    free_inputs(app);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free_resources(app);
}

/**
 * @brief      Function to free the resources used by WebCrawlerApp.
 * @param      app  The WebCrawlerApp object to free.
 */
static void web_crawler_app_free(WebCrawlerApp *app)
{
    if (!app)
        return;

    // Remove and free Main view
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewMain);
    view_free(app->view_main);

    // Remove and free Submenu
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewSubmenu);
    submenu_free(app->submenu);

    // Remove and free Configuration screen
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewConfigure);
    variable_item_list_free(app->variable_item_list_config);

    // Remove and free Text Input views
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewTextInputSSID);
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewTextInputPassword);
    free_inputs(app);

    // Remove and free About view
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewAbout);
    widget_free(app->widget_about);

    // Free the ViewDispatcher and close GUI
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    // deinit uart
    uart_deinit();

    // Free the application structure
    free_buffers(app);
    free(app);
}