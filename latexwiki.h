#ifndef CONVERT_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

struct Node;
struct Fragment;
struct Text;
struct Command;
struct Paragraph;
struct Article;
struct Document;
struct ErrorLog;

struct DocumentProcessor {
    virtual void handle(Node*) = 0;
    virtual void handle(Fragment*) = 0;
    virtual void handle(Text*) = 0;
    virtual void handle(Command*) = 0;
    virtual void handle(Paragraph*) = 0;

    Article *article;
    ErrorLog *errorLog;
};

struct DumpDocument : public DocumentProcessor {
    DumpDocument(std::ostream &out);
    virtual void handle(Node*);
    virtual void handle(Fragment*);
    virtual void handle(Text*);
    virtual void handle(Command*);
    virtual void handle(Paragraph*);

    std::ostream &out;
    int depth;
};

struct FormatDocument : public DocumentProcessor {
    FormatDocument(Document *article, std::ostream &out);
    virtual void handle(Node*);
    virtual void handle(Fragment*);
    virtual void handle(Text*);
    virtual void handle(Command*);
    virtual void handle(Paragraph*);

    std::ostream &out;
    Document *document;
};

struct ScanDocument : public DocumentProcessor {
    ScanDocument(Document *article);
    virtual void handle(Node*);
    virtual void handle(Fragment*);
    virtual void handle(Text*);
    virtual void handle(Command*);
    virtual void handle(Paragraph*);

    Document *document;
};

struct Node {
    virtual ~Node();
    virtual void handle(DocumentProcessor*) = 0;

    void add(Node *n);
    void add(Fragment *n);
    bool isEmpty() const;
    int size() const;
    Node* at(unsigned idx);

    std::vector<Node*> children;
};

struct LinkTarget {
    std::string name;
    std::string targetPage;
    std::string displayText;
    bool isFragment;
};


struct Text : public Node {
    Text(const std::string &s);
    virtual void handle(DocumentProcessor *processor) override;

    std::string text;
};


struct Fragment : public Node {
    virtual void handle(DocumentProcessor *processor) override;
};

struct Command : public Node {
    virtual void handle(DocumentProcessor *processor) override;

    std::string command;
};

struct Paragraph : public Node {
    virtual void handle(DocumentProcessor *processor) override;
};


struct Article {
    Article();
    ~Article();
    void add(Paragraph *n);
    void process(DocumentProcessor &processor);

    std::string sourceFile;
    std::string name, filename, world, category;
    std::vector<Paragraph*> paragraphs;
    bool hasPageInfo;
};

struct Document {
    void addLink(const LinkTarget &target, ErrorLog &errorLog);
    Article* byFile(const std::string &filename);

    std::vector<Article*> articles;
    std::map<std::string, LinkTarget> links;
    std::string graphicsPath;
    std::map<std::string, std::vector<Article*>> categories;
    std::map<std::string, std::vector<Article*>> worlds;
};

struct CommandInfo {
    std::string name;
    int minArgs, maxArgs;
};

enum class ErrorType {
    Warning, Error, Fatal
};

struct ErrorMsg {
    ErrorType type;
    std::string sourceFile;
    std::string message;
};

struct ErrorLog {
    ErrorLog();
    void add(ErrorType type, const std::string &file, const std::string &msg);
    bool hasErrors() const;
    bool isEmpty() const;

    int warnCount, errorCount, fatalCount;
    std::vector<ErrorMsg> errors;
};

std::string& trim(std::string &text);
Article* processFile(const std::string &sourceFile, ErrorLog &errorLog);
int g_toupper(int c);
bool is_identifier(char c);
std::string& replaceText(std::string &text, const std::string &from, const std::string &to);
std::string readFile(const std::string &filename);

void make_indexes(const std::string &pageTop, const std::string &pageBottom, Document &document);

extern bool showMissingWorld;
extern bool showMissingCategory;

#endif
