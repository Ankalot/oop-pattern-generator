#include "codegenerator.h"
#include "classtext.h"
#include "argument.h"

#include <cassert>
#include <QDebug>

CodeGenerator::CodeGenerator(bool includeGuard) {
    this->includeGuard = includeGuard;
    if (includeGuard) {
        includeGuardText1 = "#ifndef %1\n#define %1\n\n";
        includeGuardText2 = "#endif\n";
    } else {
        includeGuardText1 = "";
        includeGuardText2 = "";
    }
}

void CodeGenerator::genSingleton(QString *text, const QString &className) const {
    *text = includeGuardText1.arg(className) + QString("\
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
private:\n\
    %1() = default;\n\
    int m_number = 0;\n\n\
};\n").arg(className) + includeGuardText2;
}

void CodeGenerator::genSingleton(QString *text1, QString *text2, const QString &className) const {
    *text1 = includeGuardText1.arg(className) + QString("\
class %1 {\n\n\
public:\n\
    static %1& getInstance();\n\n\
    %1(const %1 &obj) = delete;\n\
    ~%1() = default;\n\
    %1& operator=(const %1 &obj) = delete;\n\n\
    void printCurrentAddress();\n\n\
private:\n\n\
    %1() = default;\n\
    int m_number = 0;\n\n\
};\n").arg(className) + includeGuardText2;
    *text2 = QString("\
#include <iostream>\n\
#include \"%1.h\"\n\n\
%2& %2::getInstance() {\n\
    static %2 instance;\n\
    return instance;\n\
}\n\n\
void %2::printCurrentAddress() {\n\
    std::cout << this << '\\n';\n\
}\n").arg(className.toLower()).arg(className);
}

QString makeProductArgText(Argument<QString> *argument) {
    QString constText = "";
    if (argument->constFlag())
        constText = "const ";
    return constText + argument->getType() + " " + argument->getName();
}

QString makeProductMethodsText(QVector<ClassMethod<QString> *> &productMethods, const QString &s1, const QString &s2, const QString &s3) {
    QString productMethodsText = "";
    const int productMethodsNum = productMethods.count();
    for (int methodIndex = 0; methodIndex < productMethodsNum; ++methodIndex) {
        productMethodsText += s1;
        if (productMethods[methodIndex]->constFlag()) {
            productMethodsText += "const ";
        }
        productMethodsText += productMethods[methodIndex]->getType();
        productMethodsText += " " + s2 + productMethods[methodIndex]->getName() + "(";
        const int argsNum = productMethods[methodIndex]->getArgsNum();
        for (int argIndex = 0; argIndex < argsNum-1; ++argIndex) {
            productMethodsText += makeProductArgText(productMethods[methodIndex]->getArgument(argIndex)) + ", ";
        }
        if (argsNum != 0) {
            productMethodsText += makeProductArgText(productMethods[methodIndex]->getArgument(argsNum-1));
        }
        productMethodsText += ")" + s3 + "\n";
    }
    return productMethodsText;
}

QString makeProductsText(QVector<QVector<ClassMethod<QString> *>> &productsMethods, QVector<QString> &products, QVector<QString> &factories) {
    QString productsText = "";
    const int productsNum = products.count();
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        const QString productMethodsText = makeProductMethodsText(productsMethods[productIndex], "    virtual ", "", " = 0;");
        const QString productText = QString("class %1 {\n\
public:\n\
    virtual const std::string getName() = 0;\n\
    virtual ~%1() = default;\n\n\
%2\
};\n\n").arg(productName).arg(productMethodsText);
        productsText += productText;

        const QString productFactoryMethodsText = makeProductMethodsText(productsMethods[productIndex], "    ", "", " override { }");
        const int factoriesNum = factories.count();
        for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
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

QString getAbstractFactoryMethodsText(QVector<QString> &products, const int &pointerType) {
    enum POINTER_TYPE { RAW, UNIQUE, SHARED };
    QString abstractFactoryMethodsText = "";
    QString resultType;
    switch(pointerType) {
        case RAW:
            resultType = "%1*";
            break;
        case UNIQUE:
            resultType = "std::unique_ptr<%1>";
            break;
        case SHARED:
            resultType = "std::shared_ptr<%1>";
            break;
        default:
            qWarning() << "Unexpected pointer type index";
            resultType = "?";
            break;
    }
    const int productsNum = products.count();
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        abstractFactoryMethodsText += QString("    virtual %1 factoryMethod%2() = 0;\n").arg(resultType.arg(productName)).arg(productName);
    }
    return abstractFactoryMethodsText;
}

