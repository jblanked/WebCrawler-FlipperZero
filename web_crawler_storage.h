#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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

    FURI_LOG_I(TAG, "Settings loaded: path=%s, ssid=%s, password=%s", path, ssid, password);

    // set the path, ssid, and password
    strncpy(app->path, path, path_size);
    strncpy(app->ssid, ssid, ssid_size);
    strncpy(app->password, password, password_size);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}