#ifndef WEB_CRAWLER_STORAGE_H
#define WEB_CRAWLER_STORAGE_H

#include <furi.h>
#include <storage/storage.h>

#define SETTINGS_PATH STORAGE_EXT_PATH_PREFIX "/apps_data/" http_tag "/settings.bin"
#define RECEIVED_DATA_PATH STORAGE_EXT_PATH_PREFIX "/apps_data/" http_tag "/received_data.txt"

#define MAX_RECEIVED_DATA_SIZE 1024
#define SHOW_MAX_FILE_SIZE 2048

// Define the truncation notice
#define TRUNCATION_NOTICE "\n\n[Data truncated due to size limits]"

// Function to save settings: path, SSID, and password
static void save_settings(const char *path, const char *ssid, const char *password)
{
    // Create the directory for saving settings
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/web_crawler_app");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);

    // Open the settings file
    File *file = storage_file_alloc(storage);
    if (!storage_file_open(file, SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS))
    {
        FURI_LOG_E(TAG, "Failed to open settings file for writing: %s", SETTINGS_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return;
    }

    // Save the path length and data
    size_t path_length = strlen(path) + 1; // Include null terminator
    if (storage_file_write(file, &path_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, path, path_length) != path_length)
    {
        FURI_LOG_E(TAG, "Failed to write path");
    }

    // Save the SSID length and data
    size_t ssid_length = strlen(ssid) + 1; // Include null terminator
    if (storage_file_write(file, &ssid_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, ssid, ssid_length) != ssid_length)
    {
        FURI_LOG_E(TAG, "Failed to write SSID");
    }

    // Save the password length and data
    size_t password_length = strlen(password) + 1; // Include null terminator
    if (storage_file_write(file, &password_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, password, password_length) != password_length)
    {
        FURI_LOG_E(TAG, "Failed to write password");
    }

    FURI_LOG_I(TAG, "Settings saved: path=%s, ssid=%s, password=%s", path, ssid, password);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

// Function to load settings: path, SSID, and password
static bool load_settings(char *path, size_t path_size, char *ssid, size_t ssid_size, char *password, size_t password_size, WebCrawlerApp *app)
{
    Storage *storage = furi_record_open(RECORD_STORAGE);
    File *file = storage_file_alloc(storage);

    if (!storage_file_open(file, SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
    {
        FURI_LOG_E(TAG, "Failed to open settings file for reading: %s", SETTINGS_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false; // Return false if the file does not exist
    }

    // Load the path
    size_t path_length;
    if (storage_file_read(file, &path_length, sizeof(size_t)) != sizeof(size_t) || path_length > path_size ||
        storage_file_read(file, path, path_length) != path_length)
    {
        FURI_LOG_E(TAG, "Failed to read path");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    path[path_length - 1] = '\0'; // Ensure null-termination

    // Load the SSID
    size_t ssid_length;
    if (storage_file_read(file, &ssid_length, sizeof(size_t)) != sizeof(size_t) || ssid_length > ssid_size ||
        storage_file_read(file, ssid, ssid_length) != ssid_length)
    {
        FURI_LOG_E(TAG, "Failed to read SSID");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    ssid[ssid_length - 1] = '\0'; // Ensure null-termination

    // Load the password
    size_t password_length;
    if (storage_file_read(file, &password_length, sizeof(size_t)) != sizeof(size_t) || password_length > password_size ||
        storage_file_read(file, password, password_length) != password_length)
    {
        FURI_LOG_E(TAG, "Failed to read password");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    password[password_length - 1] = '\0'; // Ensure null-termination

    // set the path, ssid, and password
    strncpy(app->path, path, path_size);
    strncpy(app->ssid, ssid, ssid_size);
    strncpy(app->password, password, password_size);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}

static bool text_show_read_lines(File *file, FuriString *str_result)
{
    // Reset the FuriString to ensure it's empty before reading
    furi_string_reset(str_result);

    // Define a buffer to hold the read data
    uint8_t buffer[SHOW_MAX_FILE_SIZE];

    // Read data into the buffer
    size_t read_count = storage_file_read(file, buffer, SHOW_MAX_FILE_SIZE);
    if (storage_file_get_error(file) != FSE_OK)
    {
        FURI_LOG_E(TAG, "Error reading from file.");
        return false;
    }

    // Append each byte to the FuriString
    for (size_t i = 0; i < read_count; i++)
    {
        furi_string_push_back(str_result, buffer[i]);
    }

    return true;
}

static bool load_received_data()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!app_instance->textbox)
    {
        FURI_LOG_E(TAG, "Textbox is NULL");
        return false;
    }

    // Open the storage record
    Storage *storage = furi_record_open(RECORD_STORAGE);
    if (!storage)
    {
        FURI_LOG_E(TAG, "Failed to open storage record");
        return false;
    }

    // Allocate a file handle
    File *file = storage_file_alloc(storage);
    if (!file)
    {
        FURI_LOG_E(TAG, "Failed to allocate storage file");
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Open the file for reading
    if (!storage_file_open(file, RECEIVED_DATA_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
    {
        FURI_LOG_E(TAG, "Failed to open received data file for reading: %s", RECEIVED_DATA_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false; // Return false if the file does not exist
    }

    // Allocate a FuriString to hold the received data
    FuriString *str_result = furi_string_alloc();
    if (!str_result)
    {
        FURI_LOG_E(TAG, "Failed to allocate FuriString");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Read data into the FuriString
    bool read_success = text_show_read_lines(file, str_result);
    if (!read_success)
    {
        FURI_LOG_E(TAG, "Failed to read data from file");
        furi_string_free(str_result);
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Check if there is more data beyond the maximum size
    char extra_byte;
    storage_file_read(file, &extra_byte, 1);

    // Retrieve the C-string from FuriString
    const char *data_cstr = furi_string_get_cstr(str_result);
    // Set the text box with the received data

    widget_reset(app_instance->textbox);
    FURI_LOG_D(TAG, "Received data: %s", data_cstr);
    if (str_result != NULL)
    {
        widget_add_text_scroll_element(
            app_instance->textbox,
            0,
            0,
            128,
            64, data_cstr);
    }
    else
    {
        widget_add_text_scroll_element(
            app_instance->textbox,
            0,
            0,
            128,
            64, "File is empty.");
    }
    // Clean up
    furi_string_free(str_result);
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}

#endif // WEB_CRAWLER_STORAGE_H