void findResMethodTypeAndResRetObj(QString *resultMethodType, QString *resultReturnObj, const int &pointerType) {
    enum POINTER_TYPE { RAW, UNIQUE, SHARED };
    switch (pointerType) {
        case RAW:
            *resultMethodType = "%1*";
            *resultReturnObj = "new %1%2";
            break;
        case UNIQUE:
            *resultMethodType = "std::unique_ptr<%1>";
            *resultReturnObj = "std::make_unique<%1%2>()";
            break;
        case SHARED:
            *resultMethodType = "std::shared_ptr<%1>";
            *resultReturnObj = "std::make_shared<%1%2>()";
            break;
        default:
            qWarning() << "Unexpected pointer type index";
            *resultMethodType = "?";
            *resultReturnObj = "?";
            break;
    }
}

QString getFactoryMethodsText(QVector<QString> &products, const QString &factoryName, const int &pointerType) {
    QString factoryMethodsText = "";
    QString resultMethodType, resultReturnObj;
    findResMethodTypeAndResRetObj(&resultMethodType, &resultReturnObj, pointerType);
    const int productsNum = products.count();
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        const QString factoryMethodText = QString("\
    %1 factoryMethod%2() {\n\
        return %3;\n\
    }\n\n").arg(resultMethodType.arg(productName)).arg(productName).arg(resultReturnObj.arg(productName).arg(factoryName));
            factoryMethodsText += factoryMethodText;
    }
    return factoryMethodsText;
}

QString makeFactoriesText(const QString &abstractFactoryName, QVector<QString> &products, QVector<QString> &factories, const int &pointerType) {
    const QString abstractFactoryMethodsText = getAbstractFactoryMethodsText(products, pointerType);
    QString factoriesText = QString("class %1 {\n\
public:\n\
%2\
};\n\n").arg(abstractFactoryName).arg(abstractFactoryMethodsText);

    const int factoriesNum = factories.count();
    for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
        const QString factoryName = factories[factoryIndex];
        const QString factoryMethodsText = getFactoryMethodsText(products, factoryName, pointerType);
        const QString factoryClassText = QString("class %1: public %2 {\n\
public:\n\
%3\
};\n\n").arg(factoryName).arg(abstractFactoryName).arg(factoryMethodsText);
        factoriesText += factoryClassText;
    }
    return factoriesText;
}

void initClassText(QVector<ClassText *> *classTexts, int *classTextCounter, const QString &name, const QString &text,
                   const QString &type) {
    assert(*classTextCounter < classTexts->count());
    (*classTexts)[*classTextCounter] = new ClassText(name, text, type);
    *classTextCounter += 1;
}

void CodeGenerator::genAbstractFactory(QString *text, const int &pointerType, const QString &abstractFactoryName, QVector<QString> &factories,
                                       QVector<QString> &products, QVector<QVector<ClassMethod<QString> *>> &productsMethods) const {
    const QString productsText = makeProductsText(productsMethods, products, factories);
    const QString factoriesText = makeFactoriesText(abstractFactoryName, products, factories, pointerType);
    *text = productsText + factoriesText;
}

void CodeGenerator::genAbstractFactoryProductsClassesHandCpp(QVector<ClassText *> *classTexts,
                                                             QVector<QString> &factories, QVector<QString> &products,
                                                             QVector<QVector<ClassMethod<QString> *>> &productsMethods,
                                                             const int &productsNum, const int &factoriesNum,
                                                             int *classTextCounter) const {
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        const QString productClassFileName = productName.toLower();
        const QString productMethodsText = makeProductMethodsText(productsMethods[productIndex], "    virtual ", "",  " = 0;");
        const QString productText = includeGuardText1.arg(productClassFileName) + QString("\
#include <string>\n\n\
class %1 {\n\
public:\n\
    virtual const std::string getName() = 0;\n\
    virtual ~%1() = default;\n\n\
%2\
};\n\n").arg(productName).arg(productMethodsText) + includeGuardText2;
        initClassText(classTexts, classTextCounter, productClassFileName, productText, ".h");

        const QString productFactoryMethodsTextH = makeProductMethodsText(productsMethods[productIndex], "    ", "", " override;");
        for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
            const QString factoryName = factories[factoryIndex];
            const QString productFactoryClassFileName = productClassFileName + factoryName.toLower();
            const QString productFactoryTextH = includeGuardText1.arg(productFactoryClassFileName) + QString("\
#include \"%1.h\"\n\n\
class %2%3: public %2 {\n\
public:\n\
    const std::string getName() override;\n\
%4\
};\n\n").arg(productClassFileName).arg(productName).arg(factoryName).arg(productFactoryMethodsTextH) + includeGuardText2;
            initClassText(classTexts, classTextCounter, productFactoryClassFileName, productFactoryTextH, ".h");

            const QString productFactoryMethodsTextCpp = makeProductMethodsText(productsMethods[productIndex], "",
                                                                                productName+factoryName+"::", " {\n\n}\n");
            const QString productFactoryTextCpp = QString("\
#include \"%1.h\"\n\n\
const std::string %2%3::getName() {\n\
    return \"%2%3\";\n\
}\n\n\
%4\n").arg(productFactoryClassFileName).arg(productName).arg(factoryName).arg(productFactoryMethodsTextCpp);
            initClassText(classTexts, classTextCounter, productFactoryClassFileName, productFactoryTextCpp, ".cpp");
        }
    }
}

