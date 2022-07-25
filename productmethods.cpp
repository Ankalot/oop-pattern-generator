#include "productmethods.h"

#include <QDebug>

ProductMethods::ProductMethods(unsigned num) {
    methods = QVector<ClassMethodTypePtr>(num);
}

ProductMethods::~ProductMethods() {
    qDeleteAll(methods);
}

ClassMethodTypePtr &ProductMethods::operator[](unsigned i) {
    return methods[i];
}

void ProductMethods::addMethod(ClassMethodTypePtr method) {
    methods.append(method);
}

void ProductMethods::setMethod(int index, ClassMethodTypePtr method) {
    if (index < methods.count())
        methods[index] = method;
    else
        qWarning() << "Vector out of bounds";
}

int ProductMethods::getCount() {
    return methods.count();
}
