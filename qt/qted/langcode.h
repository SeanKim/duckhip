#ifndef LANGCODE_H
#define LANGCODE_H

QHash<QString, QString> STATUS_CODES;


#define STATUS_CODE(lang, code) STATUS_CODES.insert(lang, code);
STATUS_CODE("", "Continue")


#endif // LANGCODE_H
