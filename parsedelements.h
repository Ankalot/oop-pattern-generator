#ifndef PARSEDELEMENTS_H
#define PARSEDELEMENTS_H

#include <QHash>

QT_BEGIN_NAMESPACE
namespace Parser { class ParsedElements; }
QT_END_NAMESPACE

class ClassText;
class BaseElement;

class ParsedElements
{
public:
    ParsedElements(int patternType, const QHash<QString, QVector<ClassText *>> &parseData);
    ~ParsedElements();

    QHash<QString, BaseElement *> getElements();

    bool rewriteInFiles();
    bool isOk();

private:
    bool ok = false;
    enum PATTERN_TYPE { SINLETON = 1, ABSTRACT_FACTORY };
    int patternType;
    QHash<QString, QVector<ClassText *>> parseData;
    QHash<QString, BaseElement *> elements;

    bool rewriteSingletonInFiles();
    bool rewriteAbstractFactoryInFiles();

    void parseSingleton();
    void parseAbstractFactory();
};

#endif // PARSEDELEMENTS_H
