"""
This is only available in uPython in version 1.5.0. and above: https://ofabel.github.io/mp-flipper/reference.html#classes
Author - JBlanked
For use with Flipper Zero and the FlipperHTTP flash: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP
Individual functions to save memory (uPython has already pushed the limits)
Lastly, be careful looping in the requests, you'll get a furi check error. I'm assuming it's a memory issue and will be fixed in future updates.
"""

import time
import flipperzero as f0


def flipper_print(*args, sep=" ", end="\n", clear_screen=True, y_start=0):
    """Print text to the Flipper Zero screen since print() doesn't show on the screen"""
    # Combine all arguments into a single string, manually joining without using splitlines
    text = sep.join(map(str, args)) + end

    # Optionally clear the screen before printing
    if clear_screen:
        f0.canvas_clear()
        f0.canvas_update()  # Ensure the screen is cleared

    # Manually handle line breaks by splitting on "\n"
    lines = []
    current_line = ""
    for char in text:
        if char == "\n":
            lines.append(current_line)
            current_line = ""
        else:
            current_line += char

    lines.append(current_line)  # Add the last line

    # Display each line on the screen
    y_position = y_start
    for line in lines:
        f0.canvas_set_text(0, y_position, line)  # Display each line
        y_position += 10  # Adjust line spacing

    # Update the display to show the text
    f0.canvas_update()


def flipper_http_read_data(sleep_ms: int = 100) -> str:
    """Read data from the Flipper Zero UART"""
    with f0.uart_open(f0.UART_MODE_USART, 115200) as uart:
        raw_data = uart.readline()
        i = 0
        while len(raw_data) == 0 and i < 5:
            raw_data = uart.readline()

            if len(raw_data) > 0:
                data = raw_data.decode()
                # flipper_print(data)
                return data

            i += 1

            time.sleep_ms(sleep_ms)

        return None


def flipper_http_send_data(value: str):
    """Send data to the Flipper Zero UART"""
    if value is None:
        return
    with f0.uart_open(f0.UART_MODE_USART, 115200) as uart:
        uart.write(value.encode())
        uart.flush()


def clear_buffer():
    """Clear the buffer of the Flipper Zero UART"""
    data = flipper_http_read_data()
    while data is not None:
        data = flipper_http_read_data()


def flipper_http_ping() -> bool:
    """Ping the WiFi Devboard to check if it is connected"""
    flipper_http_send_data("[PING]")
    data = flipper_http_read_data()
    clear_buffer()
    return "[PONG]" in data


def flipper_http_connect_wifi() -> bool:
    """Connect to WiFi"""
    flipper_http_send_data("[WIFI/CONNECT]")
    data = flipper_http_read_data()
    clear_buffer()
    # had to write it this way due to mPython limitations
    if "[SUCCESS]" in data:
        return True
    if "[CONNECTED]" in data:
        return True
    elif "[INFO]" in data:
        return True
    return False


def flipper_http_disconnect_wifi() -> bool:
    """Disconnect from WiFi"""
    flipper_http_send_data("[WIFI/DISCONNECT]")
    data = flipper_http_read_data()
    clear_buffer()
    if "[DISCONNECTED]" in data:
        return True
    if "WiFi stop" in data:
        return True
    return False


def flipper_http_list_commands() -> str:
    """List all available commands"""
    flipper_http_send_data("[LIST]")
    data = flipper_http_read_data(500)
    clear_buffer()
    return data


def flipper_http_led_on():
    """Turn on the LED"""
    flipper_http_send_data("[LED/ON]")


def flipper_http_led_off():
    """Turn off the LED"""
    flipper_http_send_data("[LED/OFF]")


def flipper_http_parse_json(key: str, json_data: str) -> str:
    """Parse JSON data"""
    flipper_http_send_data('[PARSE]{"key":"' + key + '","json":' + json_data + "}")
    data = flipper_http_read_data(500)
    clear_buffer()
    return data


