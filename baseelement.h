#ifndef BASEELEMENT_H
#define BASEELEMENT_H

#include <Qt>

QT_BEGIN_NAMESPACE
namespace Parser { class BaseElement; }
QT_END_NAMESPACE

class BaseElement
{
public:
    BaseElement();
    virtual ~BaseElement() = 0;
};

#endif // BASEELEMENT_H
