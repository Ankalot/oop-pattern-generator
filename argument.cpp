#include "argument.h"

Argument::Argument(bool isConst, QString type, QString name) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
}

bool Argument::constFlag() {
    return this->isConst;
}

QString Argument::getType() {
    return this->type;
}

QString Argument::getName() {
    return this->name;
}
