#include "argument.h"

Argument::Argument(bool isConst, QString type, QString name) {
    this->isConst = isConst;
    this->type = type;
    this->name = name;
}

bool Argument::constFlag() const {
    return isConst;
}

const QString &Argument::getType() const {
    return type;
}

const QString &Argument::getName() const {
    return name;
}
