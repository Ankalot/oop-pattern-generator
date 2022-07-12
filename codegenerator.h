#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QString>

class ClassMethod;

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class CodeGenerator; }
QT_END_NAMESPACE

class CodeGenerator {
public:
    CodeGenerator() = default;

    QString genSingleton(const QString &className) const;
    QString genAbstractFactory(int &pointerType, QVector<QString> &factories, QVector<QString> &products,
                               QVector<QVector<ClassMethod *>> &productsMethods) const;
};
#endif // CODEGENERATOR_H
