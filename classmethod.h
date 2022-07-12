#ifndef CLASSMETHOD_H
#define CLASSMETHOD_H

#include <QVector>

class Argument;

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class ClassMethod; }
QT_END_NAMESPACE

class ClassMethod {
public:
    ClassMethod(bool isConst, QString type, QString name, int argsNum);
    ~ClassMethod();

    void addArgument(Argument *arg, int i);

    bool constFlag() const;
    const QString &getType() const;
    const QString &getName() const;
    int getArgsNum() const;
    Argument *getArgument(int i) const;

private:
    bool isConst;
    QString type;
    QString name;
    int argsNum;
    QVector<Argument *>arguments;
};
#endif // CLASSMETHOD_H
