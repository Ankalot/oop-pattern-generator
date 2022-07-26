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
    enum PATTERN_TYPE { SINLETON = 1, ABSTRACT_FACTORY, BUILDER };
    int patternType;
    QHash<QString, QVector<ClassText *>> parseData;
    QHash<QString, BaseElement *> elements;

    bool writeElementsToFile(ClassText *classText);
    bool rewriteSingletonInFiles();
    bool rewriteAbstractFactoryInFiles();
    bool rewriteBuilderInFiles();

    void parseSingleton();
    void parseAbstractFactory();
    void parseBuilder();
};

#endif // PARSEDELEMENTS_H
