#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QString>

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class Argument; }
QT_END_NAMESPACE

class Argument {
public:
    Argument(bool isConst, QString type, QString name);

    bool constFlag();
    QString getType();
    QString getName();

private:
    bool isConst;
    QString type;
    QString name;
};
#endif // ARGUMENT_H
