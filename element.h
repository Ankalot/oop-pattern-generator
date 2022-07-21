#ifndef ELEMENT_H
#define ELEMENT_H

#include "baseelement.h"

#include <QString>
#include <QHash>
#include <QMap>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Parser { class Element; }
QT_END_NAMESPACE

class Element: public BaseElement
{
public:
    // includes: fileName -> vector of positions of element in it
    Element(const QString &text, const QHash<QString, QVector<int>> &includes);
    ~Element() override;

    void addInclude(const QString &fileName, const QVector<int> &poses);
    void setText(const QString &newText);
    const QString &getText();
    const QString &getInitText();

    static QMap<int, Element *> getSortedElementsFromSource(const QString &sourceFileName);

private:
    QString text;
    QString initText;
    QHash<QString, QVector<int>> includes;
    int index;

    inline static QVector<Element *> allElements;

};

#endif // ELEMENT_H
