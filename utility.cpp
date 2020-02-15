#include <fstream>
#include <string>

static const char *whitespaceChars = " \t\n\r";

int g_toupper(int c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

bool is_identifier(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

std::string readFile(const std::string &filename) {
    std::ifstream inf(filename);
    if (!inf) return "";
    std::string text, line;
    while (std::getline(inf, line)) {
        text += line;
    }
    return text;
}

std::string& trim(std::string &text) {
    std::string::size_type pos;

    pos = text.find_first_not_of(whitespaceChars);
    if (pos > 0) text.erase(0, pos);
    pos = text.find_last_not_of(whitespaceChars);
    text.erase(pos + 1);

    return text;
}
