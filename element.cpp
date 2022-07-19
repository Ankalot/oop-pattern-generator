#include "element.h"
#include "baseelement.h"

Element::Element(const QString &text, const QHash<QString, QVector<int>> &includes): BaseElement() {
    initText = text;
    this->text = text;
    this->includes = includes;
    this->index = allElements.count();
    allElements.append(this);
}

Element::~Element() {
    const int elementsNum = allElements.count();
    for (int i = index + 1; i < elementsNum; ++i)
        --allElements[i]->index;
    allElements.remove(index);
}

QMap<int, Element *> Element::getSortedElementsFromSource(const QString &sourceFileName) {
    QMap<int, Element *> elements;
    const int elementsNum = allElements.count();
    for (int elementIndex = 0; elementIndex < elementsNum; ++elementIndex) {
        QHash<QString, QVector<int>> elementIncludes = allElements[elementIndex]->includes;
        if (elementIncludes.contains(sourceFileName)) {
            QVector<int> elementPosInText = elementIncludes.value(sourceFileName);
            const int elementPosInTextNum = elementPosInText.count();
            for (int elementPosInTextIndex = 0; elementPosInTextIndex < elementPosInTextNum; ++elementPosInTextIndex) {
                elements.insert(elementPosInText[elementPosInTextIndex], allElements[elementIndex]);
            }
        }
    }
    return elements;
}

void Element::setText(const QString &newText) {
    const int shift = newText.length() - text.length();
    QHashIterator<QString, QVector<int>> i(includes);
    while (i.hasNext()) {
        i.next();
        const QString fileName = i.key();
        QMap<int, Element *> elementsFromFile = getSortedElementsFromSource(fileName);
        QMapIterator<int, Element *> j(elementsFromFile);
        int currentShift = 0;
        const int elementsFromFileNum = elementsFromFile.count();
        QVector<int> posIndexes(elementsFromFileNum);
        QVector<int> newPoses(elementsFromFileNum);
        int counter = 0;
        while (j.hasNext()) {
            j.next();
            QVector<int> elementPosInFile = j.value()->includes.value(fileName);
            posIndexes[counter] = j.value()->includes.value(fileName).indexOf(j.key());
            newPoses[counter] = j.key() + currentShift;
            if (j.value() == this)
                currentShift += shift;
            ++counter;
        }
        j = QMapIterator<int, Element *>(elementsFromFile);
        counter = 0;
        while (j.hasNext()) {
            j.next();
            j.value()->includes[fileName][posIndexes[counter]] = newPoses[counter];
            ++counter;
        }
    }
    text = newText;
}

const QString &Element::getText() {
    return text;
}

const QString &Element::getInitText() {
    return initText;
}

