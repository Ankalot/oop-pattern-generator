#include "parsedelements.h"
#include "baseelement.h"
#include "element.h"
#include "classtext.h"

#include <QDebug>
#include <QFile>

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

void ParsedElements::parseSingleton() {
    // .h file:
    // 1) find 3 words: "class", "class_name", "{"
    // 2) find all "class_name" in text
    // .cpp file:
    // 1) find all "class_name" in text, except #include "class_name.h"

    ClassText *singletonH = nullptr, *singletonCpp = nullptr;
    if (!findSingletonClassText(parseData, &singletonH, &singletonCpp))
        return;
    const QString singletonTextH = singletonH->getText();
    const QString singletonTextCpp = singletonCpp->getText();

    QRegExp word("\\w+");
    QRegExp classDef("class +\\w+ {0,}[{]");
    int classDefPos = classDef.indexIn(singletonTextH);
    if (classDefPos == -1)
        return;
    word.indexIn(singletonTextH, classDefPos+5);
    QString className = word.cap(0);
    QRegExp classNameRe("\\W" + className + "\\W");

    QVector<int> classNamePosH;
    int pos = 0;
    while ((pos = classNameRe.indexIn(singletonTextH, pos)) != -1) {
        classNamePosH.append(pos+1);
        pos += classNameRe.matchedLength();
    }

    QVector<int> classNamePosCpp;
    pos = 0;
    pos = classNameRe.indexIn(singletonTextCpp, pos);
    if (pos != -1) {
        // Don't take: #include "class_name.h". So I check if there is a " before class_name
        if (singletonTextCpp[pos-1] != "\"") {
            classNamePosCpp.append(pos+1);
        }
        pos += classNameRe.matchedLength();
        while ((pos = classNameRe.indexIn(singletonTextCpp, pos)) != -1) {
            classNamePosCpp.append(pos+1);
            pos += classNameRe.matchedLength();
        }
    }

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

void ParsedElements::parseAbstractFactory() {
    //coming soon
    ok = false;
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

bool ParsedElements::rewriteSingletonInFiles() {
    if (!elements.contains("className")) {
        qWarning() << "className key not found in elements";
        return false;
    }
    Element *classNameElement = static_cast<Element *>(elements.value("className"));
    if (!classNameElement) {
        qWarning() << "classNameElement not found";
        return false;
    }
    if (!parseData.contains("singleton")) {
        qWarning() << "singleton key not found in parseData";
        return false;
    }
    QVector<ClassText *> ClassTextHandCpp = parseData.value("singleton");
    for (int i = 0; i < 2; ++i) {
        const QString fileName = ClassTextHandCpp[i]->getFileName();
        QString fileText = ClassTextHandCpp[i]->getText();
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
    }
    return true;
}

bool ParsedElements::rewriteAbstractFactoryInFiles() {
    return false;
    //coming soon
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
