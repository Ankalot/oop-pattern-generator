#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QString>
#include "classmethod.h"

class ClassText;

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class CodeGenerator; }
QT_END_NAMESPACE

class CodeGenerator {
public:
    CodeGenerator(bool includeGuard);

    // .cpp
    void genSingleton(QString *text, const QString &className) const;
    // .h and .cpp
    void genSingleton(QString *text1, QString *text2, const QString &className) const;

    // .cpp
    void genAbstractFactory(QString *text, const int &pointerType, const QString &abstractFactoryName, QVector<QString> &factories,
                            QVector<QString> &products, QVector<QVector<ClassMethod<QString> *>> &productsMethods) const;
    // .h and .cpp for each class
    void genAbstractFactory(QVector<ClassText *> *classTexts, const int &pointerType, const QString &abstractFactoryName,
                            QVector<QString> &factories, QVector<QString> &products,
                            QVector<QVector<ClassMethod<QString> *>> &productsMethods) const;

private:
    bool includeGuard;
    QString includeGuardText1;
    QString includeGuardText2;

    void genAbstractFactoryProductsClassesHandCpp(QVector<ClassText *> *classTexts, QVector<QString> &factories, QVector<QString> &products,
                                                  QVector<QVector<ClassMethod<QString> *>> &productsMethods,
                                                  const int &productsNum, const int &factoriesNum, int *classTextCounter) const;
    void genAbstractFactoryFactoriesClassesHandCpp(QVector<ClassText *> *classTexts, const QString &abstractFactoryName,
                                                   QVector<QString> &factories, QVector<QString> &products, const int &pointerType,
                                                   const int &factoriesNum, int *classTextCounter) const;

};
#endif // CODEGENERATOR_H
