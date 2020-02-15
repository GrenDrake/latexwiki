#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "latexwiki.h"

bool showMissingWorld = false;
bool showMissingCategory = false;
bool hideWarnings = false;

void dumpErrors(const ErrorLog &errorLog, bool hideWarnings) {
    for (const ErrorMsg &msg : errorLog.errors) {
        if (hideWarnings && msg.type == ErrorType::Warning) continue;
        switch (msg.type) {
            case ErrorType::Fatal:      std::cerr << "FATAL  "; break;
            case ErrorType::Error:      std::cerr << "ERROR  "; break;
            case ErrorType::Warning:    std::cerr << "WARN   "; break;
        }
        std::cerr << msg.sourceFile << ": " << msg.message << "\n";
    }
    std::cerr << "Warnings: " << errorLog.warnCount << "; errors: " << errorLog.errorCount << "; fatals: " << errorLog.fatalCount << ".\n";
}

int main(int argc, const char **argv) {
    std::string filelist;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-noworld") showMissingWorld = true;
        else if (arg == "-nocategory") showMissingCategory = true;
        else if (arg == "-hidewarnings") hideWarnings = true;
        else if (arg == "-help") {
            std::cerr << "USAGE: convert [-noworld] -[nocategory] [project file]\n\n";
            std::cerr << "-nohelp         Show this information\n";
            std::cerr << "-noworld        Show articles with no set world\n";
            std::cerr << "-nocategory     Show articles with no set category\n";
            std::cerr << "-hidewarnings   Hide generated warnings\n";
            return 0;
        } else if (arg[0] == '-') {
            std::cerr << "Unrecognized argument " << arg << "; run \"convert -help\" for instructions.\n";
            return 1;
        } else {
            if (filelist.empty()) filelist = arg;
            else {
                std::cerr << "Cannot set project file to " << arg;
                std::cerr << " as project file already specified as ";
                std::cerr << filelist << ".\n";
            }
        }
    }
    if (filelist.empty()) filelist = "files.lst";

    Document document;
    ErrorLog errorLog;
    ScanDocument scanner(&document);
    const std::string front = readFile("front.html");
    const std::string back = readFile("back.html");

    std::chrono::milliseconds scanStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::ifstream files(filelist);
    if (!files) {
        std::cerr << "Failed to open project file " << filelist << ".\n";
        return 1;
    }
    std::string filename;
    std::cerr << "SCANNING FILES...\n";
    while (std::getline(files, filename)) {
        if (filename.empty()) continue;
        // std::cerr << '[' << filename << "]\n";
        Article *a = processFile(filename, errorLog);
        if (!a) continue;

        scanner.article = a;
        scanner.errorLog = &errorLog;
        a->process(scanner);
        if (!a->hasPageInfo) {
            errorLog.add(ErrorType::Warning, filename, "Article is missing page info.");
        }

        document.articles.push_back(a);
    }
    files.close();
    std::chrono::milliseconds scanEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::cerr << "Completed in " << (scanEnd - scanStart).count() << " ms.\n\n";

    if (errorLog.hasErrors()) {
        dumpErrors(errorLog, hideWarnings);
        return 1;
    }


    std::chrono::milliseconds writeStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::cerr << "WRITING FILES...\n";
    for (Article *article : document.articles) {
        // std::cerr << '[' << article->filename << "]\n";

        const std::string realFilename = "out/" + article->filename;
        std::ofstream outf(realFilename);
        if (!outf) {
            std::cerr << "Failed to open output file " << realFilename << "\n";
            continue;
        }
        std::string newFront = front;
        std::string::size_type titlePos = newFront.find("%TITLE%");
        newFront.replace(titlePos, 7, article->name);
        outf << newFront;
        FormatDocument dd(&document, outf);
        dd.errorLog = &errorLog;
        dd.article = article;
        article->process(dd);
        outf << back;
    }
    std::chrono::milliseconds writeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::cerr << "Completed in " << (writeEnd - writeStart).count() << " ms.\n\n";

    if (errorLog.hasErrors()) {
        dumpErrors(errorLog, hideWarnings);
        return 1;
    }

    std::chrono::milliseconds indexesStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::cerr << "WRITING INDEXES...\n";
    make_indexes(front, back, document);
    std::chrono::milliseconds indexesEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::cerr << "Completed in " << (indexesEnd - indexesStart).count() << " ms.\n\n";

    if (errorLog.hasErrors()) {
        dumpErrors(errorLog, hideWarnings);
        return 1;
    }


    std::ofstream linkFile("links.lst");
    for (auto iter : document.links) {
        linkFile << iter.first << " :: " << iter.second.name << "/" << iter.second.targetPage << "/" << iter.second.isFragment << "\n";
    }
    linkFile.close();

    if (!errorLog.isEmpty()) {
        dumpErrors(errorLog, hideWarnings);
    }
    std::cerr << "Total runtime: " << ((scanEnd - scanStart) + (writeEnd - writeStart) + (indexesStart - indexesEnd)).count() << " ms.\n";

    return 0;
}
