#include "classmethod.h"
#include "argument.h"

ClassMethod::ClassMethod(bool isConst, QString type, QString name, int argsNum) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
    this->argsNum = argsNum;
    this->arguments.resize(argsNum);
}

ClassMethod::~ClassMethod() {
    qDeleteAll(arguments);
    arguments.clear();
}

void ClassMethod::addArgument(Argument *arg, int i) {
    this->arguments[i] = arg;
}

bool ClassMethod::constFlag() {
    return this->isConst;
}

QString ClassMethod::getType() {
    return this->type;
}

QString ClassMethod::getName() {
    return this->name;
}

int ClassMethod::getArgsNum() {
    return this->argsNum;
}

Argument *ClassMethod::getArgument(int i) {
    return this->arguments[i];
}
