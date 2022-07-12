#include "classmethod.h"
#include "argument.h"

ClassMethod::ClassMethod(bool isConst, QString type, QString name, int argsNum) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
    this->argsNum = argsNum;
    arguments.resize(argsNum);
}

ClassMethod::~ClassMethod() {
    qDeleteAll(arguments);
    arguments.clear();
}

void ClassMethod::addArgument(Argument *arg, int i) {
    arguments[i] = arg;
}

bool ClassMethod::constFlag() const {
    return isConst;
}

const QString &ClassMethod::getType() const {
    return type;
}

const QString &ClassMethod::getName() const {
    return name;
}

int ClassMethod::getArgsNum() const {
    return argsNum;
}

Argument *ClassMethod::getArgument(int i) const {
    return arguments[i];
}