def flipper_http_parse_json_array(key: str, index: int, json_data: str) -> str:
    """Parse JSON data"""
    flipper_http_send_data(
        '[PARSE/ARRAY]{"key":"'
        + key
        + '","index":"'
        + index
        + '","json":'
        + json_data
        + "}"
    )
    data = flipper_http_read_data(500)
    clear_buffer()
    return data


def flipper_http_scan_wifi() -> str:
    """Scan for WiFi networks"""
    flipper_http_send_data("[WIFI/SCAN]")
    data = flipper_http_read_data(500)
    clear_buffer()
    return data


def flipper_http_save_wifi(ssid: str, password: str) -> bool:
    """Save WiFi credentials"""
    if ssid is None or password is None:
        return False
    flipper_http_send_data(
        '[WIFI/SAVE]{"ssid":"' + ssid + '","password":"' + password + '"}'
    )
    data = flipper_http_read_data()
    clear_buffer()
    return "[SUCCESS]" in data


def flipper_http_get_request(url: str) -> str:
    """Send a GET request to the specified URL"""
    if url is None:
        return ""
    flipper_http_send_data("[GET]" + url)
    if "[GET/SUCCESS]" in flipper_http_read_data():
        data = flipper_http_read_data()
        clear_buffer()
        return data
    clear_buffer()
    return ""


def flipper_http_get_request_with_headers(url: str, headers: str) -> str:
    """Send a GET request to the specified URL with headers"""
    if url is None:
        return ""
    flipper_http_send_data('[GET/HTTP]{url:"' + url + '",headers:' + headers + "}")
    if "[GET/SUCCESS]" in flipper_http_read_data():
        data = flipper_http_read_data(500)
        clear_buffer()
        return data
    clear_buffer()
    return ""


def flipper_http_post_request_with_headers(url: str, headers: str, data: str):
    """Send a POST request to the specified URL with headers and data"""
    if url is None:
        return ""
    flipper_http_send_data(
        '[POST/HTTP]{"url":"'
        + url
        + '","headers":'
        + headers
        + ',"payload":'
        + data
        + "}"
    )
    if "[POST/SUCCESS]" in flipper_http_read_data():
        data = flipper_http_read_data(500)
        clear_buffer()
        return data
    clear_buffer()
    return ""


def flipper_http_put_request_with_headers(url: str, headers: str, data: str):
    """Send a PUT request to the specified URL with headers and data"""
    if url is None:
        return ""
    flipper_http_send_data(
        '[PUT/HTTP]{"url":"'
        + url
        + '","headers":'
        + headers
        + ',"payload":'
        + data
        + "}"
    )
    if "[PUT/SUCCESS]" in flipper_http_read_data():
        data = flipper_http_read_data(500)
        clear_buffer()
        return data
    clear_buffer()
    return ""


def flipper_http_delete_request_with_headers(url: str, headers: str, data: str):
    """Send a DELETE request to the specified URL with headers and data"""
    if url is None:
        return ""
    flipper_http_send_data(
        '[DELETE/HTTP]{"url":"'
        + url
        + '","headers":'
        + headers
        + ',"payload":'
        + data
        + "}"
    )
    if "[DELETE/SUCCESS]" in flipper_http_read_data():
        data = flipper_http_read_data(500)
        clear_buffer()
        return data
    clear_buffer()
    return ""


# Example of how to use the functions
""" uncomment to run the example
flipper_print("Starting HTTP example")
clear_buffer()  # Clear the buffer before starting
time.sleep(1)
if flipper_http_ping() and flipper_http_save_wifi("JBlanked", "maingirl"):
    flipper_print("WiFi saved successfully!")
    time.sleep(2)
    if flipper_http_connect_wifi():
        flipper_print("WiFi connected successfully!")
        time.sleep(2)
        flipper_print(
            flipper_http_get_request_with_headers(
                "https://httpbin.org/get", "{Content-Type: application/json}"
            )
        )
        time.sleep(2)

        if flipper_http_disconnect_wifi():
            flipper_print("WiFi disconnected successfully!")
            time.sleep(2)
else:
    flipper_print("Failed to save WiFi credentials")
    time.sleep(2)
    flipper_print("Exiting...")
    time.sleep(2)
"""  # uncomment to run the example
