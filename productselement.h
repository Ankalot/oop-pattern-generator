#ifndef PRODUCTSELEMENT_H
#define PRODUCTSELEMENT_H

#include "baseelement.h"
#include "classmethod.h"

QT_BEGIN_NAMESPACE
namespace Parser { class ProductsElement; }
QT_END_NAMESPACE

class Element;

// coming soon, this is needed for abstract factory parser
class ProductsElement: BaseElement
{
public:
    ProductsElement();

private:
    QVector<QVector<ClassMethod<Element *>>>  products;
};

#endif // PRODUCTSELEMENT_H
