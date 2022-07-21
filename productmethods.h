#ifndef PRODUCTSELEMENT_H
#define PRODUCTSELEMENT_H

#include "baseelement.h"
#include "classmethod.h"

QT_BEGIN_NAMESPACE
namespace Parser { class ProductMethods; }
QT_END_NAMESPACE

class Element;

class ProductMethods: public BaseElement
{
public:
    ProductMethods(unsigned num = 0);
    ~ProductMethods() override;

    ClassMethod<Element *> *&operator[](unsigned i);

    void addMethod(ClassMethod<Element *> *method);
    void setMethod(int index, ClassMethod<Element *> *method);
    int getCount();

private:
    QVector<ClassMethod<Element *> *>  methods;
};

#endif // PRODUCTSELEMENT_H
