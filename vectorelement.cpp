#include "vectorelement.h"

#include <QDebug>

VectorElement::VectorElement(int num): BaseElement() {
    elements.resize(num);
}

VectorElement::~VectorElement() {
    qDeleteAll(elements);
}

void VectorElement::addElement(BaseElement *element) {
    elements.append(element);
}

void VectorElement::setElement(int index, BaseElement *element) {
    if (index < elements.count())
        elements[index] = element;
    else
        qWarning() << "Vector out of bounds";
}

BaseElement *&VectorElement::operator[](unsigned i) {
    return elements[i];
}

int VectorElement::getCount() {
    return elements.count();
}
