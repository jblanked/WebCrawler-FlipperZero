// Description: Flipper HTTP API for the Flipper Wifi Developer Board.
// Global: flipper_http_init, flipper_http_deinit, flipper_http_rx_callback(), flipper_http_send_data, flipper_http_connect_wifi, flipper_http_disconnect_wifi, flipper_http_ping, flipper_http_save_wifi, flipper_http_get_request
// License: MIT
// Author: JBlanked
// File: flipper_http.js

let serial = require("serial");

// Define the global `fhttp` object with all the functions
let fhttp = {
    // Constructor
    init: function () {
        serial.setup("usart", 115200);
    },
    // Deconstructor
    deinit: function () {
        serial.end();
    },
    // Read data from the serial port and return it line by line
    read_data: function (delay_ms) {
        let line = serial.readln(delay_ms);
        let i = 5;
        while (line === undefined && i > 0) {
            line = serial.readln(delay_ms);
            i--;
        }
        return line;
    },
    // Send data to the serial port
    send_data: function (data) {
        if (data === "") {
            return;
        }
        serial.write(data);
    },
    // Clear the incoming serial by up to 10 lines
    clear_buffer: function (search_for_success) {
        let data = this.read_data(100);
        let sdata = this.to_string(data);
        let i = 0;
        // clear all data until we get an expected response
        while (i < 5 &&
            (data !== undefined &&
                (!search_for_success || (search_for_success && !this.includes(sdata, "[SUCCESS]"))) &&
                !this.includes(sdata, "[ERROR]") &&
                !this.includes(sdata, "[INFO]") &&
                !this.includes(sdata, "[PONG]") &&
                !this.includes(sdata, "[DISCONNECTED]") &&
                !this.includes(sdata, "[CONNECTED]") &&
                !this.includes(sdata, "[GET/STARTED]") &&
                !this.includes(sdata, "[GET/END]"))) {
            data = this.read_data(100);
            sdata = this.to_string(data);
            i++;
        }
    },
    // Connect to wifi
    connect_wifi: function () {
        serial.write("[WIFI/CONNECT]");
        let response = this.read_data(500);
        if (response === undefined) {
            return false;
        }
        this.clear_buffer(true); // Clear the buffer
        return this.includes(this.to_string(response), "[SUCCESS]") || this.includes(this.to_string(response), "[CONNECTED]") || this.includes(this.to_string(response), "[INFO]");
    },
    // Disconnect from wifi
    disconnect_wifi: function () {
        serial.write("[WIFI/DISCONNECT]");
        let response = this.read_data(500);
        if (response === undefined) {
            return false;
        }
        this.clear_buffer(true); // Clear the buffer
        return this.includes(this.to_string(response), "[DISCONNECTED]") || this.includes(this.to_string(response), "WiFi stop");
    },
    // Send a ping to the board
    ping: function () {
        serial.write("[PING]");
        let response = this.read_data(100);
        if (response === undefined) {
            return false;
        }
        this.clear_buffer(true); // Clear the buffer
        return this.includes(this.to_string(response), "[PONG]");
    },
    // list available commands
    list_commands: function () {
        serial.write("[LIST]");
        let response = this.read_data(500);
        if (response === undefined) {
            return "";
        }
        return this.to_string(response);
    },
    // turn on the LED
    led_on: function () {
        serial.write("[LED/ON]");
    },
    // turn off the LED
    led_off: function () {
        serial.write("[LED/OFF]");
    },
    // parse JSON data
    parse_json: function (key, data) {
        serial.write('[PARSE]{"key":"' + key + '","data":' + data + '}');
        let response = this.read_data(500);
        if (response === undefined) {
            return "";
        }
        return this.to_string(response);
    },
    // parse JSON array
    parse_json_array: function (key, index, data) {
        serial.write('[PARSE/ARRAY]{"key":"' + key + '","index":' + index + ',"data":' + data + '}');
        let response = this.read_data(500);
        if (response === undefined) {
            return "";
        }
        return this.to_string(response);
    },
    // Get Wifi network list
    scan_wifi: function () {
        serial.write("[WIFI/SCAN]");
        let response = this.read_data(500);
        if (response === undefined) {
            return "";
        }
        return this.to_string(response);
    },
    // Save wifi settings
    save_wifi: function (ssid, password) {
        if (ssid === "" || password === "") {
            return false;
        }
        let command = '[WIFI/SAVE]{"ssid":"' + ssid + '","password":"' + password + '"}';
        serial.write(command);
        let response = this.read_data(500);
        if (response === undefined) {
            this.clear_buffer(false); // Clear the buffer
            return false;
        }
        let sresponse = this.to_string(response);
        if (this.includes(sresponse, "[SUCCESS]")) {
            this.clear_buffer(false); // Clear the buffer
            this.clear_buffer(false); // Clear the buffer
            return true;
        }
        else {
            print("Failed to save: " + response);
            this.clear_buffer(false); // Clear the buffer
            return false;
        }
    },
    // get IP address
    ip_address: function () {
        serial.write("[IP/ADDRESS]");
        let response = this.read_data(500);
        if (response === undefined) {
            return "";
        }
        return this.to_string(response);
    },
    // Send a GET request to the board
    // I reduced this to return the first line of the response that isnt undefined
    // You'll also get 'out of memory' errors if you try to read/return too much data
    // As mjs is updated, this can be improved
    get_request: function (url) {
        serial.write('[GET]' + url);
        if (this.read_data(500) === "[GET/SUCCESS] GET request successful.") {
            while (true) {
                let line = this.read_data(500);
                if (line === "[GET/END]") {
                    break;
                }
                if (line !== undefined) {
                    this.clear_buffer(false); // Clear the buffer
                    return line;
                }
            }
        }
        else {
            print("GET request failed");
        }
        this.clear_buffer(); // Clear the buffer
        return "";
    },
    // another GET request but with headers
    get_request_with_headers: function (url, headers) {
        serial.write('[GET/HTTP]{url:"' + url + '",headers:' + headers + '}');
        if (this.read_data(500) === "[GET/SUCCESS] GET request successful.") {
            while (true) {
                let line = this.read_data(500);
                if (line === "[GET/END]") {
                    break;
                }
                if (line !== undefined) {
                    this.clear_buffer(false); // Clear the buffer
                    return line;
                }
            }
        }
        else {
            print("GET request failed");
        }
        this.clear_buffer(); // Clear the buffer
        return "";
    },
    // send POST request with headers
    post_request_with_headers: function (url, headers, data) {
        serial.write('[POST/HTTP]{"url":"' + url + '","headers":' + headers + ',"payload":' + data + '}');
        if (this.read_data(500) === "[POST/SUCCESS] POST request successful.") {
            while (true) {
                let line = this.read_data(500);
                if (line === "[POST/END]") {
                    break;
                }
                if (line !== undefined) {
                    this.clear_buffer(false); // Clear the buffer
                    return line;
                }
            }
        }
        else {
            print("POST request failed");
        }
        this.clear_buffer(); // Clear the buffer
        return "";
    },
    // send PUT request with headers
    put_request_with_headers: function (url, headers, data) {
        serial.write('[PUT/HTTP]{"url":"' + url + '","headers":' + headers + ',"payload":' + data + '}');
        if (this.read_data(500) === "[PUT/SUCCESS] PUT request successful.") {
            while (true) {
                let line = this.read_data(500);
                if (line === "[PUT/END]") {
                    break;
                }
                if (line !== undefined) {
                    this.clear_buffer(false); // Clear the buffer
                    return line;
                }
            }
        }
        else {
            print("PUT request failed");
        }
        this.clear_buffer(); // Clear the buffer
        return "";
    },
    // send DELETE request with headers
    delete_request_with_headers: function (url, headers, data) {
        serial.write('[DELETE/HTTP]{"url":"' + url + '","headers":' + headers + ',"payload":' + data + '}');
        if (this.read_data(500) === "[DELETE/SUCCESS] DELETE request successful.") {
            while (true) {
                let line = this.read_data(500);
                if (line === "[DELETE/END]") {
                    break;
                }
                if (line !== undefined) {
                    this.clear_buffer(false); // Clear the buffer
                    return line;
                }
            }
        }
        else {
            print("DELETE request failed");
        }
        this.clear_buffer(); // Clear the buffer
        return "";
    },
    // Helper function to check if a string contains another string
    includes: function (text, search) {
        let stringLength = text.length;
        let searchLength = search.length;
        if (stringLength < searchLength) {
            return false;
        }
        for (let i = 0; i < stringLength; i++) {
            if (text[i] === search[0]) {
                let found = true;
                for (let j = 1; j < searchLength; j++) {
                    if (text[i + j] !== search[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return true;
                }
            }
        }
    },
    // Convert an array of characters to a string
    to_string: function (text) {
        if (text === undefined) {
            return "";
        }
        let return_text = "";
        for (let i = 0; i < text.length; i++) {
            return_text += text[i];
        }
        return return_text;
    }
};

/* Example Usage:

let textbox = require("textbox");
textbox.setConfig("end", "text");
textbox.show();
textbox.addText("Flipper HTTP Example:\n\n");
// Initialize the flipper http object
fhttp.init();
textbox.addText("Initialized!\n");
// Send ping to the board
let response = fhttp.ping();

if (response) {
    textbox.addText("Ping successful\nSaving wifi settings...\n");
    let success = fhttp.save_wifi("JBlanked", "maingirl");
    if (success) {
        textbox.addText("Wifi settings saved\nSending GET request..\n");
        let url = "https://catfact.ninja/fact";
        let data = fhttp.get_request_with_headers(url, '{"User-Agent":"curl/7.64.1","Content-Type":"application/json"}');
        if (data !== undefined && data !== "") {
            textbox.addText("GET request successful!\n\nReturned Data: \n\n" + data + "\n\nDisconnecting from wifi...\n");
            if (fhttp.disconnect_wifi()) {
                textbox.addText("Disconnected from wifi.\n");
            }
            else {
                textbox.addText("Failed to disconnect from wifi.\n");
            }

        }
        else {
            textbox.addText("GET request failed.\nDisconnecting from wifi...\n");
            if (fhttp.disconnect_wifi()) {
                textbox.addText("Disconnected from wifi.\n");
            }
            else {
                textbox.addText("Failed to disconnect from wifi.\n");
            }
        }
    }
    else {
        textbox.addText("Wifi settings failed to save.\n");
    }
}
else {
    textbox.addText("Ping failed.\n");
}
textbox.addText("Press BACK twice to exit..\n");
delay(100000); // Wait for user to hit back
textbox.addText("\nTimeout exceeded.\nExiting...\n");
delay(5000);
// Destructor
fhttp.deinit();

textbox.addText("Deinitialized!\n");
/*
