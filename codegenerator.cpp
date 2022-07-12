#include "codegenerator.h"
#include "classmethod.h"
#include "argument.h"

#include <QDebug>

QString CodeGenerator::genSingleton(const QString &className) {
    QString singletoneText = QString("\
class %1 {\n\n\
public:\n\
    static %1& getInstance() {\n\
        static %1 instance;\n\
        return instance;\n\
    }\n\n\
    %1(const %1 &obj) = delete;\n\
    ~%1() = default;\n\
    %1& operator=(const %1 &obj) = delete;\n\n\
    void printCurrentAddress() {\n\
        std::cout << this << '\\n';\n\
    }\n\n\
private:\n\n\
    %1() = default;\n\
    int m_number = 0;\n\n\
};\n").arg(className);
    return singletoneText;
};

QString makeProductArgText(Argument *argument) {
    QString constText = "";
    if (argument->constFlag())
        constText = "const ";
    return constText + argument->getType() + " " + argument->getName();
}

QString makeProductMethodsText(QVector<ClassMethod *> &productMethods, const QString &s1, const QString &s2) {
    QString productMethodsText = "";
    for (int methodIndex = 0; methodIndex < productMethods.count(); ++methodIndex) {
        productMethodsText += "    " + s1;
        if (productMethods[methodIndex]->constFlag()) {
            productMethodsText += "const ";
        }
        productMethodsText += productMethods[methodIndex]->getType();
        productMethodsText += " " + productMethods[methodIndex]->getName() + "(";
        const int argsNum = productMethods[methodIndex]->getArgsNum();
        for (int argIndex = 0; argIndex < argsNum-1; ++argIndex) {
            productMethodsText += makeProductArgText(productMethods[methodIndex]->getArgument(argIndex)) + ", ";
        }
        if (argsNum != 0) {
            productMethodsText += makeProductArgText(productMethods[methodIndex]->getArgument(argsNum-1));
        }
        productMethodsText += ")" + s2 + "\n";
    }
    return productMethodsText;
}

QString makeProductsText(QVector<QVector<ClassMethod *>> &productsMethods, QVector<QString> &products, QVector<QString> &factories) {
    QString productsText = "";
    for (int productIndex = 0; productIndex < products.count(); ++productIndex) {
        const QString productName = products[productIndex];
        const QString productMethodsText = makeProductMethodsText(productsMethods[productIndex], "virtual ",  " = 0;");
        const QString productText = QString("class %1 {\n\
public:\n\
    virtual const std::string getName() = 0;\n\
    virtual ~%1() = default;\n\n\
%2\
};\n\n").arg(productName).arg(productMethodsText);
        productsText += productText;

        const QString productFactoryMethodsText = makeProductMethodsText(productsMethods[productIndex], "", " override { }");
        for (int factoryIndex = 0; factoryIndex < factories.count(); ++factoryIndex) {
            const QString factoryName = factories[factoryIndex];
            const QString productFactoryText = QString("class %1%2: public %1 {\n\
public:\n\
    const std::string getName() override {\n\
        return \"%1%2\";\n\
    }\n\n\
%3\
};\n\n").arg(productName).arg(factoryName).arg(productFactoryMethodsText);
            productsText += productFactoryText;
        }
    }
    return productsText;
}

QString getAbstractFactoryMethodsText(QVector<QString> &products, int &pointerType) {
    QString abstractFactoryMethodsText = "";
    QString resultType;
    switch(pointerType) {
        case 0:
            resultType = "%1*";
            break;
        case 1:
            resultType = "std::unique_ptr<%1>";
            break;
        case 2:
            resultType = "std::shared_ptr<%1>";
            break;
        default:
            qWarning() << "Unexpected pointer type index";
            resultType = "?";
            break;
    }
    for (int productIndex = 0; productIndex < products.count(); ++productIndex) {
        const QString productName = products[productIndex];
        abstractFactoryMethodsText += QString("    virtual %1 factoryMethod%2() = 0;\n").arg(resultType.arg(productName)).arg(productName);
    }
    return abstractFactoryMethodsText;
}

QString getFactoryMethodsText(QVector<QString> &products, const QString &factoryName, int &pointerType) {
    QString factoryMethodsText = "";
    QString resultMethodType, resultReturnObj;
    switch (pointerType) {
        case 0:
            resultMethodType = "%1*";
            resultReturnObj = "new %1%2";
            break;
        case 1:
            resultMethodType = "std::unique_ptr<%1>";
            resultReturnObj = "std::make_unique<%1%2>()";
            break;
        case 2:
            resultMethodType = "std::shared_ptr<%1>";
            resultReturnObj = "std::make_shared<%1%2>()";
            break;
        default:
            qWarning() << "Unexpected pointer type index";
            resultMethodType = "?";
            resultReturnObj = "?";
            break;
    }
    for (int productIndex = 0; productIndex < products.count(); ++productIndex) {
        const QString productName = products[productIndex];
        const QString factoryMethodText = QString("\
    %1 factoryMethod%2() {\n\
        return %3;\n\
    }\n\n").arg(resultMethodType.arg(productName)).arg(productName).arg(resultReturnObj.arg(productName).arg(factoryName));
            factoryMethodsText += factoryMethodText;
    }
    return factoryMethodsText;
}

QString makeFactoriesText(QVector<QString> &products, QVector<QString> &factories, int &pointerType) {
    const QString abstractFactoryMethodsText = getAbstractFactoryMethodsText(products, pointerType);
    QString factoriesText = QString("class Factory {\n\
public:\n\
%1\
};\n\n").arg(abstractFactoryMethodsText);

    for (int factoryIndex = 0; factoryIndex < factories.count(); ++factoryIndex) {
        const QString factoryName = factories[factoryIndex];
        const QString factoryMethodsText = getFactoryMethodsText(products, factoryName, pointerType);
        const QString factoryClassText = QString("class %1: public Factory {\n\
public:\n\
%2\
};\n\n").arg(factoryName).arg(factoryMethodsText);
        factoriesText += factoryClassText;
    }
    return factoriesText;
}

QString CodeGenerator::genAbstractFactory(int &pointerType, QVector<QString> &factories, QVector<QString> &products,
                                          QVector<QVector<ClassMethod *>> &productsMethods) {
    const QString productsText = makeProductsText(productsMethods, products, factories);
    const QString factoriesText = makeFactoriesText(products, factories, pointerType);
    return (productsText + factoriesText);
}
