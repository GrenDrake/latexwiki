#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "latexwiki.h"


CommandInfo BADINFO = { "", 0, 0 };

std::vector<CommandInfo> commandInfo = {
    {   "pageinfo",         4, 4 },
    {   "chapter",          1, 1 },
    {   "section",          1, 1 },
    {   "subsection",       1, 1 },
    {   "subsubsection",    1, 1 },
    {   "paragraph",        1, 1 },
    {   "addlabel",         2, 2 },
    {   "label",            1, 1 },
    {   "pr",               1, 1 },

    {   "begin",            1, 99 },
    {   "end",              1, 1 },
    {   "item",             0, 1 },

    {   "textbf",           1, 1 },
    {   "emph",             1, 1 },
    {   "nexustext",        1, 1 },
    {   "vocab",            5, 5 },

    {   "toprule",          0, 0 },
    {   "midrule",          0, 0 },
    {   "bottomrule",       0, 0 },
    {   "captionof",        0, 0 },


    {   "startorbittable",  0, 0 },
    {   "stoporbittable",   0, 0 },
    {   "planetrow",        5, 5 },
    {   "moonrow",          2, 2 },

    {   "startinfobox",     0, 0 },
    {   "stopinfobox",      0, 0 },
    {   "infotitle",        1, 1 },
    {   "infoline",         2, 2 },
    {   "infohead",         1, 1 },

    {   "narrowimage",      2, 2 },
    {   "mediumimage",      2, 2 },

    {   "LaTeX",            0, 1 },
    {   "clearpage",        0, 0 },
};

const CommandInfo& getCommandInfo(const std::string &name) {
    for (const CommandInfo &cinfo : commandInfo) {
        if (cinfo.name == name) return cinfo;
    }
    return BADINFO;
}

void forwardToNextArgument(const std::string &s, std::string::size_type &pos) {
    std::string::size_type npos = pos;
    while (npos < s.size() && s[npos] == ' ') ++npos;
    if (npos > s.size()) return;
    if (s[npos] == '{' || s[npos] == '[') pos = npos;
}

bool processCommand(const std::string &sourceFile, const std::string &s, std::string::size_type &pos, Node *parent, ErrorLog &errorLog) {
    ++pos;
    std::string::size_type start = pos;

    if (pos >= s.size()) {
        errorLog.add(ErrorType::Fatal, sourceFile, "Unexpected end of text.");
        return false;
    } else if (!is_identifier(s[pos])) {
        parent->add(new Text(s.substr(pos, 1)));
        ++pos;
        return true;
    }

    while (pos < s.size() && is_identifier(s[pos])) ++pos;
    Command *cmd = new Command;
    if (!cmd) {
        errorLog.add(ErrorType::Fatal, sourceFile, "Failed to allocate memory for command.");
        return false;
    }
    parent->add(cmd);

    cmd->command = s.substr(start, pos - start);
    const CommandInfo &cinfo = getCommandInfo(cmd->command);
    if (cinfo.name.empty()) {
        errorLog.add(ErrorType::Warning, sourceFile, "Unknown command " + cmd->command + ".");
    }

    forwardToNextArgument(s, pos);
    while (pos < s.size() && (s[pos] == '{' || s[pos] == '[')) {
        char endChar = s[pos] == '{' ? '}' : ']';
        Fragment *f = new Fragment;
        if (!f) {
            errorLog.add(ErrorType::Fatal, sourceFile, "Failed to allocate memory for fragment.");
            return false;
        }

        ++pos;
        start = pos;
        while (pos < s.size() && s[pos] != endChar) {
            if (s[pos] == '\\') {
                if (pos > start) {
                    f->add(new Text(s.substr(start, pos - start)));
                }
                if (!processCommand(sourceFile, s, pos, f, errorLog)) return false;
                start = pos;
            } else {
                ++pos;
            }
        }
        if (pos > start) {
            f->add(new Text(s.substr(start, pos - start)));
        } else {
            f->add(new Text(""));
        }

        ++pos;
        forwardToNextArgument(s, pos);
        cmd->add(f);
    }

    if (!cinfo.name.empty() && (cmd->size() < cinfo.minArgs || cmd->size() > cinfo.maxArgs)) {
        std::stringstream msg;
        msg << "Command " << cmd->command << " expects " << cinfo.minArgs;
        if (cinfo.minArgs != cinfo.maxArgs) msg << " to " << cinfo.maxArgs;
        msg << " argument(s), but found " << cmd->size() << ".";

        errorLog.add(ErrorType::Error, sourceFile, msg.str());
    }

    return true;
}

Article* processFile(const std::string &sourceFile, ErrorLog &errorLog) {
    if (sourceFile.size() <= 4 || sourceFile.substr(sourceFile.size() - 4) != ".tex") {
        errorLog.add(ErrorType::Fatal, sourceFile, "Unknown input file format.");
        return nullptr;
    }
    std::string::size_type start = sourceFile.find_last_of('/');
    if (start == std::string::npos) start = 0;
    else ++start;
    const std::string dest = sourceFile.substr(start, sourceFile.size() - 3 - start) + "html";

    std::ifstream inf(sourceFile);
    if (!inf) {
        errorLog.add(ErrorType::Fatal, sourceFile, "Could not open file for reading.");
        return nullptr;
    }

    std::vector<std::string> paragraphs;
    std::string line, current;
    while (std::getline(inf, line)) {
        trim(line);
        if (line.empty()) {
            if (!current.empty()) {
                paragraphs.push_back(current);
                current.clear();
            }
        } else {
            if (!current.empty()) current += ' ';
            current += line;
        }
    }
    if (!current.empty()) paragraphs.push_back(current);

    Article *article = new Article;
    article->sourceFile = sourceFile;
    article->filename = dest;
    for (const std::string &s : paragraphs) {
        std::string::size_type start = 0, pos = 0;
        Paragraph *p = new Paragraph;

        for (pos = 0; pos < s.size(); ) {
            if (s[pos] == '\\') {
                if (pos > start) {
                    p->add(new Text(s.substr(start, pos - start)));
                }
                if (!processCommand(sourceFile, s, pos, p, errorLog)) return nullptr;
                start = pos;
            } else {
                ++pos;
            }
        }
        if (pos != start) {
            p->add(new Text(s.substr(start, pos - start)));
        }

        article->add(p);
    }

    return article;
}
