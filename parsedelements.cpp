#include "parsedelements.h"
#include "baseelement.h"
#include "element.h"
#include "classtext.h"
#include "vectorelement.h"
#include "productmethods.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>

bool ParsedElements::isOk() {
    return ok;
}

bool findSingletonClassText(const QHash<QString, QVector<ClassText *>> &parseData, ClassText **singletonH, ClassText **singletonCpp) {
    if (!parseData.contains("singleton"))
        return false;
    QVector<ClassText *> ClassTextHandCpp = parseData.value("singleton");
    if (ClassTextHandCpp.count() != 2)
        return false;
    if (ClassTextHandCpp[0]->getFileType() == ".cpp") {
        *singletonCpp = ClassTextHandCpp[0];
        *singletonH = ClassTextHandCpp[1];
    } else {
        *singletonCpp = ClassTextHandCpp[1];
        *singletonH = ClassTextHandCpp[0];
    }
    return true;
}

QVector<int> findWordPosInText(const QString &word, const QString &text, const QString &type) {
    const QRegExp wordRe("\\W" + word + "\\W");
    QVector<int> wordPos;
    int pos = 0;
    pos = wordRe.indexIn(text, pos);
    if (pos != -1) {
        // Don't take: #include "class_name.h". So I check if there is a " before class_name
        if (type == ".h" or text[pos] != "\"") {
            wordPos.append(pos+1);
        }
        pos += wordRe.matchedLength();
        while ((pos = wordRe.indexIn(text, pos)) != -1) {
            wordPos.append(pos+1);
            pos += wordRe.matchedLength();
        }
    }
    return wordPos;
}

void ParsedElements::parseSingleton() {
    // .h file:
    // 1) find 3 words: "class", "class_name", "{"
    // 2) find all "class_name" in text
    // .cpp file:
    // 1) find all "class_name" in text, except #include "class_name.h"

    ClassText *singletonH = nullptr, *singletonCpp = nullptr;
    if (!findSingletonClassText(parseData, &singletonH, &singletonCpp)) {
        qWarning() << "Something is wrong with imported files";
        return;
    }
    const QString singletonTextH = singletonH->getText();
    const QString singletonTextCpp = singletonCpp->getText();

    QRegExp word("\\w+");
    QRegExp classDef("class +\\w+ {0,}[{]");
    int classDefPos = classDef.indexIn(singletonTextH);
    if (classDefPos == -1) {
        qWarning() << "Can't find class in singleton .h file";
        return;
    }
    word.indexIn(singletonTextH, classDefPos+5);
    QString className = word.cap(0);

    QVector<int> classNamePosH = findWordPosInText(className, singletonTextH, ".h");
    QVector<int> classNamePosCpp = findWordPosInText(className, singletonTextCpp, ".cpp");

    QHash<QString, QVector<int>> includes;
    includes.insert(singletonH->getFileName(), classNamePosH);
    includes.insert(singletonCpp->getFileName(), classNamePosCpp);
    Element *classNameElement = new Element(className, includes);
    elements.insert("className", (classNameElement));

    ok = true;
    //debug
    /*
    const int classNamePosHNum = classNamePosH.count();
    for (int i = 0; i < classNamePosHNum; ++i) {
        qDebug() << classNamePosH[i] << word.indexIn(singletonTextH, classNamePosH[i]) << word.cap(0);
    }
    const int classNamePosCppNum = classNamePosCpp.count();
    for (int i = 0; i < classNamePosCppNum; ++i) {
        qDebug() << classNamePosCpp[i] << word.indexIn(singletonTextCpp, classNamePosCpp[i]) << word.cap(0);
    }
    */
}

bool findAbstractFactoryClassText(const QHash<QString, QVector<ClassText *>> &parseData, ClassText **abstractFactoryH) {
    if (!parseData.contains("abstractFactory"))
        return false;
    QVector<ClassText *> abstractFactoryHvec = parseData.value("abstractFactory");
    if (abstractFactoryHvec.count() != 1 and abstractFactoryHvec[0]->getFileType() != ".h")
        return false;
    *abstractFactoryH = abstractFactoryHvec[0];
    return true;
}

