#include "classmethods.h"

#include <QDebug>

ClassMethods::ClassMethods(unsigned num) {
    methods = QVector<ClassMethodTypePtr>(num);
}

ClassMethods::~ClassMethods() {
}

ClassMethodTypePtr &ClassMethods::operator[](unsigned i) {
    return methods[i];
}

void ClassMethods::addMethod(ClassMethodTypePtr method) {
    methods.append(method);
}

void ClassMethods::setMethod(int index, ClassMethodTypePtr method) {
    if (index < methods.count())
        methods[index] = method;
    else
        qWarning() << "Vector out of bounds";
}

int ClassMethods::getCount() {
    return methods.count();
}
