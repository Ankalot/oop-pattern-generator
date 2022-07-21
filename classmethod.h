#ifndef CLASSMETHOD_H
#define CLASSMETHOD_H

#include <QVector>
#include "stdio.h"

#include "argument.h"
#include "element.h"

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class ClassMethod; }
QT_END_NAMESPACE

template<class T>
class ClassMethod {
public:
    ClassMethod(std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst, T type, T name, int argsNum = 0);
    ~ClassMethod();

    void setArgument(Argument<T> *arg, int i);
    void addArgument(Argument<T> *arg);

    std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> constFlag() const;
    const T &getType() const;
    const T &getName() const;
    int getArgsNum() const;
    Argument<T> *getArgument(int i) const;

private:
    std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst;
    T type;
    T name;
    int argsNum;
    QVector<Argument<T> *>arguments;
};

template<class T>
ClassMethod<T>::ClassMethod(std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> isConst, T type, T name, int argsNum) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
    arguments.resize(argsNum);
}

template<class T>
ClassMethod<T>::~ClassMethod() {
    if constexpr (std::is_same<T, Element *>::value)
        delete isConst;
    if constexpr (std::is_pointer<T>::value) {
        delete type;
        delete name;
    }
    qDeleteAll(arguments);
    arguments.clear();
}

template<class T>
void ClassMethod<T>::setArgument(Argument<T> *arg, int i) {
    arguments[i] = arg;
}

template<class T>
void ClassMethod<T>::addArgument(Argument<T> *arg) {
    arguments.append(arg);
}

template<class T>
std::conditional_t<std::is_same<T, Element *>::value, Element*, bool> ClassMethod<T>::constFlag() const {
    return isConst;
}

template<class T>
const T &ClassMethod<T>::getType() const {
    return type;
}

template<class T>
const T &ClassMethod<T>::getName() const {
    return name;
}

template<class T>
int ClassMethod<T>::getArgsNum() const {
    return arguments.count();
}

template<class T>
Argument<T> *ClassMethod<T>::getArgument(int i) const {
    return arguments[i];
}

#endif // CLASSMETHOD_H
