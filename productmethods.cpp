#include "productmethods.h"

#include <QDebug>

ProductMethods::ProductMethods(unsigned num) {
    methods = QVector<ClassMethod<Element *> *>(num);
}

ProductMethods::~ProductMethods() {
    qDeleteAll(methods);
}

ClassMethod<Element *> *&ProductMethods::operator[](unsigned i) {
    return methods[i];
}

void ProductMethods::addMethod(ClassMethod<Element *> *method) {
    methods.append(method);
}

void ProductMethods::setMethod(int index, ClassMethod<Element *> *method) {
    if (index < methods.count())
        methods[index] = method;
    else
        qWarning() << "Vector out of bounds";
}

int ProductMethods::getCount() {
    return methods.count();
}
