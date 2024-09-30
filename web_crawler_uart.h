#include <storage/storage.h>
#include <furi_hal_gpio.h>
#include <furi_hal_serial.h>

#define UART_CH (FuriHalSerialIdUsart)
#define BAUDRATE (115200)
#define RX_BUF_SIZE 256
#define SETTINGS_PATH STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler_app/settings.bin"
#define SETTINGS_PATH_SD "/apps_data/web_crawler_app/settings.txt"
#define TAG "WebCrawler"

typedef struct
{
    FuriStreamBuffer *uart_stream; // Stream buffer for UART communication
    FuriHalSerialHandle *serial_handle;
} WebCrawlerUart;

// Declare the UART instance
WebCrawlerUart uart;

static void uart_rx_callback(FuriHalSerialHandle *handle, FuriHalSerialRxEvent event, void *context)
{
    UNUSED(context);
    if (event == FuriHalSerialRxEventData)
    {
        uint8_t data = furi_hal_serial_async_rx(handle);        // Read the incoming byte
        furi_stream_buffer_send(uart.uart_stream, &data, 1, 0); // Send to stream buffer
    }
}

// Initialize UART
void uart_init()
{
    uart.uart_stream = furi_stream_buffer_alloc(RX_BUF_SIZE * 10, 1); // Increase buffer size if necessary
    uart.serial_handle = furi_hal_serial_control_acquire(UART_CH);
    furi_hal_serial_init(uart.serial_handle, BAUDRATE);
    furi_hal_serial_async_rx_start(uart.serial_handle, uart_rx_callback, NULL, false);
    FURI_LOG_I(TAG, "UART initialized.");
}

// Deinitialize UART
void uart_deinit()
{
    furi_hal_serial_async_rx_stop(uart.serial_handle);
    furi_hal_serial_deinit(uart.serial_handle);
    furi_hal_serial_control_release(uart.serial_handle);
    furi_stream_buffer_free(uart.uart_stream);
}

// Function to send settings via UART
void send_settings_via_uart(const char *path, const char *ssid, const char *password)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "{\"path\":\"%s\",\"ssid\":\"%s\",\"password\":\"%s\"}", path, ssid, password);
    size_t data_length = strlen(buffer);

    FURI_LOG_I(TAG, "Sending settings via UART: %s", buffer);
    furi_hal_serial_tx(uart.serial_handle, (uint8_t *)buffer, data_length);
}

bool first_run = true;

// Function to read data from UART after sending settings and save to a file
bool read_data_from_uart_and_save(Canvas *canvas) // Pass the canvas context
{
    canvas_draw_str(canvas, 1, 35, "Initializing...");

    char line_buffer[RX_BUF_SIZE + 1];
    bool started = false;
    size_t total_received_data = 0;

    // Full path for the output file where we will save the received data
    const char *output_file_path = STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler_app/received_data.txt";

    // Ensure the directory exists
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler_app");
    Storage *storage = NULL;
    File *file = NULL;

    if (first_run)
    {
        first_run = false;
        return false;
    }

    FURI_LOG_I(TAG, "Waiting for data...");
    canvas_draw_str(canvas, 1, 50, "Saving data. Please wait");

    // Start reading data from UART line by line
    while (true)
    {
        size_t bytes_received = furi_stream_buffer_receive(uart.uart_stream, line_buffer, RX_BUF_SIZE, 2000); // Increase the timeout to 2000 ms

        if (bytes_received > 0)
        {
            line_buffer[bytes_received] = '\0'; // Null-terminate

            // Remove any carriage returns
            for (size_t i = 0; i < bytes_received; i++)
            {
                if (line_buffer[i] == '\r')
                {
                    memmove(&line_buffer[i], &line_buffer[i + 1], bytes_received - i);
                    bytes_received--;
                    i--;
                }
            }

            // Check for the start marker
            if (!started && strstr(line_buffer, "[BIN/STARTED]") != NULL)
            {
                FURI_LOG_I(TAG, "Started receiving data... please wait and do not disconnect the device or leave this screen.");
                canvas_draw_str(canvas, 1, 60, "Receiving data...");
                started = true;

                // Now open the storage and file for writing since we've started receiving data
                storage = furi_record_open(RECORD_STORAGE);
                storage_common_mkdir(storage, directory_path); // Create directory if it doesn't exist
                file = storage_file_alloc(storage);

                if (!storage_file_open(file, output_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS))
                {
                    FURI_LOG_E(TAG, "Failed to open output file for writing.");
                    canvas_draw_str(canvas, 1, 60, "Failed to open file");
                    storage_file_free(file);
                    furi_record_close(RECORD_STORAGE);
                    return false;
                }

                continue; // Skip to the next iteration after opening the file
            }

            // Check for the finish marker
            if (started && strstr(line_buffer, "[BIN/FINISHED]") != NULL)
            {
                FURI_LOG_I(TAG, "Finished receiving data.");
                canvas_draw_str(canvas, 1, 70, "Finished GET request.");
                break; // End the loop once the finish marker is detected
            }

            // If the data transfer has started, save data to the file
            if (started && file)
            {
                storage_file_write(file, line_buffer, bytes_received);
                total_received_data += bytes_received;
            }
        }
        else
        {
            // Timeout or no data received
            FURI_LOG_E(TAG, "Timeout.");
            break;
        }
    }

    // Close the file and storage only if they were opened
    if (file)
    {
        storage_file_close(file);
        storage_file_free(file);
    }
    if (storage)
    {
        furi_record_close(RECORD_STORAGE);
    }

    FURI_LOG_I(TAG, "Data reception complete. Total bytes received: %zu", total_received_data);

    if (total_received_data > 0)
    {
        canvas_draw_str(canvas, 1, 70, "Complete!");
        return true;
    }
    else
    {
        return false;
    }
}