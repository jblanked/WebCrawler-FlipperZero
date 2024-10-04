static void free_buffers(WebCrawlerApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "Invalid app context");
        return;
    }
    if (app->path)
    {
        free(app->path);
        app->path = NULL;
    }

    if (app->temp_buffer_path)
    {
        free(app->temp_buffer_path);
        app->temp_buffer_path = NULL;
    }

    if (app->ssid)
    {
        free(app->ssid);
        app->ssid = NULL;
    }

    if (app->temp_buffer_ssid)
    {
        free(app->temp_buffer_ssid);
        app->temp_buffer_ssid = NULL;
    }

    if (app->password)
    {
        free(app->password);
        app->password = NULL;
    }

    if (app->temp_buffer_password)
    {
        free(app->temp_buffer_password);
        app->temp_buffer_password = NULL;
    }
}

static void free_resources(WebCrawlerApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "Invalid app context");
        return;
    }

    free_buffers(app);
    free(app);
}

static void free_all(WebCrawlerApp *app, char *reason)
{
    FURI_LOG_E(TAG, reason);
    if (app->view_main)
        view_free(app->view_main);
    if (app->submenu)
        submenu_free(app->submenu);
    if (app->variable_item_list_config)
        variable_item_list_free(app->variable_item_list_config);
    if (app->view_dispatcher)
        view_dispatcher_free(app->view_dispatcher);
    if (app->widget_about)
        widget_free(app->widget_about);
    if (app->textbox)
        widget_free(app->textbox);

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
    {
        FURI_LOG_E(TAG, "Invalid app context");
        return;
    }

    // Remove and free Main view
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewRun);
    view_free(app->view_main);

    // diconnect wifi
    if (!flipper_http_disconnect_wifi())
    {
        FURI_LOG_E(TAG, "Failed to disconnect from WiFi");
    }

    // Deinitialize UART
    flipper_http_deinit();

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

    // Remove and free About view
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewAbout);
    widget_free(app->widget_about);

    // Remove and free Textbox view
    view_dispatcher_remove_view(app->view_dispatcher, WebCrawlerViewData);
    widget_free(app->textbox);

    // Free the ViewDispatcher and close GUI
    view_dispatcher_free(app->view_dispatcher);

    // Free the application structure
    free_buffers(app);
    free(app);
}
