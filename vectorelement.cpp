#include "vectorelement.h"

#include <QDebug>

VectorElement::VectorElement(int num): BaseElement() {
    elements.resize(num);
}

VectorElement::~VectorElement() {
    qDeleteAll(elements);
}

void VectorElement::addElement(Element *element) {
    elements.append(element);
}

void VectorElement::setElement(int index, Element *element) {
    if (index < elements.count())
        elements[index] = element;
    else
        qWarning() << "Vector out of bounds";
}

Element *VectorElement::getElement(int index) {
    return elements[index];
}

int VectorElement::getCount() {
    return elements.count();
}