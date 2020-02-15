
#include "latexwiki.h"

ErrorLog::ErrorLog()
: warnCount(0), errorCount(0), fatalCount(0)
{ }

void ErrorLog::add(ErrorType type, const std::string &file, const std::string &msg) {
    switch(type) {
        case ErrorType::Warning:    ++warnCount;    break;
        case ErrorType::Error:      ++errorCount;   break;
        case ErrorType::Fatal:      ++fatalCount;   break;
    }
    errors.push_back(ErrorMsg{type, file, msg});
}

bool ErrorLog::hasErrors() const {
    return errorCount > 0 || fatalCount > 0;
}

bool ErrorLog::isEmpty() const {
    return errors.empty();
}
