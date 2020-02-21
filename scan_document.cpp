#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "latexwiki.h"

ScanDocument::ScanDocument(Document *document)
: document(document)
{ }

void ScanDocument::handle(Node *node) {
    if (node) node->handle(this);
}

void ScanDocument::handle(Fragment *fragment) {
    for (Node *c : fragment->children) {
        handle(c);
    }
}

void ScanDocument::handle(Text *text) {
}

void ScanDocument::handle(Command *command) {
    if (command->command == "label") {
        errorLog->add(ErrorType::Warning, article->sourceFile, "Avoid use of \\label command.");
        Text *name = dynamic_cast<Text*>(command->children[0]);
        if (!name) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Label text may not contain commands.");
            return;
        }

        LinkTarget entry = { name->text, article->filename, name->text, true };
        document->addLink(entry, *errorLog);
    } else if (command->command == "addlabel") {
        Text *name = dynamic_cast<Text*>(command->at(0));
        if (!name) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Label text may not contain commands.");
            return;
        }
        Text *target = dynamic_cast<Text*>(command->at(1));
        if (!target) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Label target may not contain commands.");
            return;
        }

        LinkTarget entry = { target->text, article->filename, name->text, true };
        document->addLink(entry, *errorLog);
    } else if (command->command == "pageinfo") {
        article->hasPageInfo = true;
        Text *title = dynamic_cast<Text*>(command->at(0));
        if (!title) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Page title may not contain commands.");
            return;
        }
        article->name = title->text;

        Text *name = dynamic_cast<Text*>(command->at(1));
        if (!name) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Page label may not contain commands.");
            return;
        }

        LinkTarget entry = { name->text, article->filename, article->name, false };
        document->addLink(entry, *errorLog);

        Text *world = dynamic_cast<Text*>(command->at(2));
        if (!world) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Page world may not contain commands.");
            return;
        }
        article->world = world->text;
        document->worlds[world->text].push_back(article);

        Text *category = dynamic_cast<Text*>(command->at(3));
        if (!category) {
            errorLog->add(ErrorType::Error, article->sourceFile, "Page category may not contain commands.");
            return;
        }
        article->category = category->text;
        document->categories[category->text].push_back(article);

    } else {
        for (Node *c : command->children) {
            handle(c);
        }
    }
}

void ScanDocument::handle(Paragraph *paragraph) {
    for (Node *c : paragraph->children) {
        handle(c);
    }
}
