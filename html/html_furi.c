#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <html/html_furi.h>

/*
 * Helper function: Checks if the substring of the FuriString starting at index `pos`
 * matches the given C-string `needle`.
 * Returns true if it matches; otherwise false.
 */
static bool furi_string_sub_equals(FuriString *str, int pos, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (pos + needle_len > furi_string_size(str))
    {
        return false;
    }
    for (size_t i = 0; i < needle_len; i++)
    {
        if (furi_string_get_char(str, pos + i) != needle[i])
        {
            return false;
        }
    }
    return true;
}

/*
 * @brief Parse a Furigana string from an HTML tag, handling nested child tags.
 *
 * This version accepts an HTML tag as a C-string (e.g., "<p>") and searches
 * for the content inside the corresponding opening and closing tags within
 * the provided HTML string, taking into account nested occurrences of the tag.
 *
 * For example, given the HTML string:
 *     "<p><h1><p><h1>Test</h1></p></h1></p>"
 * and searching with tag "<p>" the function will return:
 *     "<h1><p><h1>Test</h1></p></h1>"
 *
 * @param tag The HTML tag to parse (including the angle brackets).
 * @param html The HTML string to parse (as a FuriString).
 * @return A newly allocated FuriString containing the parsed content,
 *         or an empty FuriString if the tag is not found.
 */
FuriString *html_furi_parse(const char *tag, FuriString *html)
{
    int tag_len = strlen(tag);

    // Ensure the tag is at least 3 characters long (e.g., "<p>")
    if (tag_len < 3)
    {
        FURI_LOG_E("html_furi_parse", "Invalid tag length");
        return NULL;
    }

    // Extract the inner tag name from the provided C-string tag.
    // For example, for "<p>" extract "p".
    int inner_len = tag_len - 2; // Exclude the '<' and '>'
    char inner_tag[inner_len + 1];
    for (int i = 0; i < inner_len; i++)
    {
        inner_tag[i] = tag[i + 1];
    }
    inner_tag[inner_len] = '\0';

    // Build the expected closing tag as a C-string (e.g., "</p>").
    int closing_tag_size = inner_len + 4; // "</" + inner tag + ">" + '\0'
    char closing_tag[closing_tag_size];
    snprintf(closing_tag, closing_tag_size, "</%s>", inner_tag);

    // Find the opening tag in the HTML.
    // Locate the first occurrence of the opening tag.
    int html_len = furi_string_size(html);
    int open_tag_index = -1;
    for (int i = 0; i <= html_len - tag_len; i++)
    {
        if (furi_string_sub_equals(html, i, tag))
        {
            open_tag_index = i;
            break;
        }
    }
    if (open_tag_index == -1)
    {
        // Opening tag not found; return an empty FuriString.
        FURI_LOG_E("html_furi_parse", "Opening tag not found");
        return NULL;
    }

    // Content starts immediately after the opening tag.
    int content_start = open_tag_index + tag_len;

    // Skip any leading whitespace.
    while (content_start < html_len && furi_string_get_char(html, content_start) == ' ')
    {
        content_start++;
    }

    // Now search for the matching closing tag. We use a depth counter to handle nested tags.
    int depth = 1;
    int i = content_start;
    int matching_close_index = -1;
    while (i <= html_len - 1)
    {
        // Check for opening tag first.
        if (furi_string_sub_equals(html, i, tag))
        {
            depth++;
            i += tag_len;
            continue;
        }
        // Check for closing tag.
        if (furi_string_sub_equals(html, i, closing_tag))
        {
            depth--;
            if (depth == 0)
            {
                matching_close_index = i;
                break;
            }
            i += strlen(closing_tag);
            continue;
        }
        i++;
    }

    if (matching_close_index == -1)
    {
        // Matching closing tag not found; return an empty FuriString.
        FURI_LOG_E("html_furi_parse", "Matching closing tag not found");
        return NULL;
    }

    // The content spans from content_start up to matching_close_index.
    size_t content_length = matching_close_index - content_start;
    if (memmgr_get_free_heap() < (content_length + 1 + 1024)) // 1KB buffer
    {
        FURI_LOG_E("html_furi_parse", "Not enough heap to allocate result");
        return NULL;
    }

    // Allocate the result string and copy the content.
    FuriString *result = furi_string_alloc();
    furi_string_reserve(result, content_length);
    furi_string_set_n(result, html, content_start, content_length);

    return result;
}
