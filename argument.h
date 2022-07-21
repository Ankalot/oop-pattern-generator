#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QString>
#include "stdio.h"
#include "element.h"

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class Argument; }
QT_END_NAMESPACE

template<class T>
class Argument {
public:
    Argument(std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst, T type, T name);
    ~Argument();

    std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> constFlag() const;
    const T &getType() const;
    const T &getName() const;

private:
    std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst;
    T type;
    T name;
};

template<class T>
Argument<T>::~Argument() {
    if constexpr (std::is_same<T, Element *>::value)
        delete isConst;
    if constexpr (std::is_pointer<T>::value) {
        delete type;
        delete name;
    }
}

template<class T>
Argument<T>::Argument(std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst, T type, T name) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
}

template<class T>
std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> Argument<T>::constFlag() const {
    return isConst;
}

template<class T>
const T &Argument<T>::getType() const {
    return type;
}

template<class T>
const T &Argument<T>::getName() const {
    return name;
}

#endif // ARGUMENT_H