bool findSpecialCpp(ClassText **specialCpp, const QVector<ClassText *> &allHandCpp, const QString &specialHFileName) {
    QRegExp HFileNameOnlyRe("\\w+.h");
    if (HFileNameOnlyRe.indexIn(specialHFileName) == -1)
        return false;
    const QString HFileNameOnly = HFileNameOnlyRe.cap(0);
    QRegExp specialInclude("#include +\"" + HFileNameOnly + "\"");
    const int allHandCppNum = allHandCpp.count();
    for (int allHandCppIndex = 0; allHandCppIndex < allHandCppNum; ++allHandCppIndex) {
        if (allHandCpp[allHandCppIndex]->getFileType() == ".cpp")
            if (specialInclude.indexIn(allHandCpp[allHandCppIndex]->getText()) != -1) {
                *specialCpp = allHandCpp[allHandCppIndex];
                return true;
            }
    }
    return false;
}

bool findSpecialH(QVector<ClassText *> *specialH, const QVector<ClassText *> &allHandCpp, const QString &specialHFileName) {
    (*specialH).clear();
    bool ok = false;
    QRegExp HFileNameOnlyRe("\\w+.h");
    if (HFileNameOnlyRe.indexIn(specialHFileName) == -1)
        return false;
    const QString HFileNameOnly = HFileNameOnlyRe.cap(0);
    QRegExp specialInclude("#include +\"" + HFileNameOnly + "\"");
    const int allHandCppNum = allHandCpp.count();
    for (int allHandCppIndex = 0; allHandCppIndex < allHandCppNum; ++allHandCppIndex) {
        if (allHandCpp[allHandCppIndex]->getFileType() == ".h")
            if (specialInclude.indexIn(allHandCpp[allHandCppIndex]->getText()) != -1) {
                (*specialH).append(allHandCpp[allHandCppIndex]);
                ok = true;
            }
    }
    return ok;
}

bool findFactoriesClassText(const QHash<QString, QVector<ClassText *>> &parseData, QVector<ClassText *> *factoriesH,
                            QVector<ClassText *> *factoriesCpp) {
    if (!parseData.contains("factories"))
        return false;
    QVector<ClassText *> factoriesHandCpp = parseData.value("factories");
    const int factoriesHandCppNum = factoriesHandCpp.count();
    for (int factoryHandCppIndex = 0; factoryHandCppIndex < factoriesHandCppNum; ++factoryHandCppIndex) {
        if (factoriesHandCpp[factoryHandCppIndex]->getFileType() == ".h")
            (*factoriesH).append(factoriesHandCpp[factoryHandCppIndex]);
    }
    const int factoriesHNum = (*factoriesH).count();
    ClassText *factoryCpp = nullptr;
    for (int factoryHIndex = 0; factoryHIndex < factoriesHNum; ++factoryHIndex) {
        if (!findSpecialCpp(&factoryCpp, factoriesHandCpp, (*factoriesH)[factoryHIndex]->getFileName()))
            return false;
        (*factoriesCpp).append(factoryCpp);
    }
    if (factoriesHNum != (*factoriesCpp).count())
        return false;
    return true;
}

