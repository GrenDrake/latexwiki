#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include "latexwiki.h"

struct IndexEntry {
    std::string name;
    std::string category;
    std::string world;
    std::string targetFile;
    std::string targetFragment;
};

void make_alpha(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo);
void make_world(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo);
void make_category(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo);

bool sort_alpha(const IndexEntry &left, const IndexEntry &right) {
    return left.name < right.name;
}

std::vector<std::string> missingWorld;
std::vector<std::string> missingCategory;

void printList(const std::vector<std::string> &list) {
    for (unsigned i = 0; i < list.size(); ++i) {
        if (list.size() > 2) {
            if (i > 0) std::cerr << ", ";
            if (i == list.size() - 1) std::cerr << "and ";
        } else if (list.size() == 2) {
            if (i > 0) std::cerr << " and ";
        }
        std::cerr << list[i];
    }
}

void make_indexes(const std::string &pageTop, const std::string &pageBottom, Document &document) {
    std::vector<IndexEntry> pinfo;

    for (auto iter : document.links) {
        Article *toPage = document.byFile(iter.second.targetPage);
        if (!toPage) {
            continue;
        }

        IndexEntry entry;
        entry.name = iter.second.displayText;
        entry.targetFile = toPage->filename;
        if (iter.second.isFragment) {
            entry.targetFragment = iter.second.name;
        } else {
            entry.world = toPage->world;
            entry.category = toPage->category;
        }
        pinfo.push_back(entry);
    }

    std::sort(pinfo.begin(), pinfo.end(), sort_alpha);
    make_alpha(pageTop, pageBottom, pinfo);
    make_world(pageTop, pageBottom, pinfo);
    make_category(pageTop, pageBottom, pinfo);

    if (showMissingWorld && !missingWorld.empty()) {
        std::cerr << "Articles without defined world:\n";
        for (const std::string &name : missingWorld) {
            std::cerr << "    " << name << '\n';
        }
    }
    if (showMissingCategory && !missingCategory.empty()) {
        std::cerr << "Articles without defined category:\n";
        for (const std::string &name : missingCategory) {
            std::cerr << "    " << name << '\n';
        }
    }
}

void make_alpha(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo) {
    std::ofstream alphaFile("out/by_alpha.html");
    std::string newFront = pageTop;
    std::string::size_type titlePos = newFront.find("%TITLE%");
    newFront.replace(titlePos, 7, "Alphabetical Index");
    alphaFile << newFront;
    alphaFile << "<h1>Alphabetical Index</h1>\n";
    alphaFile << "<ul class='indexlist'>\n";

    char lastchar = 0;
    for (const IndexEntry entry : pinfo) {
        char firstchar = g_toupper(entry.name[0]);
        if (lastchar != firstchar) {
            alphaFile << "</ul>\n<h2 class='indexhead'>" << firstchar << "</h2>\n<ul class='indexlist'>\n";
            lastchar = firstchar;
        }

        alphaFile << "<li><a href='" << entry.targetFile;
        if (!entry.targetFragment.empty()) {
            alphaFile << '#' << entry.targetFragment;
        }
        alphaFile << "'>" << entry.name;
        alphaFile << "</a>\n";
    }

    alphaFile << "</ul>\n";
    alphaFile << pageBottom;
    alphaFile.close();
}

void make_world(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo) {
    std::map<std::string, std::vector<IndexEntry>> data;

    for (const IndexEntry entry : pinfo) {
        if (entry.world.empty()) {
            if (entry.targetFragment.empty()) {
                missingWorld.push_back(entry.name);
            }
            continue;
        }

        data[entry.world].push_back(entry);
    }

    std::ofstream alphaFile("out/by_world.html");
    std::string newFront = pageTop;
    std::string::size_type titlePos = newFront.find("%TITLE%");
    newFront.replace(titlePos, 7, "World Index");
    alphaFile << newFront;
    alphaFile << "<h1>World Index</h1>\n";
    alphaFile << "<ul>\n";

    for (auto iter : data) {
        alphaFile << "</ul>\n<h2 class='indexhead'>" << iter.first << "</h2>\n<ul class='indexlist'>\n";
        for (const IndexEntry &entry : iter.second) {
            alphaFile << "<li><a href='" << entry.targetFile;
            if (!entry.targetFragment.empty()) {
                alphaFile << '#' << entry.targetFragment;
            }
            alphaFile << "'>" << entry.name;
            alphaFile << "</a>\n";
        }
        alphaFile << "</ul>\n";
    }

    alphaFile << pageBottom;
    alphaFile.close();
}


void make_category(const std::string &pageTop, const std::string &pageBottom, std::vector<IndexEntry> &pinfo) {
    std::map<std::string, std::vector<IndexEntry>> data;

    for (const IndexEntry entry : pinfo) {
        if (entry.category.empty()) {
            if (entry.targetFragment.empty()) {
                missingCategory.push_back(entry.name);
            }
            continue;
        }

        data[entry.category].push_back(entry);
    }

    std::ofstream alphaFile("out/by_category.html");
    std::string newFront = pageTop;
    std::string::size_type titlePos = newFront.find("%TITLE%");
    newFront.replace(titlePos, 7, "Category Index");
    alphaFile << newFront;
    alphaFile << "<h1>Category Index</h1>\n";
    alphaFile << "<ul>\n";

    for (auto iter : data) {
        alphaFile << "</ul>\n<h2 class='indexhead'>" << iter.first << "</h2>\n<ul class='indexlist'>\n";
        for (const IndexEntry &entry : iter.second) {
            alphaFile << "<li><a href='" << entry.targetFile;
            if (!entry.targetFragment.empty()) {
                alphaFile << '#' << entry.targetFragment;
            }
            alphaFile << "'>" << entry.name;
            alphaFile << "</a>\n";
        }
        alphaFile << "</ul>\n";
    }

    alphaFile << pageBottom;
    alphaFile.close();
}
