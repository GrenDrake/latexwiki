#include <iostream>
#include <string>
#include <vector>

#include "latexwiki.h"

FormatDocument::FormatDocument(Document *article, std::ostream &out)
: out(out), document(article)
{ }

void FormatDocument::handle(Node *node) {
    if (node) node->handle(this);
}

void FormatDocument::handle(Fragment *fragment) {
    for (Node *c : fragment->children) {
        handle(c);
    }
}

void FormatDocument::handle(Text *text) {
    out << text->text;
}

void FormatDocument::handle(Command *command) {
    if (command->command == "pageinfo") {
    } else if (command->command == "section" || command->command == "chapter") {
        out << "<h1>";
        handle(command->children.front());
        out << "</h1>";
    } else if (command->command == "subsection") {
        out << "<h2>";
        handle(command->children.front());
        out << "</h2>";
    } else if (command->command == "subsubsection") {
        out << "<h3>";
        handle(command->children.front());
        out << "</h3>";
    } else if (command->command == "paragraph") {
        out << "<span class='parahead'>";
        handle(command->children.front());
        out << "</span>";
    } else if (command->command == "label") {
        out << "<span id='";
        Text *t = dynamic_cast<Text*>(command->children.front());
        if (t) {
            out << t->text;
        } else {
            errorLog->add(ErrorType::Error, article->sourceFile, "Invalid content in label text.");
        }
        out << "'></span>";
    } else if (command->command == "addlabel") {
        out << "<span id='";
        Text *t = dynamic_cast<Text*>(command->at(1));
        if (t) {
            out << t->text;
        } else {
            errorLog->add(ErrorType::Error, article->sourceFile, "Invalid content in label text.");
        }
        out << "'></span>";
    } else if (command->command == "pr") {
        Text *name = dynamic_cast<Text*>(command->at(0));
        if (!name) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Label text may not contain commands.");
            return;
        }
    } else if (command->command == "pageref") {
        Text *name = dynamic_cast<Text*>(command->at(0));
        if (!name) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Label text may not contain commands.");
            return;
        }

        out << "(<a class='pageref' href='";
        auto iter = document->links.find(name->text);
        if (iter == document->links.end()) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Unknown link target \"" + name->text + "\".");
            out << "name->text";
        } else {
            out << iter->second.targetPage;
            if (iter->second.isFragment) {
                out << '#' << iter->second.name;
            }
        }
        out << "'>link</a>)";

    } else if (command->command == "emph") {
        out << "<i>";
        handle(command->children.front());
        out << "</i>";
    } else if (command->command == "textbf") {
        out << "<b>";
        handle(command->children.front());
        out << "</b>";
    } else if (command->command == "nexustext") {
        out << "<span class='nexustext'>";
        handle(command->children.front());
        out << "</span>";
    } else if (command->command == "vocab") {
        out << "<p><span class='nexustext'>";
        handle(command->at(1));
        out << "</span> <b>";
        handle(command->at(0));
        out << "</b> ";
        handle(command->at(2));
        out << " <i>";
        handle(command->at(3));
        out << ".</i> ";
        handle(command->at(4));
        out << "</p>";

    } else if (command->command == "narrowimage") {
        out << "<figure class='narrowimage'><img src='" << document->graphicsPath;
        handle(command->children.front());
        out << ".png'><br><caption>";
        handle(command->at(1));
        out << "</caption></figure>";
    } else if (command->command == "mediumimage") {
        out << "<figure class='mediumimage'><img src='" << document->graphicsPath;
        handle(command->children.front());
        out << ".png'><br><caption>";
        handle(command->at(1));
        out << "</caption></figure>";
    } else if (command->command == "wideimage") {
        out << "<figure class='wideimage'><img src='" << document->graphicsPath;
        handle(command->children.front());
        out << ".png'><br><caption>";
        handle(command->at(1));
        out << "</caption></figure>";

    } else if (command->command == "startinfobox") {
        out << "<table class='infobox'>";
    } else if (command->command == "stopinfobox") {
        out << "</table>";
    } else if (command->command == "infoline") {
        out << "<tr><td class='infoleft'>";
        handle(command->at(0));
        out << "</td><td class='inforight'>";
        handle(command->at(1));
        out << "</td></tr>";
    } else if (command->command == "infotitle") {
        out << "<tr><td colspan='2' class='infotitle'>";
        handle(command->at(0));
        out << "</td></tr>";
    } else if (command->command == "infohead") {
        out << "<tr><td colspan='2' class='infohead'>";
        handle(command->at(0));
        out << "</td></tr>";

    } else if (command->command == "startdblinfobox") {
        out << "<table class='dblinfobox'>";
    } else if (command->command == "stopdblinfobox") {
        out << "</table>";
    } else if (command->command == "dblinfoline") {
        out << "<tr><td class='dblinfoleft'>";
        handle(command->at(0));
        out << "</td><td class='dblinforight'>";
        handle(command->at(1));
        out << "</td><td class='dblinfoleft'>";
        handle(command->at(2));
        out << "</td><td class='dblinforight'>";
        handle(command->at(3));
        out << "</td></tr>";
    } else if (command->command == "dblinfotitle") {
        out << "<tr><td colspan='4' class='dblinfotitle'>";
        handle(command->at(0));
        out << "</td></tr>";

    } else if (command->command == "startorbittable") {
        out << "<table class='orbittable'>\n";
        out << "<tr><th>Orbit</th><th>Radius (AU)</th><th>Type</th><th>BB Temp (K)</th><th>Moonlets</th>\n";
    } else if (command->command == "stoporbittable") {
        out << "</table>\n";
    } else if (command->command == "planetrow") {
        out << "<tr class='planet'><td>";
        handle(command->at(0));
        out << "</td><td>";
        handle(command->at(1));
        out << "</td><td>";
        handle(command->at(2));
        out << "</td><td>";
        handle(command->at(3));
        out << "</td><td>";
        handle(command->at(4));
        out << "</td></tr>\n";
    } else if (command->command == "moonrow") {
        out << "<tr class='moon'><td>";
        handle(command->at(0));
        out << "</td><td></td><td>";
        handle(command->at(1));
        out << "</td></tr>\n";

    } else if (command->command == "begin") {
        Text *text = dynamic_cast<Text*>(command->at(0));
        if (!text) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Environment name must be text.");
        } else {
            if (text->text == "description")    out << "<ul>\n";
            else if (text->text == "itemize")   out << "<ol>\n";
        }
    } else if (command->command == "end") {
        Text *text = dynamic_cast<Text*>(command->at(0));
        if (!text) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Environment name must be text.");
        } else {
            if (text->text == "description")    out << "</ul>\n";
            else if (text->text == "itemize")   out << "</ol>\n";
        }
    } else if (command->command == "item") {
        out << "<li>";
        if (command->size() > 0) {
            out << "<span class='lihead'>";
            handle(command->at(0));
            out << "</span>";
        }

    } else if (command->command == "LaTeX") {
        out << "LaTeX";
    } else if (command->command == "degree") {
        out << "&deg;";
    } else if (command->command == "times") {
        out << "&times;";
    } else if (command->command == "LaTeX") {
        out << "LaTeX";
    } else if (command->command == "parbox") {
        handle(command->at(1));
    } else if (command->command == "clearpage" || command->command == "textwidth"
            || command->command == "linewidth") {
        // do nothing
    } else {
        for (Node *c : command->children) {
            handle(c);
        }
    }
}

void FormatDocument::handle(Paragraph *paragraph) {
    out << "<p>";
    for (Node *c : paragraph->children) {
        handle(c);
    }
    out << "\n\n";
}
