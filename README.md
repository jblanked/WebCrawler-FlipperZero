## Overview

**Web Crawler** is a custom application designed for the Flipper Zero device, allowing users to configure and manage GET requests directly from their Flipper Zero.

## Requirements
- WiFi Dev Board for Flipper Zero with WebCrawler Flash: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP
- WiFi Access Point

## Features

- **Configurable Request**: Specify the URL of the website you want to send a GET request to.
- **Wi-Fi Configuration**: Enter your Wi-Fi SSID and password to enable network communication.
- **File Management**: Automatically saves and manages received data on the device's storage, allowing users to view, rename, and delete the received data at any time.

## Usage

1. **Connection**: Turn off your Flipper, connect the WiFi Dev Board, then turn your Flipper back on.

2. **Launch the Web Crawler App**: Navigate to the Apps menu on your Flipper Zero, select GPIO, then scroll down and select **Web Crawler**.

3. **Main Menu**: Upon launching, you'll see a submenu containing the following options:
   - **Run**: Initiate the GET request.
   - **About**: View information about the Web Crawler app.
   - **Settings**: Set up parameters or perform file operations.

4. **Settings**: Select **Settings** and navigate to WiFi Settings. Use the Flipper Zero's navigation buttons to input and confirm your settings. Do the same for the Request settings. Once configured, these settings will be saved and used for subsequent GET request operations.

5. **Running the Crawler**: Select **Run** from the main submenu to start the GET request process. The app will:
   - **Send Settings**: Transmit the GET request via serial to the WiFi Dev Board.
   - **Receive Data**: Listen for incoming data.
   - **Store Data**: Save the received data to the device's storage for later retrieval.
   - **Log**: Display detailed analysis of the operation status on the screen.

6. **Accessing Received Data**: After the GET request operation completes, you can access the received data by:
   - Connecting to Flipper and opening the SD/apps_data/web_crawler_app/ storage directory to access the received_data.txt file (or the file name/type customized in the settings).
   - Navigating to File Settings and selecting Read File.

## Setting Up Parameters

1. **Path (URL)**
   - Enter the complete URL of the website you intend to crawl (e.g., https://www.example.com/).

2. **SSID (WiFi Network)**
   - Provide the name of your WiFi network to enable the Flipper Zero to communicate over the network.

3. **Password (WiFi Network)**
   - Input the corresponding password for your WiFi network.

4. **Set File Type**
   - Enter your desired file extension. After saving, the app will rename your file, applying the new extension.

5. **Rename File**
   - Provide your desired file name. After saving, the app will rename your file with the new name.

## Saving Settings

After entering the desired configuration parameters, the app automatically saves these settings for use during the GET request process. You can update these settings at any time by navigating back to the **Settings** menu.

## Logging and Debugging

The Web Crawler app uses logging to help identify issues:

- **Info Logs**: Provide general information about the app's operations (e.g., UART initialization, sending settings).
- **Error Logs**: Indicate problems encountered during execution (e.g., failed to open settings file).

Connect your Flipper Zero to a computer and use a serial terminal to view these logs for detailed troubleshooting.

*Happy Crawling! 🕷️* 