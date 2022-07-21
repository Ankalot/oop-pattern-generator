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

    BaseElement *&operator[](unsigned i);

    void addElement(BaseElement *element);
    void setElement(int index, BaseElement *element);
    int getCount();

private:
    QVector<BaseElement *> elements;

};

#endif // VECTORELEMENT_H