bool findProductsClassText(const QHash<QString, QVector<ClassText *>> &parseData, QVector<ClassText *> *productsH,
                           QVector<QVector<ClassText *>> *childProductsH, QVector<QVector<ClassText *>> *childProductsCpp) {
    // if there is a .h file which does not have .cpp implementation then it is (abstract) product
    if (!parseData.contains("products"))
        return false;
    QVector<ClassText *> allProductsHandCpp = parseData.value("products");
    const int allProductsNum = allProductsHandCpp.count();
    QVector<ClassText *> groupChildProductsH;
    ClassText *childProductCpp;
    for (int i = 0; i < allProductsNum; ++i) {
        if (allProductsHandCpp[i]->getFileType() == ".h") {
            if (!findSpecialCpp(&childProductCpp, allProductsHandCpp, allProductsHandCpp[i]->getFileName())) {
                if (findSpecialH(&groupChildProductsH, allProductsHandCpp, allProductsHandCpp[i]->getFileName())) {
                    productsH->append(allProductsHandCpp[i]);
                    childProductsH->append(groupChildProductsH);
                    QVector<ClassText *> groupChildProductsCpp;
                    const int groupChildProductsHNum = groupChildProductsH.count();
                    for (int j = 0; j < groupChildProductsHNum; ++j) {
                        if (findSpecialCpp(&childProductCpp, allProductsHandCpp, groupChildProductsH[j]->getFileName()))
                            groupChildProductsCpp.append(childProductCpp);
                        else
                            return false;
                    }
                    childProductsCpp->append(groupChildProductsCpp);
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

Element *makeElementFromNameAndPos(const QString &name, const int pos, ClassText *text) {
    QHash<QString, QVector<int>> includes;
    QVector<int> posInText(1);
    posInText[0] = pos;
    includes.insert(text->getFileName(), posInText);
    return new Element(name, includes);
}

bool findClassBody(QString *classBody, int *classBodyPos, const QString &text) {
    const QRegularExpression classDef("class +\\w+ {0,}[{](\\X+)[}][;]");
    QRegularExpressionMatch classDefMatch = classDef.match(text);
    if (!classDefMatch.hasMatch())
        return false;
    *classBody = classDefMatch.captured(1);
    *classBodyPos = classDefMatch.capturedStart(1);
    return true;
}

void findProductMethods(ProductMethods **prodMethodsElement, ClassText *productH) {
    const QString productTextH = productH->getText();
    QString classBody;
    int classBodyPos;
    if (!findClassBody(&classBody, &classBodyPos, productTextH)) {
        qWarning() << "Can't find class in" << productH->getFileName() << "file";
        return;
    }

    // при изменении константности аргумента в product1.h происходит дичьььь, но в наследниках .h и .cpp меняется нормально(почти)
    // также в наследниках const первое если его не было, но появилось, то оно вылезет за скобку "("

    const QRegularExpression argDef(" *(?<const>(const|)) *(?<type>(\\w|[:])+) +(?<name>\\w+)");
    const QRegularExpression methodDef("virtual *(?<const>(const|)) *(?<type>(\\w|[:])+) +(?<name>\\w+) *[(](?<args>.*)[)]");
    QRegularExpressionMatchIterator i = methodDef.globalMatch(classBody);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.captured("name") == "getName")
            continue;
        int isConstPos = classBodyPos + match.capturedStart("const");
        if (match.captured("const") == "")
            --isConstPos; //actually "" does not have a symbol position, so this is a crutch
        Element *isConstElement = makeElementFromNameAndPos(match.captured("const"), isConstPos, productH);
        Element *typeElement = makeElementFromNameAndPos(match.captured("type"), classBodyPos +
                                                         match.capturedStart("type"), productH);
        Element *nameElement = makeElementFromNameAndPos(match.captured("name"), classBodyPos +
                                                         match.capturedStart("name"), productH);
        ClassMethod<Element *> *method = new ClassMethod<Element *>(isConstElement, typeElement, nameElement);

        const QString argsBody = match.captured("args");
        const int argsBodyPos = classBodyPos + match.capturedStart("args");
        QRegularExpressionMatchIterator j = argDef.globalMatch(argsBody);
        while (j.hasNext()) {
            QRegularExpressionMatch match = j.next();
            int isConstPos = argsBodyPos + match.capturedStart("const");
            if (match.captured("const") == "")
                --isConstPos; //actually "" does not have a symbol position, so this is a crutch
            Element *isConstElement = makeElementFromNameAndPos(match.captured("const"), isConstPos, productH);
            Element *typeElement = makeElementFromNameAndPos(match.captured("type"), argsBodyPos + match.capturedStart("type"), productH);
            Element *nameElement = makeElementFromNameAndPos(match.captured("name"), argsBodyPos + match.capturedStart("name"), productH);
            Argument<Element *> *argument = new Argument<Element *>(isConstElement, typeElement, nameElement);
            method->addArgument(argument);
        }

        (*prodMethodsElement)->addMethod(method);
    }
}

bool isThisMethod(ClassMethod<Element *> *method, const QString &isConst, const QString &type, const QString &name,
                  const QVector<QString> &argsConst, const QVector<QString> &argsType, const QVector<QString> &argsName) {
    if (method->constFlag()->getText() != isConst)
        return false;
    if (method->getType()->getText() != type)
        return false;
    if (method->getName()->getText() != name)
        return false;
    const int argsNum = method->getArgsNum();
    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
        Argument<Element *> *arg = method->getArgument(argIndex);
        if (arg->constFlag()->getText() != argsConst[argIndex])
            return false;
        if (arg->getType()->getText() != argsType[argIndex])
            return false;
        if (arg->getName()->getText() != argsName[argIndex])
            return false;
    }
    return true;
}

bool addInfoAboutMethod(ProductMethods *prodMethodsElement, const QString &isConst, const QString &type, const QString &name,
                        const QVector<QString> &argsConst, const QVector<QString> &argsType, const QVector<QString> &argsName,
                        const int &isConstPos, const int &typePos, const int &namePos,
                        const QVector<int> &argsConstPos, const QVector<int> &argsTypePos, const QVector<int> &argsNamePos,
                        const QString &fileName) {
    bool ok = false;
    const int methodsNum = prodMethodsElement->getCount();
    QVector<int> pos = QVector<int>(1);
    for (int methodIndex = 0; methodIndex < methodsNum; ++methodIndex) {
        ClassMethod<Element *> *method = (*prodMethodsElement)[methodIndex];
        if (isThisMethod(method, isConst, type, name, argsConst, argsType, argsName)) {
            pos[0] = isConstPos;
            method->constFlag()->addInclude(fileName, pos);
            pos[0] = typePos;
            method->getType()->addInclude(fileName, pos);
            pos[0] = namePos;
            method->getName()->addInclude(fileName, pos);
            const int argsNum = method->getArgsNum();
            for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
                Argument<Element *> *arg = method->getArgument(argIndex);
                pos[0] = argsConstPos[argIndex];
                arg->constFlag()->addInclude(fileName, pos);
                pos[0] = argsTypePos[argIndex];
                arg->getType()->addInclude(fileName, pos);
                pos[0] = argsNamePos[argIndex];
                arg->getName()->addInclude(fileName, pos);
            }
            ok = true;
        }
    }

    return ok;
}

bool findChildClassBody(QString *classBody, int *classBodyPos, const QString &text) {
    const QRegularExpression classDef("class +\\w+ *: (public|private|protected) +\\w+ {0,}[{](\\X+)[}][;]");
    QRegularExpressionMatch classDefMatch = classDef.match(text);
    if (!classDefMatch.hasMatch())
        return false;
    *classBody = classDefMatch.captured(2);
    *classBodyPos = classDefMatch.capturedStart(2);
    return true;
}

void addInfoAboutProductMethods(ProductMethods **prodMethodsElement, ClassText *productFile) {
    const QString productText = productFile->getText();
    QString body;
    int bodyPos = 0;
    if (productFile->getFileType() == ".cpp")
        body = productText;
    else if (!findChildClassBody(&body, &bodyPos, productText)) {
        qWarning() << "Can't find child class in" << productFile->getFileName() << "file";
        return;
    }

    const QRegularExpression argDef(" *(?<const>(const|)) *(?<type>(\\w|[:])+) +(?<name>\\w+)");
    QRegularExpression methodDef;
    if (productFile->getFileType() == ".cpp")
        methodDef.setPattern("(?<const>(const|)) *(?<type>(\\w|[:])+) +((\\w+)[:][:])(?<name>\\w+) *[(](?<args>.*)[)]");
    else
        methodDef.setPattern("(?<const>(const|)) *(?<type>(\\w|[:])+) +(?<name>\\w+) *[(](?<args>.*)[)]");
    QRegularExpressionMatchIterator i = methodDef.globalMatch(body);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.captured("name") == "getName")
            continue;
        const QString isConst = match.captured("const");
        int isConstPos = bodyPos + match.capturedStart("const");
        if (match.captured("const") == "")
            --isConstPos; //actually "" does not have a symbol position, so this is a crutch
        const QString type = match.captured("type");
        const int typePos = bodyPos + match.capturedStart("type");
        const QString name = match.captured("name");
        const int namePos = bodyPos + match.capturedStart("name");
        const QString argsBody = match.captured("args");
        const int argsBodyPos = bodyPos + match.capturedStart("args");

        QVector<QString> argsConst, argsType, argsName;
        QVector<int> argsConstPos, argsTypePos, argsNamePos;
        QRegularExpressionMatchIterator j = argDef.globalMatch(argsBody);
        while (j.hasNext()) {
            QRegularExpressionMatch match = j.next();
            argsConst.append(match.captured("const"));
            int isConstPos = argsBodyPos + match.capturedStart("const");
            if (match.captured("const") == "")
                --isConstPos; //actually "" does not have a symbol position, so this is a crutch
            argsConstPos.append(isConstPos);
            argsType.append(match.captured("type"));
            argsTypePos.append(argsBodyPos + match.capturedStart("type"));
            argsName.append(match.captured("name"));
            argsNamePos.append(argsBodyPos + match.capturedStart("name"));
        }

        if (!addInfoAboutMethod(*prodMethodsElement, isConst, type, name, argsConst, argsType, argsName,
                                isConstPos, typePos, namePos, argsConstPos, argsTypePos, argsNamePos, productFile->getFileName())) {
            qCritical() << "Can't find method" << isConst << type << name << "in file" << productFile->getFileName();
        }
    }
}

