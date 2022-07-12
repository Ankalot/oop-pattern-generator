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

    bool constFlag();
    QString getType();
    QString getName();
    int getArgsNum();
    Argument *getArgument(int i);

private:
    bool isConst;
    QString type;
    QString name;
    int argsNum;
    QVector<Argument *>arguments;
};
#endif // CLASSMETHOD_H
