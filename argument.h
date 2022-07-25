#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QString>
#include <cstdio>
#include <memory>
#include "element.h"

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class Argument; }
QT_END_NAMESPACE

using ElementPtr = std::shared_ptr<Element>;

template<class T>
class Argument {
public:
    Argument(std::conditional_t<std::is_same<T, ElementPtr>::value, ElementPtr, bool> isConst, T type, T name);
    ~Argument();

    std::conditional_t<std::is_same<T, ElementPtr>::value, ElementPtr, bool> constFlag() const;
    const T &getType() const;
    const T &getName() const;

private:
    std::conditional_t<std::is_same<T, ElementPtr>::value, ElementPtr, bool> isConst;
    T type;
    T name;
};

template<class T>
Argument<T>::~Argument() { }

template<class T>
Argument<T>::Argument(std::conditional_t<std::is_same<T, ElementPtr>::value, ElementPtr, bool> isConst, T type, T name) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
}

template<class T>
std::conditional_t<std::is_same<T, ElementPtr>::value, ElementPtr, bool> Argument<T>::constFlag() const {
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