void ParsedElements::parseAbstractFactory() {
    // abstractFactoryName

    ClassText *abstractFactoryH = nullptr;
    if (!findAbstractFactoryClassText(parseData, &abstractFactoryH)) {
        qWarning() << "Something is wrong with imported abstract factory .h file";
        return;
    }
    const QString abstractFactoryTextH = abstractFactoryH->getText();

    const QRegExp word("\\w+");
    const QRegExp classDef("class +\\w+ {0,}[{]");
    int classDefPos = classDef.indexIn(abstractFactoryTextH);
    if (classDefPos == -1) {
        qWarning() << "Can't find class in abstract factory .h file";
        return;
    }
    word.indexIn(abstractFactoryTextH, classDefPos+5);
    const QString abstractFactoryName = word.cap(0);

    QVector<int> abstractFactoryNamePosH = findWordPosInText(abstractFactoryName, abstractFactoryTextH, ".h");

    QVector<ClassText *> factoriesH, factoriesCpp;
    if (!findFactoriesClassText(parseData, &factoriesH, &factoriesCpp)) {
        qWarning() << "Something is wrong with imported factories files";
        return;
    }

    const int factoriesNum = factoriesH.count();
    QHash<QString, QVector<int>> abstractFactoryNameIncludes;
    abstractFactoryNameIncludes.insert(abstractFactoryH->getFileName(), abstractFactoryNamePosH);
    for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
        QVector<int> abstractFactoryNamePosH = findWordPosInText(abstractFactoryName, factoriesH[factoryIndex]->getText(), ".h");
        QVector<int> abstractFactoryNamePosCpp = findWordPosInText(abstractFactoryName, factoriesCpp[factoryIndex]->getText(), ".cpp");
        abstractFactoryNameIncludes.insert(factoriesH[factoryIndex]->getFileName(), abstractFactoryNamePosH);
        abstractFactoryNameIncludes.insert(factoriesCpp[factoryIndex]->getFileName(), abstractFactoryNamePosCpp);
    }
    Element *abstractFactoryNameElement = new Element(abstractFactoryName, abstractFactoryNameIncludes);
    elements.insert("abstractFactoryName", abstractFactoryNameElement);

    // factoriesNamesElements

    const QRegExp childClassDef("class +\\w+ {0,} {0,}[:] +\\w+ +" + abstractFactoryName + " {0,}[{]");
    VectorElement *factoriesNamesElements = new VectorElement(factoriesNum);
    for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
        const QString factoryTextH = factoriesH[factoryIndex]->getText();
        const QString factoryTextCpp = factoriesCpp[factoryIndex]->getText();
        int classDefPos = childClassDef.indexIn(factoryTextH);
        if (classDefPos == -1) {
            qWarning() << "Can't find class in factory " + factoriesH[factoryIndex]->getFileName() + " file";
            return;
        }
        word.indexIn(factoryTextH, classDefPos+5);
        const QString factoryName = word.cap(0);

        QVector<int> factoryNamePosH = findWordPosInText(factoryName, factoryTextH, ".h");
        QVector<int> factoryNamePosCpp = findWordPosInText(factoryName, factoryTextCpp, ".cpp");

        QHash<QString, QVector<int>> factoryNameIncludes;
        factoryNameIncludes.insert(factoriesH[factoryIndex]->getFileName(), factoryNamePosH);
        factoryNameIncludes.insert(factoriesCpp[factoryIndex]->getFileName(), factoryNamePosCpp);
        Element *factoryNameElement = new Element(factoryName, factoryNameIncludes);
        factoriesNamesElements->setElement(factoryIndex, factoryNameElement);
    }
    elements.insert("factoriesNames", factoriesNamesElements);

    // productsNamesElements

    QVector<ClassText *> productsH;
    QVector<QVector<ClassText *>> childProductsH, childProductsCpp;
    if (!findProductsClassText(parseData, &productsH, &childProductsH, &childProductsCpp)) {
        qWarning() << "Something is wrong with imported products files";
        return;
    }

    const int productsNum = productsH.count();
    VectorElement *productsNamesElements = new VectorElement(productsNum);
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        const QString productTextH = productsH[productIndex]->getText();
        int classDefPos = classDef.indexIn(productTextH);
        if (classDefPos == -1) {
            qWarning() << "Can't find class in product " + productsH[productIndex]->getFileName() + " file";
            return;
        }
        word.indexIn(productTextH, classDefPos+5);
        const QString productName = word.cap(0);

        QVector<int> productNamePosH = findWordPosInText(productName, productTextH, ".h");
        QHash<QString, QVector<int>> productNameIncludes;
        productNameIncludes.insert(productsH[productIndex]->getFileName(), productNamePosH);

        const int groupProductNum = childProductsH[productIndex].count();
        QVector<int> childProductNamePosH, childProductNamePosCpp;
        for (int groupProductIndex = 0; groupProductIndex < groupProductNum; ++groupProductIndex) {
            childProductNamePosH = findWordPosInText(productName, childProductsH[productIndex][groupProductIndex]->
                                                     getText(), ".h");
            childProductNamePosCpp = findWordPosInText(productName, childProductsCpp[productIndex][groupProductIndex]->
                                                       getText(), ".cpp");
            productNameIncludes.insert(childProductsH[productIndex][groupProductIndex]->getFileName(),
                                       childProductNamePosH);
            productNameIncludes.insert(childProductsCpp[productIndex][groupProductIndex]->getFileName(),
                                       childProductNamePosCpp);
        }

        productNamePosH = findWordPosInText(productName, abstractFactoryTextH, ".h");
        QVector<int> productNamePosCpp;
        productNameIncludes.insert(abstractFactoryH->getFileName(), productNamePosH);
        for (int factoryIndex = 0; factoryIndex < factoriesNum; ++factoryIndex) {
            productNamePosH = findWordPosInText(productName, factoriesH[factoryIndex]->getText(), ".h");
            productNamePosCpp = findWordPosInText(productName, factoriesCpp[factoryIndex]->getText(), ".cpp");
            productNameIncludes.insert(factoriesH[factoryIndex]->getFileName(), productNamePosH);
            productNameIncludes.insert(factoriesCpp[factoryIndex]->getFileName(), productNamePosCpp);
        }

        Element *productNameElement = new Element(productName, productNameIncludes);
        productsNamesElements->setElement(productIndex, productNameElement);
    }
    elements.insert("productsNames", productsNamesElements);

    // productsMethods

    VectorElement *productsMethodsElement = new VectorElement(productsNum);
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ProductMethods *prodMethodsElement = new ProductMethods;
        findProductMethods(&prodMethodsElement, productsH[productIndex]);

        const int childProductsNum = childProductsH[productIndex].count();
        for (int childProductIndex = 0; childProductIndex < childProductsNum; ++childProductIndex) {
            addInfoAboutProductMethods(&prodMethodsElement, childProductsH[productIndex][childProductIndex]);
            addInfoAboutProductMethods(&prodMethodsElement, childProductsCpp[productIndex][childProductIndex]);
        }

        productsMethodsElement->setElement(productIndex, prodMethodsElement);
    }
    elements.insert("productsMethods", productsMethodsElement);

    ok = true;
}