QString getPointerInclude(const int &pointerType) {
    enum POINTER_TYPE { RAW, UNIQUE, SHARED };
    switch (pointerType) {
        case RAW:
            return "";
        case UNIQUE:
        case SHARED:
            return "#include <memory>\n";
        default:
            qWarning() << "Unexpected pointer type index";
            return "?";
    }
}

QString getFactoryMethodsTextForH(QVector<QString> &products, const int &pointerType) {
    QString factoryMethodsText = "";
    QString resultMethodType, resultReturnObject;
    //actually i don't need resultReturnObject
    findResMethodTypeAndResRetObj(&resultMethodType, &resultReturnObject, pointerType);
    const int productsNum = products.count();
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        const QString factoryMethodText = QString("    %1 factoryMethod%2();\n").arg(resultMethodType.arg(productName)).arg(productName);
            factoryMethodsText += factoryMethodText;
    }
    return factoryMethodsText;
}

QString getFactoryMethodsTextForCpp(QVector<QString> &products, const QString &factoryName, const int &pointerType) {
    QString factoryMethodsText = "";
    QString resultMethodType, resultReturnObj;
    findResMethodTypeAndResRetObj(&resultMethodType, &resultReturnObj, pointerType);
    const int productsNum = products.count();
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productName = products[productIndex];
        const QString factoryMethodText = QString("\
%1 %2::factoryMethod%3() {\n\
    return %4;\n\
}\n\n").arg(resultMethodType.arg(productName)).arg(factoryName).arg(productName).arg(resultReturnObj.arg(productName).arg(factoryName));
        factoryMethodsText += factoryMethodText;
    }
    return factoryMethodsText;
}

void CodeGenerator::genAbstractFactoryFactoriesClassesHandCpp(QVector<ClassText *> *classTexts, const QString &abstractFactoryName,
                                                              QVector<QString> &factories, QVector<QString> &products,
                                                              const int &pointerType, const int &factoriesNum, int *classTextCounter) const {
    const QString pointerInclude = getPointerInclude(pointerType);
    const QString abstractFactoryFileName = abstractFactoryName.toLower();
    const QString abstractFactoryMethodsText = getAbstractFactoryMethodsText(products, pointerType);
    QString factoryText = includeGuardText1.arg(abstractFactoryFileName) + QString("\
%1\n\
class %2 {\n\
public:\n\
%3\
};\n\n").arg(pointerInclude).arg(abstractFactoryName).arg(abstractFactoryMethodsText) + includeGuardText2;
    initClassText(classTexts, classTextCounter, abstractFactoryFileName, factoryText, ".h");

    for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
        const QString factoryName = factories[factoryIndex];
        const QString factoryFileName = factoryName.toLower();
        const QString factoryMethodsTextH = getFactoryMethodsTextForH(products, pointerType);
        const QString factoryClassTextH = includeGuardText1.arg(factoryFileName) + QString("\
#include \"%1.h\"\n\n\
class %2: public %3 {\n\
public:\n\
%4\
};\n\n").arg(abstractFactoryFileName).arg(factoryName).arg(abstractFactoryName).arg(factoryMethodsTextH) + includeGuardText2;
        initClassText(classTexts, classTextCounter, factoryFileName, factoryClassTextH, ".h");

        const QString factoryMethodsTextCpp = getFactoryMethodsTextForCpp(products, factoryName, pointerType);
        const QString factoryClassTextCpp = QString("\
#include \"%1.h\"\n\n\
%2").arg(factoryFileName).arg(factoryMethodsTextCpp);
        initClassText(classTexts, classTextCounter, factoryFileName, factoryClassTextCpp, ".cpp");
    }
}

void CodeGenerator::genAbstractFactory(QVector<ClassText *> *classTexts, const int &pointerType, const QString &abstractFactoryName,
                                       QVector<QString> &factories, QVector<QString> &products,
                                       QVector<QVector<ClassMethod<QString> *>> &productsMethods) const {
    const int productsNum = products.count();
    const int factoriesNum = factories.count();
    const int classTextNum = productsNum + productsNum*factoriesNum*2 + 1 + factoriesNum*2;
    classTexts->resize(classTextNum);
    int classTextCounter = 0;
    genAbstractFactoryProductsClassesHandCpp(classTexts, factories, products, productsMethods,
                                             productsNum, factoriesNum, &classTextCounter);
    genAbstractFactoryFactoriesClassesHandCpp(classTexts, abstractFactoryName, factories, products, pointerType,
                                              factoriesNum, &classTextCounter);
}
