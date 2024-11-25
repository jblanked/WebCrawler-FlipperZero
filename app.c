#include <web_crawler.h>
#include <alloc/web_crawler_alloc.h>
/**
 * @brief      Entry point for the WebCrawler application.
 * @param      p  Input parameter - unused
 * @return     0 to indicate success, -1 on failure
 */
int32_t web_crawler_app(void *p)
{
    UNUSED(p);

    app_instance = web_crawler_app_alloc();
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "Failed to allocate WebCrawlerApp");
        return -1;
    }

    if (!flipper_http_ping())
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        return -1;
    }

    // Edit from Derek Jamison
    if (app_instance->text_input_ssid != NULL && app_instance->text_input_password != NULL)
    {
        // Try to wait for pong response.
        uint8_t counter = 10;
        while (fhttp.state == INACTIVE && --counter > 0)
        {
            FURI_LOG_D(TAG, "Waiting for PONG");
            furi_delay_ms(100);
        }

        if (counter == 0)
        {
            DialogsApp *dialogs = furi_record_open(RECORD_DIALOGS);
            DialogMessage *message = dialog_message_alloc();
            dialog_message_set_header(
                message, "[FlipperHTTP Error]", 64, 0, AlignCenter, AlignTop);
            dialog_message_set_text(
                message,
                "Ensure your WiFi Developer\nBoard or Pico W is connected\nand the latest FlipperHTTP\nfirmware is installed.",
                0,
                63,
                AlignLeft,
                AlignBottom);
            dialog_message_show(dialogs, message);
            dialog_message_free(message);
            furi_record_close(RECORD_DIALOGS);
        }
    }

    // Run the application
    view_dispatcher_run(app_instance->view_dispatcher);

    // Free resources after the application loop ends
    web_crawler_app_free(app_instance);

    return 0;
}