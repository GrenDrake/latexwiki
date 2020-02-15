#include "latexwiki.h"

Node::~Node() {
    for (Node *node : children) delete node;
}

void Node::add(Node *n) {
    if (!n) return;
    children.push_back(n);
}

void Node::add(Fragment *n) {
    if (!n) {
        children.push_back(new Text(""));
        return;
    }
    if (n->size() == 1) {
        children.push_back(n->children[0]);
        n->children.clear();
        delete n;
    } else {
        children.push_back(n);
    }
}

bool Node::isEmpty() const {
    return children.empty();
    }

int Node::size() const {
    return children.size();
}

Node* Node::at(unsigned idx) {
    if (idx >= children.size()) return nullptr;
    return children[idx];
}


Text::Text(const std::string &s)
: text(s)
{ }

void Text::handle(DocumentProcessor *processor) {
    processor->handle(this);
}

void Fragment::handle(DocumentProcessor *processor) {
    processor->handle(this);
}

void Command::handle(DocumentProcessor *processor) {
    processor->handle(this);
}

void Paragraph::handle(DocumentProcessor *processor) {
    processor->handle(this);
}

Article::Article()
: hasPageInfo(false)
{ }

Article::~Article() {
    for (Paragraph *p : paragraphs) delete p;
}

void Article::add(Paragraph *n) {
    if (!n) return;
    paragraphs.push_back(n);
}

void Article::process(DocumentProcessor &processor) {
    for (Paragraph *p : paragraphs) {
        processor.handle(p);
    }
}


void Document::addLink(const LinkTarget &target, ErrorLog &errorLog) {
    auto existing = links.find(target.name);
    if (existing != links.end()) {
        errorLog.add(ErrorType::Error, target.targetPage, "Cannot add label: label already exists");
        return;
    }

    links.insert(std::make_pair(target.name, target));
}

Article* Document::byFile(const std::string &filename) {
    for (Article *iter : articles) {
        if (iter->filename == filename) {
            return iter;
        }
    }
    return nullptr;
}
