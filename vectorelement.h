#ifndef VECTORELEMENT_H
#define VECTORELEMENT_H

#include "baseelement.h"
#include "element.h"

#include <QVector>

QT_BEGIN_NAMESPACE
namespace Parser { class VectorElement; }
QT_END_NAMESPACE

class VectorElement: public BaseElement
{
public:
    VectorElement(int num = 0);
    ~VectorElement() override;

    Element *&operator[](unsigned i);

    void addElement(Element *element);
    void setElement(int index, Element *element);
    int getCount();

private:
    QVector<Element *> elements;

};

#endif // VECTORELEMENT_H
