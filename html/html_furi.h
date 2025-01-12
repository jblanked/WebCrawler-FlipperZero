#pragma once
#include <furi.h>
#include <furi_hal.h>
/*
 * @brief Parse a Furigana string from an HTML tag
 * @param tag The HTML tag to parse
 * @param html The HTML string to parse
 * @return A Furigana string containing the parsed Furigana
 */
FuriString *html_furi_parse(const char *tag, FuriString *html);