ParsedElements::ParsedElements(int patternType, const QHash<QString, QVector<ClassText *>> &parseData) {
    this->parseData = parseData;
    this->patternType = patternType;
    switch (patternType) {
        case SINLETON:
            parseSingleton();
            break;
        case ABSTRACT_FACTORY:
            parseAbstractFactory();
            break;
        default:
            qCritical() << "Unexpected pattern type";
            break;
    }
}

ParsedElements::~ParsedElements() {
    foreach (BaseElement *element, elements) {
        delete element;
    }
}

QString replaceWordInText(const QString &text, const QString &word1, const QString &word2, int pos) {
    QString text1 = text.left(pos);
    QString text2 = text.mid(pos + word1.length());
    return text1 + word2 + text2;
}

bool findElement(QHash<QString, BaseElement *> &elements, const QString &key, Element **element) {
    if (!elements.contains(key))
        return false;
    *element = static_cast<Element *>(elements.value(key));
    if (!element)
        return false;
    return true;
}

bool ParsedElements::writeElementsToFile(ClassText *classText) {
    const QString fileName = classText->getFileName();
    QString fileText = classText->getText();
    QMap<int, Element *> elementsTextInFile = Element::getSortedElementsFromSource(fileName);
    QMapIterator<int, Element *> j(elementsTextInFile);
    while (j.hasNext()) {
        j.next();
        if (j.value()->getText() == j.value()->getInitText())
            continue;
        fileText = replaceWordInText(fileText, j.value()->getInitText(), j.value()->getText(), j.key());
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Can't open file " + fileName;
        return false;
    }
    QTextStream stream(&file);
    stream << fileText;
    file.close();
    return true;
}

