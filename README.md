## Overview

**Web Crawler** is a custom application designed for the [Flipper Zero](https://flipperzero.one/) device. This app allows users to configure and manage web crawling operations directly from their Flipper Zero. 

## Features

- **Configurable Website Path**: Specify the URL of the website you want to crawl.
- **Wi-Fi Configuration**: Enter your Wi-Fi SSID and password to enable network communication.
- **Data Storage**: Automatically saves received data to the device's storage for easy access.


## Usage

1. **Launch the Web Crawler App**

   Navigate to the `Apps` menu on your Flipper Zero, select `GPIO`, then scroll down and select **Web Crawler**.

2. **Main Menu**

   Upon launching, you'll be presented with a submenu containing the following options:

   - **Run**: Initiate the web crawling operation.
   - **About**: View information about the Web Crawler app.
   - **Configure**: Set up the crawling parameters.

3. **Configuring Settings**

   Select **Configure** to enter the following parameters:

   - **Path**: Enter the URL of the website you wish to crawl.
   - **SSID**: Input your Wi-Fi network's SSID.
   - **Password**: Provide the corresponding Wi-Fi password.

   Use the Flipper Zero's navigation buttons to input and confirm your settings. Once configured, these settings will be saved and used for subsequent crawling operations.

4. **Running the Crawler**

   Select **Run** from the main submenu to start the web crawling process. The app will:

   - **Send Settings**: Transmit the configured path, SSID, and password via UART to the Wifi Dev Board.
   - **Receive Data**: Listen for incoming data from the UART interface.
   - **Store Data**: Save the received data to the device's storage for later retrieval.

   Monitor the operation state displayed on the main screen to track progress.

5. **Accessing Crawled Data**

   After the crawling operation completes, navigate to the storage directory to access the `received_data.txt` file, which contains the crawled information.

## Setting Up Parameters

1. **Path (URL)**
   - Enter the complete URL of the website you intend to crawl (e.g., `https://www.example.com/`).

2. **SSID (Wi-Fi Network)**
   - Provide the name of your Wi-Fi network to enable the Flipper Zero to communicate over the network if required.

3. **Password (Wi-Fi Network)**
   - Input the corresponding password for your Wi-Fi network.

## Saving Settings

After entering the desired configuration parameters, the app automatically saves these settings and uses them during the web crawling process. You can update these settings at any time by navigating back to the **Configure** menu.

## Logging and Debugging

The Web Crawler app utilizes logging to help identify issues:

- **Info Logs**: Provide general information about the app's operations (e.g., UART initialization, sending settings).
- **Error Logs**: Indicate problems encountered during execution (e.g., failed to open settings file).

Connect your Flipper Zero to a computer and use a serial terminal to view these logs for detailed troubleshooting.


*Happy Crawling! 🕷️*
