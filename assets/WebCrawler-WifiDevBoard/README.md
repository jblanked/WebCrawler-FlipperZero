## Installation
1. Download the `web_crawler_bootloader.bin`, `web_crawler_firmware_a.bin`, and `web_crawler_partitions.bin` files from within the `WebCrawler-WifiDevBoard` folder.
2. Connect your Flipper Zero to your computer.
3. Open up qFlipper.
4. Click on the File-Opener.
5. Naviate to `SD Card/apps_data/esp_flasher/
6. Drag all three bin files (or the entire folder) into the directory.
7. Plug your Wi-Fi Devboard into the Flipper.
8. Press and keep holding the boot button while you press the reset button once, release the boot button after 2 seconds.
9. Open the ESP Flasher app on your Flipper Zero, it should be located under `Apps->GPIO` from the main menu. If not, download it from the Flipper App Store.
10. Click on Manual Flash.
11. Click on Bootloader and select the `web_crawler_bootloader.bin` that you downloaded earlier.
12. Click on Part Table and select the `web_crawler_partitions.bin` that you downloaded earlier.
13. Click on FirmwareA and select the `web_crawler_firmware_a.bin` that you downloaded earlier.
14. Click on FLASH - fast and follow the instructions on the screen.
15. Now you are all set to use the Web Crawler app.