bool ParsedElements::rewriteSingletonInFiles() {
    if (!parseData.contains("singleton")) {
        qWarning() << "singleton key not found in parseData";
        return false;
    }
    QVector<ClassText *> ClassTextHandCpp = parseData.value("singleton");
    for (int i = 0; i < 2; ++i) {
        if (!writeElementsToFile(ClassTextHandCpp[i]))
            return false;
    }
    return true;
}

bool findVectorElement(QHash<QString, BaseElement *> &elements, const QString &key, VectorElement **element) {
    if (!elements.contains(key))
        return false;
    *element = static_cast<VectorElement *>(elements.value(key));
    if (!element)
        return false;
    return true;
}

bool ParsedElements::rewriteAbstractFactoryInFiles() {
    ClassText *abstractFactoryH = nullptr;
    if (!findAbstractFactoryClassText(parseData, &abstractFactoryH)) {
        qWarning() << "Something is wrong with imported abstract factory .h file";
        return false;
    }
    QVector<ClassText *> factoriesHandCpp;
    if (!parseData.contains("factories")) {
        qWarning() << "Something is wrong with imported factories files";
        return false;
    }
    factoriesHandCpp = parseData.value("factories");
    QVector<ClassText *> allProductsHandCpp;
    if (!parseData.contains("products")) {
        qWarning() << "Something is wrong with imported products files";
        return false;
    }
    allProductsHandCpp = parseData.value("products");

    if (!writeElementsToFile(abstractFactoryH))
        return false;
    const int factoriesHandCppNum = factoriesHandCpp.count();
    for (int factoriesHandCppIndex = 0; factoriesHandCppIndex < factoriesHandCppNum; ++factoriesHandCppIndex)
        if (!writeElementsToFile(factoriesHandCpp[factoriesHandCppIndex]))
            return false;
    const int allProductsHandCppNum = allProductsHandCpp.count();
    for (int allProductsHandCppIndex = 0; allProductsHandCppIndex < allProductsHandCppNum; ++allProductsHandCppIndex)
        if (!writeElementsToFile(allProductsHandCpp[allProductsHandCppIndex]))
            return false;

    return true;
}

bool ParsedElements::rewriteInFiles() {
    switch (patternType) {
        case SINLETON:
            return rewriteSingletonInFiles();
            break;
        case ABSTRACT_FACTORY:
            return rewriteAbstractFactoryInFiles();
            break;
        default:
            qCritical() << "Unexpected pattern type";
            break;
    }
    return false;
}

QHash<QString, BaseElement *> ParsedElements::getElements() {
    return elements;
}
