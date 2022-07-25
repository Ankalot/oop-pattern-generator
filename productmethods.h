#ifndef PRODUCTSELEMENT_H
#define PRODUCTSELEMENT_H

#include "baseelement.h"
#include "classmethod.h"

QT_BEGIN_NAMESPACE
namespace Parser { class ProductMethods; }
QT_END_NAMESPACE

using ClassMethodTypePtr = ClassMethod<std::shared_ptr<Element>> *;

class Element;

class ProductMethods: public BaseElement
{
public:
    ProductMethods(unsigned num = 0);
    ~ProductMethods() override;

    ClassMethodTypePtr &operator[](unsigned i);

    void addMethod(ClassMethodTypePtr method);
    void setMethod(int index, ClassMethodTypePtr method);
    int getCount();

private:
    QVector<ClassMethodTypePtr>  methods;
};

#endif // PRODUCTSELEMENT_H
