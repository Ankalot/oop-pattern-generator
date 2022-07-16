#ifndef CLASSMETHOD_H
#define CLASSMETHOD_H

#include <QVector>

#include "argument.h"

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class ClassMethod; }
QT_END_NAMESPACE

template<class T>
class ClassMethod {
public:
    ClassMethod(bool isConst, T type, T name, int argsNum);
    ~ClassMethod();

    void addArgument(Argument<T> *arg, int i);

    bool constFlag() const;
    const T &getType() const;
    const T &getName() const;
    int getArgsNum() const;
    Argument<T> *getArgument(int i) const;

private:
    bool isConst;
    T type;
    T name;
    int argsNum;
    QVector<Argument<T> *>arguments;
};

template<class T>
ClassMethod<T>::ClassMethod(bool isConst, T type, T name, int argsNum) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
    this->argsNum = argsNum;
    arguments.resize(argsNum);
}

template<class T>
ClassMethod<T>::~ClassMethod() {
    qDeleteAll(arguments);
    arguments.clear();
}

template<class T>
void ClassMethod<T>::addArgument(Argument<T> *arg, int i) {
    arguments[i] = arg;
}

template<class T>
bool ClassMethod<T>::constFlag() const {
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
    return argsNum;
}

template<class T>
Argument<T> *ClassMethod<T>::getArgument(int i) const {
    return arguments[i];
}

#endif // CLASSMETHOD_H
