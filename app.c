#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <dialogs/dialogs.h>
#include <web_crawler_e.h>
#include <web_crawler_uart.h>
#include <web_crawler_storage.h>
#include <web_crawler_callback.h>
#include <web_crawler_free.h>
#include <web_crawler_i.h>
/**
 * @brief      Entry point for the WebCrawler application.
 * @param      p  Input parameter - unused
 * @return     0 to indicate success, -1 on failure
 */
int32_t web_crawler_app(void *p)
{
    UNUSED(p);

    WebCrawlerApp *app = web_crawler_app_alloc();
    if (!app)
    {
        FURI_LOG_E(TAG, "Failed to allocate WebCrawlerApp");
        return -1;
    }

    // Run the application
    view_dispatcher_run(app->view_dispatcher);

    // Free resources after the application loop ends
    web_crawler_app_free(app);

    return 0;
}