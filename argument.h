#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QString>

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class Argument; }
QT_END_NAMESPACE

template<class T>
class Argument {
public:
    Argument(bool isConst, T type, T name);

    bool constFlag() const;
    const T &getType() const;
    const T &getName() const;

private:
    bool isConst;
    T type;
    T name;
};

template<class T>
Argument<T>::Argument(bool isConst, T type, T name) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
}

template<class T>
bool Argument<T>::constFlag() const {
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
