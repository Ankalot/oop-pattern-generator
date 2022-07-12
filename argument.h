#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QString>

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class Argument; }
QT_END_NAMESPACE

class Argument {
public:
    Argument(bool isConst, QString type, QString name);

    bool constFlag() const;
    const QString &getType() const;
    const QString &getName() const;

private:
    bool isConst;
    QString type;
    QString name;
};
#endif // ARGUMENT_H
