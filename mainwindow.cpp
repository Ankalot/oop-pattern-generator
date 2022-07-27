#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codegenerator.h"
#include "classmethod.h"
#include "classtext.h"
#include "argument.h"
#include "exportwindow.h"
#include "importwindow.h"
#include "parsedelements.h"
#include "element.h"
#include "vectorelement.h"
#include "classmethods.h"

#include <cassert>
#include <QCheckBox>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QCloseEvent>

void MainWindow::writeSettings()
{
    settings->beginGroup("MainWindow");
    settings->setValue("size", size());
    settings->setValue("pos", pos());
    settings->endGroup();
}

void MainWindow::readSettings()
{
    settings->beginGroup("MainWindow");
    resize(settings->value("size", QSize(800, 500)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    settings->endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), codeGenerator(new CodeGenerator(true))
{
    ui->setupUi(this);
    ui->cmbBoxPatternName->addItem("Select pattern");
    ui->cmbBoxPatternName->addItem("Singleton");
    ui->cmbBoxPatternName->addItem("Abstract factory");
    ui->cmbBoxPatternName->addItem("Builder");
    ui->gridLayoutSpecial->setRowStretch(0, 1);

    patternTypesList = new QStringList;
    *patternTypesList << "Select pattern" << "Singleton" << "Abstract factory" << "Builder";

    connect(ui->cmbBoxPatternName, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboBox_indexChanged()));

    settings = new QSettings("OOP-P-G", "Main settings");
    readSettings();
}

QSpinBox *getSpinBoxClassMethodArgsNum(QWidget *spinBoxNumArgsContent) {
    if (!spinBoxNumArgsContent)
        qCritical() << "spinBoxNumArgsContent not found";
    QLayout *layout = spinBoxNumArgsContent->layout();
    if (!layout)
        qCritical() << "spinBoxNumArgsContent layout not found";
    QHBoxLayout *spinBoxNumArgsLayout = qobject_cast<QHBoxLayout *>(layout);
    if (!spinBoxNumArgsLayout)
        qCritical() << "spinBoxNumArgsLayout not found";
    QLayoutItem *spinBoxNumArgsItem = spinBoxNumArgsLayout->itemAt(0);
    if (!spinBoxNumArgsItem)
        qCritical() << "spinBoxNumArgs item not found";
    QSpinBox *spinBoxNumArgs = qobject_cast<QSpinBox *>(spinBoxNumArgsItem->widget());
    if (!spinBoxNumArgs)
        qCritical() << "spinBoxNumArgs not found";
    return spinBoxNumArgs;
}

int getClassMethodArgsNum(QWidget *spinBoxNumArgsContent) {
    QSpinBox *spinBoxNumArgs = getSpinBoxClassMethodArgsNum(spinBoxNumArgsContent);
    return spinBoxNumArgs->value();
}

QCheckBox *getCheckBoxClassMethodConst(QWidget *checkBoxConstContent) {
    if (!checkBoxConstContent)
        qCritical() << "checkBoxConstContent not found";
    QLayout *layout = checkBoxConstContent->layout();
    if (!layout)
        qCritical() << "checkBoxConstContent layout not found";
    QHBoxLayout *checkBoxConstLayout = qobject_cast<QHBoxLayout *>(layout);
    if (!checkBoxConstLayout)
        qCritical() << "checkBoxConstLayout not found";
    QLayoutItem *checkBoxConstItem = checkBoxConstLayout->itemAt(0);
    if (!checkBoxConstItem)
        qCritical() << "checkBoxConst item not found";
    QCheckBox *checkBoxConst = qobject_cast<QCheckBox *>(checkBoxConstItem->widget());
    if (!checkBoxConst)
        qCritical() << "checkBoxConst not found";
    return checkBoxConst;
}

bool checkCheckBoxConst(QWidget *checkBoxConstContent) {
    QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(checkBoxConstContent);
    return checkBoxConst->isChecked();
}

QString MainWindow::getExportFolderPath() {
    QString folderPath = settings->value("Export/folderPath").toString();
    QCharRef lastChar = folderPath[folderPath.length()-1];
    if (lastChar != "/" and lastChar != "\\")
        folderPath += "/";
    return folderPath;
}

bool MainWindow::writeTextToFile(const QString &fileFullName, const QString &text) {
    QFile file(fileFullName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Can't open file " + fileFullName;
        ui->statusBar->showMessage("Can't open file: " + fileFullName, 5000);
        return false;
    }
    QTextStream stream(&file);
    stream << text;
    file.close();
    return true;
}

bool MainWindow::generateSingleton(int exportType) {
    QLineEdit *lineEditSngltn = ui->centralwidget->findChild<QLineEdit *>("lineEditSngltn");
    if (!lineEditSngltn)
        qCritical() << "lineEditSngltn not found";
    const QString className = lineEditSngltn->text();

    switch (exportType) {
        case CLIPBOARD: {
            QString text;
            codeGenerator->genSingleton(&text, className);
            QApplication::clipboard()->setText(text);
            break;
        } case CPP_FILE: {
            QString text;
            codeGenerator->genSingleton(&text, className);
            const QString fileName = settings->value("Export/fileName").toString();
            const QString folderPath = getExportFolderPath();
            return writeTextToFile(folderPath + fileName + ".cpp", text);
            break;
        } case H_AND_CPP_FILES:
            QString text1, text2;
            codeGenerator->genSingleton(&text1, &text2, className);
            const QString fileName = className.toLower();
            const QString folderPath = getExportFolderPath();
            if (!writeTextToFile(folderPath + fileName + ".h", text1))
                return false;
            if (!writeTextToFile(folderPath + fileName + ".cpp", text2))
                return false;
            break;
    }
    return true;
}

QVector<ClassMethod<QString> *> getClassMethodsFromTable(QTableWidget *tableClassMethods) {
    QVector<ClassMethod<QString> *> classMethods;
    if (!tableClassMethods)
        qCritical() << "tableClassMethods not found";
    const int classMethodsNum = tableClassMethods->rowCount();
    classMethods.resize(classMethodsNum);

    for (int classMethodIndex = 0; classMethodIndex < classMethodsNum; ++classMethodIndex) {
        const int argsNum = getClassMethodArgsNum(tableClassMethods->cellWidget(classMethodIndex, 0));
        const bool isConst = checkCheckBoxConst(tableClassMethods->cellWidget(classMethodIndex, 1));
        QTableWidgetItem *typeItem = tableClassMethods->item(classMethodIndex, 2);
        if (!typeItem)
            qCritical() << "type item not found";
        const QString type = typeItem->text();
        QTableWidgetItem *nameItem = tableClassMethods->item(classMethodIndex, 3);
        if (!nameItem)
            qCritical() << "name item not found";
        const QString name = nameItem->text();
        ClassMethod<QString> *classMethod = new ClassMethod<QString>(isConst, type, name, argsNum);

        for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
            QTableWidgetItem *typeItem = tableClassMethods->item(classMethodIndex, 5+argIndex*3);
            if (!typeItem)
                qCritical() << "type item not found";
            QTableWidgetItem *nameItem = tableClassMethods->item(classMethodIndex, 6+argIndex*3);
            if (!nameItem)
                qCritical() << "name item not found";
            Argument<QString> *arg = new Argument<QString>(checkCheckBoxConst(tableClassMethods->cellWidget(classMethodIndex,
                                                                                                              4+argIndex*3)),
                                         typeItem->text(), nameItem->text());
            classMethod->setArgument(arg, argIndex);
        }
        classMethods[classMethodIndex] = classMethod;
    }
    return  classMethods;
}

QVector<QVector<ClassMethod<QString> *>> getClassesMethodsFromScrollArea(QScrollArea *listOfClassesMethods) {
    if (!listOfClassesMethods)
        qCritical() << "listOfClassesMethods not found";
    QWidget *listOfClassesMethodsWidget = listOfClassesMethods->widget();
    if (!listOfClassesMethodsWidget)
        qCritical() << "listOfClassesMethods widget not found";
    QHBoxLayout *layoutClassesMethodsList = qobject_cast<QHBoxLayout *>(listOfClassesMethodsWidget->layout());
    if (!layoutClassesMethodsList)
        qCritical() << "layoutClassesMethodsList not found";
    const int classesNum = layoutClassesMethodsList->count();

    QVector<QVector<ClassMethod<QString> *>> classesMethods(classesNum);

    for (int classItemIndex = 0; classItemIndex < classesNum; ++classItemIndex) {
        QWidget *classMethodsContent = layoutClassesMethodsList->itemAt(classItemIndex)->widget();
        QVBoxLayout *classMethods = qobject_cast<QVBoxLayout *>(classMethodsContent->layout());
        if (!classMethods)
            qCritical() << "classMethods not found";
        QLayoutItem *tableClassMethodsItem = classMethods->itemAt(1); //0 - labels and boxes widget, 1 - table
        if (!tableClassMethodsItem)
            qCritical() << "tableClassMethods item not found";
        QTableWidget *tableClassMethods = qobject_cast<QTableWidget *>(tableClassMethodsItem->widget());
        QVector<ClassMethod<QString> *> classMethodsVec = getClassMethodsFromTable(tableClassMethods);
        classesMethods[classItemIndex] = classMethodsVec;
    }
    return classesMethods;
}

bool MainWindow::generateAbstractFactory(int exportType) {
    bool success = true;
    QRadioButton *btnRawPointer = ui->centralwidget->findChild<QRadioButton *>("btnRawPointer");
    if (!btnRawPointer)
        qCritical() << "btnRawPointer not found";
    QRadioButton *btnUniquePointer = ui->centralwidget->findChild<QRadioButton *>("btnUniquePointer");
    if (!btnUniquePointer)
        qCritical() << "btnUniquePointer not found";

    enum POINTER_TYPE { RAW, UNIQUE, SHARED };

    int pointerType = RAW;
    if (btnUniquePointer->isChecked())
        pointerType = UNIQUE;
    else if (not btnRawPointer->isChecked())
        pointerType = SHARED;

    QLineEdit *lineEditFactoryName = ui->centralwidget->findChild<QLineEdit *>("lineEditFactoryName");
    if (!lineEditFactoryName)
        qCritical() << "lineEditFactoryName not found";
    const QString abstractFactoryName = lineEditFactoryName->text();

    QListWidget *listOfFactories = ui->centralwidget->findChild<QListWidget *>("listOfFactories");
    if (!listOfFactories)
        qCritical() << "listOfFactories not found";
    QVector<QString> factories(listOfFactories->count());
    const int factoriesNum = listOfFactories->count();
    for (int factoryItemIndex = 0; factoryItemIndex < factoriesNum; ++factoryItemIndex) {
        factories[factoryItemIndex] = listOfFactories->item(factoryItemIndex)->text();
    }

    QListWidget *listOfProducts = ui->centralwidget->findChild<QListWidget *>("listOfProducts");
    if (!listOfProducts)
        qCritical() << "listOfProducts not found";
    QVector<QString> products(listOfProducts->count());
    const int productsNum = listOfProducts->count();
    for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
        products[productItemIndex] = listOfProducts->item(productItemIndex)->text();
    }

    QScrollArea *listOfProductsMethods = ui->centralwidget->findChild<QScrollArea *>("listOfProductsMethods");
    QVector<QVector<ClassMethod<QString> *>> productsMethods = getClassesMethodsFromScrollArea(listOfProductsMethods);

    switch (exportType) {
        case CLIPBOARD: {
            QString text;
            codeGenerator->genAbstractFactory(&text, pointerType, abstractFactoryName, factories, products, productsMethods);
            QApplication::clipboard()->setText(text);
            break;
        } case CPP_FILE: {
            QString text;
            codeGenerator->genAbstractFactory(&text, pointerType, abstractFactoryName, factories, products, productsMethods);
            const QString fileName = settings->value("Export/fileName").toString();
            const QString folderPath = getExportFolderPath();
            if (!writeTextToFile(folderPath + fileName + ".cpp", text))
                success = false;
            break;
        } case H_AND_CPP_FILES:
            QVector<ClassText *> classTexts;
            codeGenerator->genAbstractFactory(&classTexts, pointerType, abstractFactoryName, factories, products, productsMethods);
            const QString folderPath = getExportFolderPath();
            const int classTextsNum = classTexts.count();
            for (int classTextIndex = 0; classTextIndex < classTextsNum; ++classTextIndex) {
                if (!writeTextToFile(folderPath + classTexts[classTextIndex]->getFileName() + \
                                     classTexts[classTextIndex]->getFileType(), classTexts[classTextIndex]->getText())) {
                    success = false;
                    continue;
                }
            }
            qDeleteAll(classTexts);
            break;
    }

    for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
        qDeleteAll(productsMethods[productItemIndex]);
    }
    return success;
}

void MainWindow::initParsedPatternAndUi(QVector<QObject **> &uiElements, QStringList &uiElementsName,
                                        QVector<BaseElement **> &elements, QStringList &elementsName) {
    const int uiElementsNum = uiElements.count();
    for (int uiElementIndex = 0; uiElementIndex < uiElementsNum; ++uiElementIndex) {
        QObject **uiElement = uiElements[uiElementIndex];
        const QString uiElementName = uiElementsName[uiElementIndex];
        *uiElement = ui->centralwidget->findChild<QObject *>(uiElementName);
        if (!*uiElement)
            qCritical() << uiElementName << "not found";
    }
    QHash<QString, BaseElement *> elementsHash = parsedPattern->getElements();
    const int elementsNum = elements.count();
    for (int elementIndex = 0; elementIndex < elementsNum; ++elementIndex) {
        BaseElement **element = elements[elementIndex];
        const QString elementName = elementsName[elementIndex];
        *element = elementsHash.value(elementName);
    }
}

void MainWindow::writeUiToParsedSingleton() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;
    QLineEdit *lineEditSngltn = nullptr;
    uiElements.append(reinterpret_cast<QObject **>(&lineEditSngltn));
    uiElementsName.append("lineEditSngltn");
    Element *className = nullptr;
    elements.append(reinterpret_cast<BaseElement **>(&className));
    elementsName.append("className");
    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);
    className->setText(lineEditSngltn->text());
}

void findWidgetsInProductMethodsContentItem(QLayoutItem *productMethodsContentItem, QSpinBox **spinBoxNumMethods,
                                            QTableWidget **tableProductMethods) {
    if (!productMethodsContentItem)
        qCritical() << "productMethodsContent item not found";
    QWidget *productMethodsContent = productMethodsContentItem->widget();
    QVBoxLayout *productMethodsLayout = qobject_cast<QVBoxLayout *>(productMethodsContent->layout());
    if (!productMethodsLayout)
        qCritical() << "productMethods not found";
    QLayoutItem *labelAndBoxesContentItem = productMethodsLayout->itemAt(0); //0 - labels and boxes widget, 1 - table
    if (!labelAndBoxesContentItem)
        qCritical() << "labelAndButtonsContent item not found";
    QWidget *labelAndBoxesContent = labelAndBoxesContentItem->widget();
    QHBoxLayout *labelAndBoxes = qobject_cast<QHBoxLayout *>(labelAndBoxesContent->layout());
    if (!labelAndBoxes)
        qCritical() << "labelAndButtons not found";
    QLayoutItem *spinBoxNumMethodsItem = labelAndBoxes->itemAt(2); // 0 - label, 1 - label, 2 - spinBox
    if (!spinBoxNumMethodsItem)
        qCritical() << "spinBoxNumMethods item not found";
    *spinBoxNumMethods = qobject_cast<QSpinBox *>(spinBoxNumMethodsItem->widget());
    if (!*spinBoxNumMethods)
        qCritical() << "spinBoxNumMethods not found";
    QLayoutItem *tableProductMethodsItem = productMethodsLayout->itemAt(1); //0 - labels and boxes widget, 1 - table
    if (!tableProductMethodsItem) {
        qCritical() << "tableProductMethods item not found";
    }
    *tableProductMethods = qobject_cast<QTableWidget *>(tableProductMethodsItem->widget());
    if (!*tableProductMethods)
        qCritical() << "tableProductMethods not found";
}

void MainWindow::writeParsedConstructorFromTable(QTableWidget *tableClassMethods, ClassMethod<ElementPtr> *classConstructor) {
    const int argsNum = classConstructor->getArgsNum();
    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
        Argument<ElementPtr> *arg = classConstructor->getArgument(argIndex);
        QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(0, 4+argIndex*3));

        const QString isConst = checkBoxConst->isChecked() ? "const" : "";
        arg->constFlag()->setText(isConst);

        const QString methodType = tableClassMethods->item(0, 5+argIndex*3)->text();
        arg->getType()->setText(methodType);
        const QString methodName = tableClassMethods->item(0, 6+argIndex*3)->text();
        arg->getName()->setText(methodName);
    }
}

void MainWindow::writeParsedMethodFromTable(QTableWidget *tableClassMethods, int methodIndex, ClassMethod<ElementPtr> *classMethod) {
    QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(methodIndex, 1));
    const QString isConst = checkBoxConst->isChecked() ? "const" : "";
    classMethod->constFlag()->setText(isConst);

    const QString methodType = tableClassMethods->item(methodIndex, 2)->text();
    classMethod->getType()->setText(methodType);
    const QString methodName = tableClassMethods->item(methodIndex, 3)->text();
    classMethod->getName()->setText(methodName);

    const int argsNum = classMethod->getArgsNum();
    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
        Argument<ElementPtr> *arg = classMethod->getArgument(argIndex);
        QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(methodIndex, 4+argIndex*3));

        const QString isConst = checkBoxConst->isChecked() ? "const" : "";
        arg->constFlag()->setText(isConst);

        const QString methodType = tableClassMethods->item(methodIndex, 5+argIndex*3)->text();
        arg->getType()->setText(methodType);
        const QString methodName = tableClassMethods->item(methodIndex, 6+argIndex*3)->text();
        arg->getName()->setText(methodName);
    }
}

void MainWindow::writeUiToParsedAbstractFactory() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;
    QLineEdit *lineEditFactoryName = nullptr;
    QSpinBox *spinBoxNumFactories = nullptr;
    QSpinBox *spinBoxNumProducts = nullptr;
    QListWidget *listOfFactories = nullptr;
    QListWidget *listOfProducts = nullptr;
    QHBoxLayout *layoutProductsMethodsList = nullptr;
    uiElements << reinterpret_cast<QObject **>(&lineEditFactoryName) << reinterpret_cast<QObject **>(&spinBoxNumFactories)
               << reinterpret_cast<QObject **>(&spinBoxNumProducts) << reinterpret_cast<QObject **>(&listOfFactories)
               << reinterpret_cast<QObject **>(&listOfProducts) << reinterpret_cast<QObject **>(&layoutProductsMethodsList);
    uiElementsName << "lineEditFactoryName" << "spinBoxNumFactories" << "spinBoxNumProducts" << "listOfFactories" << "listOfProducts"
                   << "layoutProductsMethodsList";
    Element *abstractFactoryName = nullptr;
    VectorElement *factoriesNames = nullptr;
    VectorElement *productsNames = nullptr;
    VectorElement *productsMethods = nullptr;
    elements << reinterpret_cast<BaseElement **>(&abstractFactoryName) << reinterpret_cast<BaseElement **>(&factoriesNames)
             << reinterpret_cast<BaseElement **>(&productsNames) << reinterpret_cast<BaseElement **>(&productsMethods);
    elementsName << "abstractFactoryName" << "factoriesNames" << "productsNames" << "productsMethods";
    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);
    abstractFactoryName->setText(lineEditFactoryName->text());
    const int factoriesNum = listOfFactories->count();
    const int productsNum = listOfProducts->count();
    for (int i = 0; i < factoriesNum; ++i)
        static_cast<Element *>((*factoriesNames)[i])->setText(listOfFactories->item(i)->text());
    for (int i = 0; i < productsNum; ++i)
        static_cast<Element *>((*productsNames)[i])->setText(listOfProducts->item(i)->text());

    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ClassMethods *productMethods = static_cast<ClassMethods *>((*productsMethods)[productIndex]);

        QSpinBox *spinBoxNumMethods; //don't need this actually
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethodsList->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = productMethods->getCount();
        for (int methodIndex = 0; methodIndex < methodsNum; ++methodIndex) {
            writeParsedMethodFromTable(tableProductMethods, methodIndex, (*productMethods)[methodIndex]);
        }
    }
}

void MainWindow::writeUiToParsedBuilder() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;

    QLineEdit *lineEditDirectorName = nullptr;
    QSpinBox *spinBoxDirectorMethodsNum = nullptr;
    QLineEdit *lineEditAbstractBuilderName = nullptr;
    QSpinBox *spinBoxAbstractBuilderMethodsNum = nullptr;
    QSpinBox *spinBoxBuildersNum = nullptr;
    QListWidget *listBuildersNames = nullptr;
    QListWidget *listProductsNames = nullptr;
    QTableWidget *directorMethods = nullptr;
    QTableWidget *abstractBuilderMethods = nullptr;
    QHBoxLayout *layoutProductsMethods = nullptr;
    uiElements << reinterpret_cast<QObject **>(&lineEditDirectorName) << reinterpret_cast<QObject **>(&spinBoxDirectorMethodsNum)
               << reinterpret_cast<QObject **>(&lineEditAbstractBuilderName) << reinterpret_cast<QObject **>(&spinBoxAbstractBuilderMethodsNum)
               << reinterpret_cast<QObject **>(&spinBoxBuildersNum) << reinterpret_cast<QObject **>(&listBuildersNames)
               << reinterpret_cast<QObject **>(&listProductsNames) << reinterpret_cast<QObject **>(&directorMethods)
               << reinterpret_cast<QObject **>(&abstractBuilderMethods) << reinterpret_cast<QObject **>(&layoutProductsMethods);
    uiElementsName << "lineEditDirectorName" << "spinBoxDirectorMethodsNum" << "lineEditAbstractBuilderName" << "spinBoxAbstractBuilderMethodsNum"
                   << "spinBoxBuildersNum" << "listBuildersNames" << "listProductsNames" << "directorMethods" << "abstractBuilderMethods"
                   << "layoutProductsMethods";

    Element *directorNameElement = nullptr;
    ClassMethods *directorMethodsElement = nullptr;
    Element *abstractBuilderNameElement = nullptr;
    ClassMethods *abstractBuilderMethodsElement = nullptr;
    VectorElement *buildersNamesElement = nullptr;
    VectorElement *productsNamesElement = nullptr;
    VectorElement *productsMethodsElement = nullptr;
    elements << reinterpret_cast<BaseElement **>(&directorNameElement) << reinterpret_cast<BaseElement **>(&directorMethodsElement)
             << reinterpret_cast<BaseElement **>(&abstractBuilderNameElement) << reinterpret_cast<BaseElement **>(&abstractBuilderMethodsElement)
             << reinterpret_cast<BaseElement **>(&buildersNamesElement) << reinterpret_cast<BaseElement **>(&productsNamesElement)
             << reinterpret_cast<BaseElement **>(&productsMethodsElement);
    elementsName << "directorName" << "directorMethods" << "abstractBuilderName" << "abstractBuilderMethods"
                 << "buildersNames" << "productsNames" << "productsMethods";

    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);

    const int directorMethodsNum = spinBoxDirectorMethodsNum->value();
    const int abstractBuilderMethodsNum = spinBoxAbstractBuilderMethodsNum->value();
    const int buildersNum = spinBoxBuildersNum->value();
    const int productsNum = buildersNum;
    directorNameElement->setText(lineEditDirectorName->text());
    abstractBuilderNameElement->setText(lineEditAbstractBuilderName->text());

    for (int i = 0; i < buildersNum; ++i) {
        BaseElement *builderNameBaseElement = (*buildersNamesElement)[i];
        Element *builderNameElement = static_cast<Element *>(builderNameBaseElement);
        if (!builderNameElement)
            qCritical() << "builderNameElement is not an Element in buildersNamesElement";
        BaseElement *productNameBaseElement = (*productsNamesElement)[i];
        Element *productNameElement = static_cast<Element *>(productNameBaseElement);
        if (!productNameElement)
            qCritical() << "productNameElement is not an Element in productsNamesElement";

        builderNameElement->setText(listBuildersNames->item(i)->text());
        productNameElement->setText(listProductsNames->item(i)->text());
    }

    for (int i = 0; i < directorMethodsNum; ++i) {
        writeParsedMethodFromTable(directorMethods, i, (*directorMethodsElement)[i]);
    }

    for (int i = 0; i < abstractBuilderMethodsNum; ++i) {
        writeParsedMethodFromTable(abstractBuilderMethods, i, (*abstractBuilderMethodsElement)[i]);
    }

    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ClassMethods *productMethods = static_cast<ClassMethods *>((*productsMethodsElement)[productIndex]);

        QSpinBox *spinBoxNumMethods;
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethods->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = spinBoxNumMethods->value(); // don't count constructor
        writeParsedConstructorFromTable(tableProductMethods, (*productMethods)[0]);
        for (int methodIndex = 1; methodIndex <= methodsNum; ++methodIndex) {
            writeParsedMethodFromTable(tableProductMethods, methodIndex, (*productMethods)[methodIndex]);
        }
    }
}

bool MainWindow::generateBuilder(int exportType) {
    bool success = true;

    QLineEdit *lineEditDirectorName = ui->centralwidget->findChild<QLineEdit *>("lineEditDirectorName");
    if (!lineEditDirectorName)
        qCritical() << "lineEditDirectorName not found";
    const QString directorName = lineEditDirectorName->text();

    QLineEdit *lineEditAbstractBuilderName = ui->centralwidget->findChild<QLineEdit *>("lineEditAbstractBuilderName");
    if (!lineEditAbstractBuilderName)
        qCritical() << "lineEditAbstractBuilderName not found";
    const QString abstractBuilderName = lineEditAbstractBuilderName->text();

    QListWidget *listBuildersNames = ui->centralwidget->findChild<QListWidget *>("listBuildersNames");
    if (!listBuildersNames)
        qCritical() << "listBuildersNames not found";
    const int buildersNum = listBuildersNames->count();
    QVector<QString> buildersNames(buildersNum);
    for (int builderIndex = 0; builderIndex < buildersNum; ++builderIndex) {
        buildersNames[builderIndex] = listBuildersNames->item(builderIndex)->text();
    }

    QListWidget *listProductsNames = ui->centralwidget->findChild<QListWidget *>("listProductsNames");
    if (!listProductsNames)
        qCritical() << "listProductsNames not found";
    const int productsNum = buildersNum;
    QVector<QString> productsNames(productsNum);
    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        productsNames[productIndex] = listProductsNames->item(productIndex)->text();
    }

    QScrollArea *productsMethods = ui->centralwidget->findChild<QScrollArea *>("productsMethods");
    QTableWidget *directorMethods = ui->centralwidget->findChild<QTableWidget *>("directorMethods");
    QTableWidget *abstractBuilderMethods = ui->centralwidget->findChild<QTableWidget *>("abstractBuilderMethods");
    QVector<QVector<ClassMethod<QString> *>> productsMethodsVec = getClassesMethodsFromScrollArea(productsMethods);
    QVector<ClassMethod<QString> *> directorMethodsVec = getClassMethodsFromTable(directorMethods);
    QVector<ClassMethod<QString> *> abstractBuilderMethodsVec = getClassMethodsFromTable(abstractBuilderMethods);

    switch (exportType) {
        case CLIPBOARD: {
            QString text;
            codeGenerator->genBuilder(&text, directorName, abstractBuilderName, buildersNames, productsNames,
                                      directorMethodsVec, abstractBuilderMethodsVec, productsMethodsVec);
            QApplication::clipboard()->setText(text);
            break;
        } case CPP_FILE: {
            QString text;
            codeGenerator->genBuilder(&text, directorName, abstractBuilderName, buildersNames, productsNames,
                                      directorMethodsVec, abstractBuilderMethodsVec, productsMethodsVec);
            const QString fileName = settings->value("Export/fileName").toString();
            const QString folderPath = getExportFolderPath();
            if (!writeTextToFile(folderPath + fileName + ".cpp", text))
                success = false;
            break;
        } case H_AND_CPP_FILES: {
            QVector<ClassText *> classTexts;
            codeGenerator->genBuilder(&classTexts, directorName, abstractBuilderName, buildersNames, productsNames,
                                      directorMethodsVec, abstractBuilderMethodsVec, productsMethodsVec);
            const QString folderPath = getExportFolderPath();
            const int classTextsNum = classTexts.count();
            for (int classTextIndex = 0; classTextIndex < classTextsNum; ++classTextIndex) {
                if (!writeTextToFile(folderPath + classTexts[classTextIndex]->getFileName() + \
                                     classTexts[classTextIndex]->getFileType(), classTexts[classTextIndex]->getText())) {
                    success = false;
                    continue;
                }
            }
            qDeleteAll(classTexts);
            break;
        }
    }

    for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
        qDeleteAll(productsMethodsVec[productItemIndex]);
    }
    qDeleteAll(directorMethodsVec);
    qDeleteAll(abstractBuilderMethodsVec);
    return success;
}

void MainWindow::on_pushBtnGenerate_clicked()
{
    const QString patternTypeName = ui->cmbBoxPatternName->currentText();
    const int patternType = patternTypesList->indexOf(patternTypeName);
    const int exportType = settings->value("Export/type", CLIPBOARD).toInt();
    if (exportType < CLIPBOARD or exportType > H_AND_CPP_FILES) {
        qWarning() << "export type settings corrupted";
        ui->statusBar->showMessage("export type settings corrupted", 5000);
    }
    bool success = false;

    switch (patternType) {
        case NO_PATTERN:
            return;
        case SINGLETON: {
            if (ui->checkBoxImport->isChecked()) {
                writeUiToParsedSingleton();
                success = parsedPattern->rewriteInFiles();
            } else
                success = generateSingleton(exportType);
            break;
        } case ABSTRACT_FACTORY: {
            if (ui->checkBoxImport->isChecked()) {
                writeUiToParsedAbstractFactory();
                success = parsedPattern->rewriteInFiles();
            } else
                success = generateAbstractFactory(exportType);
            break;
        } case BUILDER: {
            if (ui->checkBoxImport->isChecked()) {
                writeUiToParsedBuilder();
                success = parsedPattern->rewriteInFiles();
            } else
                success = generateBuilder(exportType);
            break;
        }default:
            qWarning() << "Unexpected pattern type";
            break;
    }

    if (success)
        statusBar()->showMessage("Code generated!", 5000);
}

void delWidgetsFromLayout(QLayout *layout) {
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            widget->deleteLater();
        if (QLayout *childLayout = item->layout())
            delWidgetsFromLayout(childLayout);
        delete item;
    }
}

void clearRowColFromGridLayout(QGridLayout *layout) {
    const int rowCount = layout->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        layout->setRowMinimumHeight(row, 0);
        layout->setRowStretch(row, 0);
    }
    const int columnCount = layout->columnCount();
    for (int column = 0; column < columnCount; ++column) {
        layout->setColumnMinimumWidth(column, 0);
        layout->setColumnStretch(column, 0);
    }
}

void clearGridLayout(QGridLayout *layout) {
    delWidgetsFromLayout(layout);
    clearRowColFromGridLayout(layout);
}

void addItemToListWidget(QListWidget *listWidget, const QString &itemName) {
    QListWidgetItem *item = new QListWidgetItem(itemName);
    item->setFlags(item->flags () | Qt::ItemIsEditable);
    listWidget->addItem(item);
}

void MainWindow::changeClassesNumInNamesList(QListWidget *listOfClasses, int nextClassesNum, const QString &className) {
    const int currClassesNum = listOfClasses->count();
    if (nextClassesNum > currClassesNum) {
        for (int i = currClassesNum; i < nextClassesNum; ++i) {
            addItemToListWidget(listOfClasses, className + QString::number(i + 1));
        }
    } else {
        for (int i = currClassesNum; i > nextClassesNum; --i) {
            delete listOfClasses->item(i-1);
        }
    }
}

void MainWindow::spinBoxNumFactoriesChanged(const int nextMethodsNum) {
    QListWidget *listOfFactories = ui->centralwidget->findChild<QListWidget *>("listOfFactories");
    if (!listOfFactories)
        qCritical() << "listOfFactories not found";
    changeClassesNumInNamesList(listOfFactories, nextMethodsNum, "Factory");
}

void MainWindow::changeClassesNumInNamesListAndMethodsList(QListWidget *listOfClasses, QHBoxLayout *layoutMethodsList,
                                               int nextProductsNum, const QString &className, bool withConstructor) {
    const int currProductsNum = listOfClasses->count();
    if (nextProductsNum > currProductsNum) {
        for (int i = currProductsNum; i < nextProductsNum; ++i) {
            addItemToListWidget(listOfClasses, className + QString::number(i + 1));
            addItemToLayoutMethodsList(layoutMethodsList, className + QString::number(i + 1), withConstructor);
        }
    } else {
        for (int i = currProductsNum; i > nextProductsNum; --i) {
            delete listOfClasses->item(i-1);
            delItemFromLayoutProductsMethodsList(layoutMethodsList, i-1);
        }
    }
}

void MainWindow::spinBoxNumProductsChanged(const int nextProductsNum) {
    QListWidget *listOfProducts = ui->centralwidget->findChild<QListWidget *>("listOfProducts");
    if (!listOfProducts)
        qCritical() << "listOfProducts not found";
    QScrollArea *listOfProductsMethods = ui->centralwidget->findChild<QScrollArea *>("listOfProductsMethods");
    if (!listOfProductsMethods)
        qCritical() << "listOfProductsMethods not found";
    QWidget *listOfProductsMethodsWidget = listOfProductsMethods->widget();
    if (!listOfProductsMethodsWidget)
        qCritical() << "listOfProductsMethods widget not found";
    QHBoxLayout *layoutProductsMethodsList = qobject_cast<QHBoxLayout *>(listOfProductsMethodsWidget->layout());
    if (!layoutProductsMethodsList)
        qCritical() << "layoutProductsMethodsList not found";
    changeClassesNumInNamesListAndMethodsList(listOfProducts, layoutProductsMethodsList, nextProductsNum, "Product", false);
}

void addCheckBoxConstToCell(QTableWidget *table, const int rowIndex, const int columnIndex) {
    QWidget *checkBoxConstContent = new QWidget;
    QHBoxLayout *checkBoxConstLayout = new QHBoxLayout;
    QCheckBox *checkBoxConst = new QCheckBox;

    checkBoxConst->setStyleSheet("QCheckBox::indicator { width:25px; height: 25px;}");
    checkBoxConstContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    checkBoxConstLayout->setMargin(0);
    checkBoxConstLayout->setAlignment(Qt::AlignCenter);

    checkBoxConstContent->setLayout(checkBoxConstLayout);
    checkBoxConstLayout->addWidget(checkBoxConst);

    table->setCellWidget(rowIndex, columnIndex, checkBoxConstContent);
}

int findMaxArgsNumInTable(QTableWidget *tableProductMethods, const int currMethodIndex) {
    const int methodsNum = tableProductMethods->rowCount();
    int maxArgsNum = 0;

    for (int i = 0; i < currMethodIndex; ++i) {
        const int argsNum = getClassMethodArgsNum(tableProductMethods->cellWidget(i, 0));
        if (argsNum > maxArgsNum)
            maxArgsNum = argsNum;
    }
    for (int i = currMethodIndex+1; i < methodsNum; ++i) {
        const int argsNum = getClassMethodArgsNum(tableProductMethods->cellWidget(i, 0));
        if (argsNum > maxArgsNum)
            maxArgsNum = argsNum;
    }

    return  maxArgsNum;
}

void MainWindow::changeArgsCountInTable(const int nextArgsNum) {
    QWidget *spinBoxNumArgsContent = qobject_cast<QWidget *>(QObject::sender()->parent()->parent());
    if (!spinBoxNumArgsContent)
        qCritical() << "spinBoxNumArgsContent not found";
    QTableWidget *tableProductMethods = qobject_cast<QTableWidget *>(spinBoxNumArgsContent->parent());
    if (!tableProductMethods)
        qCritical() << "tableProductMethods not found";
    const int currArgsNum = (tableProductMethods->columnCount()-4)/3;
    const int currMethodsNum = tableProductMethods->rowCount();

    if (nextArgsNum > currArgsNum) {
        tableProductMethods->setColumnCount(4+nextArgsNum*3);
        for (int i = currArgsNum; i < nextArgsNum; ++i) {
            tableProductMethods->setHorizontalHeaderItem(4+i*3, new QTableWidgetItem("Arg"+QString::number(i+1)+" const"));
            tableProductMethods->setHorizontalHeaderItem(5+i*3, new QTableWidgetItem("Arg"+QString::number(i+1)+" type"));
            tableProductMethods->setHorizontalHeaderItem(6+i*3, new QTableWidgetItem("Arg"+QString::number(i+1)+" name"));
            for (int j = 0; j < currMethodsNum; ++j) {
                addCheckBoxConstToCell(tableProductMethods, j, 4+i*3);
                tableProductMethods->setItem(j, 5+i*3, new QTableWidgetItem(""));
                tableProductMethods->setItem(j, 6+i*3, new QTableWidgetItem(""));
            }
        }
    } else {
        const int currMethodIndex = QObject::sender()->objectName().toInt(); // spinBoxNumArgs in addSpinBoxNumArgsToCell
        const int minPossibleArgsNum = qMax(findMaxArgsNumInTable(tableProductMethods, currMethodIndex), nextArgsNum);
        tableProductMethods->setColumnCount(4+minPossibleArgsNum*3);
    }
}

void MainWindow::addSpinBoxNumArgsToCell(QTableWidget *table, const int rowIndex, const int columnIndex) {
    QWidget *spinBoxNumArgsContent = new QWidget;
    QHBoxLayout *spinBoxNumArgsLayout = new QHBoxLayout;
    QSpinBox *spinBoxNumArgs = new QSpinBox;

    spinBoxNumArgs->setFixedSize(60, 30);
    spinBoxNumArgs->setObjectName(QString::number(rowIndex));
    spinBoxNumArgsContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    spinBoxNumArgsLayout->setMargin(0);
    spinBoxNumArgsLayout->setAlignment(Qt::AlignCenter);

    spinBoxNumArgsContent->setLayout(spinBoxNumArgsLayout);
    spinBoxNumArgsLayout->addWidget(spinBoxNumArgs);

    table->setCellWidget(rowIndex, columnIndex, spinBoxNumArgsContent);

    connect(spinBoxNumArgs, SIGNAL(valueChanged(int)), this, SLOT(changeArgsCountInTable(int)));
}

void MainWindow::addRowToMethodsTable(unsigned i, QTableWidget *tableMethods) {
    addSpinBoxNumArgsToCell(tableMethods, i, 0);
    addCheckBoxConstToCell(tableMethods, i, 1);
    tableMethods->setItem(i, 2, new QTableWidgetItem(""));
    tableMethods->setItem(i, 3, new QTableWidgetItem(""));
    const int currArgsNum = int((tableMethods->columnCount()-4)/3);
    for (int j = 0; j < currArgsNum; ++j) {
        addCheckBoxConstToCell(tableMethods, i, 4+j*3);
        tableMethods->setItem(i, 5+j*3, new QTableWidgetItem(""));
        tableMethods->setItem(i, 6+j*3, new QTableWidgetItem(""));
    }
}

void MainWindow::changeMethodsCountInTable(int nextMethodsNum, bool withConstructor) {
    QWidget *methodsContent = qobject_cast<QWidget *>(QObject::sender()->parent()->parent());
    if (!methodsContent)
        qCritical() << "methodsContent not found";
    QVBoxLayout *methodsLayout = qobject_cast<QVBoxLayout *>(methodsContent->layout());
    if (!methodsLayout)
        qCritical() << "methodsLayout not found";
    QLayoutItem *tableMethodsItem = methodsLayout->itemAt(1); //0 - labels and boxes widget, 1 - table
    if (!tableMethodsItem)
        qCritical() << "tableMethodsItem item not found";
    QTableWidget *tableMethods = qobject_cast<QTableWidget *>(tableMethodsItem->widget());
    if (!tableMethods)
        qCritical() << "tableMethods not found";

    const int currMethodsNum = tableMethods->rowCount();
    if (withConstructor)
        nextMethodsNum++;
    tableMethods->setRowCount(nextMethodsNum);
    if (nextMethodsNum > currMethodsNum) {
        for (int i = currMethodsNum; i < nextMethodsNum; ++i) {
            addRowToMethodsTable(i, tableMethods);
        }
    }
}

void MainWindow::delItemFromLayoutProductsMethodsList(QHBoxLayout *layoutProductsMethodsList, const int index) {
    QLayoutItem *productMethodsContentItem = layoutProductsMethodsList->itemAt(index);
    if (!productMethodsContentItem)
        qCritical() << "productMethodsContent item not found";
    layoutProductsMethodsList->removeItem(productMethodsContentItem);
    productMethodsContentItem->widget()->deleteLater();
    delete productMethodsContentItem;
    QWidget *widget = layoutProductsMethodsList->parentWidget();
    widget->setFixedWidth(widget->minimumWidth()-TableOfMethodsWidth);
}

QTableWidget *MainWindow::makeMethodsTable(bool withConstructor, const QString &className) {
    QTableWidget *tableMethods = new QTableWidget;
    tableMethods->setFont(QFont("MS Shell Dlg 2", 12));
    tableMethods->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableMethods->setRowCount(0);
    tableMethods->setColumnCount(4);
    tableMethods->setHorizontalHeaderItem(0, new QTableWidgetItem("Num args"));
    tableMethods->setColumnWidth(0, 100);
    tableMethods->setHorizontalHeaderItem(1, new QTableWidgetItem("Const"));
    tableMethods->setColumnWidth(1, 80);
    tableMethods->setHorizontalHeaderItem(2, new QTableWidgetItem("Output type"));
    tableMethods->setHorizontalHeaderItem(3, new QTableWidgetItem("Name"));
    if (withConstructor) {
        tableMethods->setRowCount(1);
        addRowToMethodsTable(0, tableMethods);
        tableMethods->cellWidget(0, 1)->setEnabled(false);
        tableMethods->item(0, 2)->setFlags(tableMethods->item(0, 2)->flags() ^ Qt::ItemIsEditable);
        tableMethods->item(0, 3)->setText(className);
        tableMethods->item(0, 3)->setFlags(tableMethods->item(0, 3)->flags() ^ Qt::ItemIsEditable);
    }
    return tableMethods;
}

void MainWindow::addItemToLayoutMethodsList(QHBoxLayout *layoutMethodsList, const QString &className, bool withConstructor) {
    QWidget *methodsContent = new QWidget;
    QVBoxLayout *methodsLayout = new QVBoxLayout;
    QWidget *labelAndBoxesContent = new QWidget;
    QHBoxLayout *labelAndBoxes = new QHBoxLayout;
    QLabel *labelMethods = new QLabel(className + " methods:");
    QLabel *lblNumMethods = new QLabel("Num methods:");
    QSpinBox *spinBoxNumMethods = new QSpinBox;
    QTableWidget *tableMethods = makeMethodsTable(withConstructor, className);

    methodsContent->setLayout(methodsLayout);
    methodsLayout->setMargin(0);
    methodsLayout->addWidget(labelAndBoxesContent);
    methodsLayout->addWidget(tableMethods);
    labelAndBoxesContent->setLayout(labelAndBoxes);
    labelAndBoxes->setMargin(0);
    labelAndBoxes->addWidget(labelMethods);
    labelAndBoxes->addWidget(lblNumMethods);
    labelAndBoxes->addWidget(spinBoxNumMethods);
    labelMethods->setFont(QFont("MS Shell Dlg 2", 12));
    labelMethods->setFixedHeight(30);
    lblNumMethods->setFont(QFont("MS Shell Dlg 2", 12));
    lblNumMethods->setFixedSize(130, 30);
    spinBoxNumMethods->setFont(QFont("MS Shell Dlg 2", 12));
    spinBoxNumMethods->setFixedSize(60, 30);
    spinBoxNumMethods->setRange(0, 99);

    QWidget *widget = layoutMethodsList->parentWidget();
    widget->setFixedWidth(widget->minimumWidth()+TableOfMethodsWidth);

    layoutMethodsList->addWidget(methodsContent);

    connect(spinBoxNumMethods, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, withConstructor](int nextMethodsNum){
        changeMethodsCountInTable(nextMethodsNum, withConstructor);
    });
}

void MainWindow::changeConstructorNameInTable(QListWidgetItem *classNameItem) {
    QListWidget *listOfClasses = classNameItem->listWidget();
    const int index = listOfClasses->currentRow();
    QStringList classesNames, classesMethods;
    classesNames << "listOfProducts" << "listProductsNames";
    classesMethods << "listOfProductsMethods" << "productsMethods";
    const int classNameIndex = classesNames.indexOf(listOfClasses->objectName());
    if (classNameIndex >= classesMethods.count())
        qCritical() << "Unknown classes methods list (QScrollArea)";
    QScrollArea *listOfClassesMethods = ui->centralwidget->findChild<QScrollArea *>(classesMethods[classNameIndex]);
    if (!listOfClassesMethods)
        qCritical() << "listOfClassesMethods not found";
    QWidget *contentOfListOfClassesMethods = listOfClassesMethods->widget();
    QHBoxLayout *layoutClassesMethodsList = qobject_cast<QHBoxLayout *>(contentOfListOfClassesMethods->layout());
    if (!layoutClassesMethodsList)
        qCritical() << "layoutClassesMethodsList not found";
    QLayoutItem *classMethodsContentItem = layoutClassesMethodsList->itemAt(index);
    if (!classMethodsContentItem)
        qCritical() << "classMethodsContentItem item not found";
    QWidget *classMethodsContent = classMethodsContentItem->widget();
    QVBoxLayout *classMethods = qobject_cast<QVBoxLayout *>(classMethodsContent->layout());
    if (!classMethods)
        qCritical() << "classMethods not found";
    QLayoutItem *tableMethodsItem = classMethods->itemAt(1); //0 - labels and boxes widget, 1 - table
    if (!tableMethodsItem)
        qCritical() << "tableMethods item not found";
    QTableWidget *tableMethods = qobject_cast<QTableWidget *>(tableMethodsItem->widget());
    if (!tableMethods)
        qCritical() << "tableMethods not found";
    QTableWidgetItem *constructorNameItem = tableMethods->item(0, 3);
    if (!constructorNameItem)
        qCritical() << "constructor name item not found";
    constructorNameItem->setText(classNameItem->text());
}

void MainWindow::changeNameInTable(QListWidgetItem *classNameItem) {
    QListWidget *listOfClasses = classNameItem->listWidget();
    const int index = listOfClasses->currentRow();
    QStringList classesNames, classesMethods;
    classesNames << "listOfProducts" << "listProductsNames";
    classesMethods << "listOfProductsMethods" << "productsMethods";
    const int classNameIndex = classesNames.indexOf(listOfClasses->objectName());
    if (classNameIndex >= classesMethods.count())
        qCritical() << "Unknown classes methods list (QScrollArea)";
    QScrollArea *listOfClassesMethods = ui->centralwidget->findChild<QScrollArea *>(classesMethods[classNameIndex]);
    if (!listOfClassesMethods)
        qCritical() << "listOfClassesMethods not found";
    QWidget *contentOfListOfClassesMethods = listOfClassesMethods->widget();
    QHBoxLayout *layoutClassesMethodsList = qobject_cast<QHBoxLayout *>(contentOfListOfClassesMethods->layout());
    if (!layoutClassesMethodsList)
        qCritical() << "layoutClassesMethodsList not found";
    QLayoutItem *classMethodsContentItem = layoutClassesMethodsList->itemAt(index);
    if (!classMethodsContentItem)
        qCritical() << "classMethodsContentItem item not found";
    QWidget *classMethodsContent = classMethodsContentItem->widget();
    QVBoxLayout *classMethods = qobject_cast<QVBoxLayout *>(classMethodsContent->layout());
    if (!classMethods)
        qCritical() << "classMethods not found";
    QLayoutItem *labelAndButtonsContentItem = classMethods->itemAt(0); //0 - labels and boxes widget, 1 - table
    if (!labelAndButtonsContentItem)
        qCritical() << "labelAndButtonsContent item not found";
    QWidget *labelAndButtonsContent = labelAndButtonsContentItem->widget();
    QHBoxLayout *labelAndButtons = qobject_cast<QHBoxLayout *>(labelAndButtonsContent->layout());
    if (!labelAndButtons)
        qCritical() << "labelAndButtons not found";
    QLayoutItem *labelProductMethodsItem = labelAndButtons->itemAt(0);
    if (!labelProductMethodsItem)
        qCritical() << "labelProductMethods item not found";
    QLabel *labelProductMethods = qobject_cast<QLabel *>(labelProductMethodsItem->widget());
    if (!labelProductMethods)
        qCritical() << "labelProductMethods not found";
    labelProductMethods->setText(classNameItem->text() + " methods:");
}

void clearParseData(QHash<QString, QVector<ClassText *>> *parseData) {
    foreach (QVector<ClassText *> classTexts, *parseData) {
        qDeleteAll(classTexts);
    }
    parseData->clear();
}

void MainWindow::comboBox_indexChanged() {
    ui->pushBtnGenerate->setEnabled(not ui->checkBoxImport->isChecked());
    ui->checkBoxImport->setEnabled(true);
    unfreezeUi();
    clearParseData(&parseData);
    delete parsedPattern;
    parsedPattern = nullptr;

    const QString patternType = ui->cmbBoxPatternName->currentText();
    const int patternTypeIndex = patternTypesList->indexOf(patternType);

    clearGridLayout(ui->gridLayoutSpecial);

    switch (patternTypeIndex) {
        case NO_PATTERN:
            ui->gridLayoutSpecial->setRowStretch(0, 1);
            break;
        case SINGLETON: {
            QLabel *labelSngltn = new QLabel("Enter class name:");
            QLineEdit *lineEditSngltn = new QLineEdit;

            labelSngltn->setMinimumSize(200, 40);
            lineEditSngltn->setMinimumSize(200, 40);
            lineEditSngltn->setObjectName("lineEditSngltn");

            ui->gridLayoutSpecial->addWidget(labelSngltn, 0, 0);
            ui->gridLayoutSpecial->addWidget(lineEditSngltn, 0, 1);

            ui->gridLayoutSpecial->setRowStretch(1, 1);
            break;
        } case ABSTRACT_FACTORY: {
            QGridLayout *gridLayoutSpecial1 = new QGridLayout;

            QGridLayout *gridLayoutSpecial11 = new QGridLayout;
            QSpinBox *spinBoxNumFactories = new QSpinBox;
            QSpinBox *spinBoxNumProducts = new QSpinBox;
            QLabel *labelNumFactories = new QLabel("Enter num of factories:");
            QLabel *labelNumProducts = new QLabel("Enter num of products:");

            QVBoxLayout *ptrTypeAndFactoryNameLayout = new QVBoxLayout;
            QHBoxLayout *factoryNameLayout = new QHBoxLayout;
            QLabel *lblFactoryName = new QLabel("Abstract factory name:");
            QLineEdit *lineEditFactoryName = new QLineEdit;
            QHBoxLayout *ptrTypeLayout = new QHBoxLayout;
            QRadioButton *btnRawPointer = new QRadioButton("raw pointer");
            QRadioButton *btnUniquePointer = new QRadioButton("unique pointer");
            QRadioButton *btnSharedPointer = new QRadioButton("shared pointer");

            QGridLayout *gridLayoutSpecial2 = new QGridLayout;
            QListWidget *listOfFactories = new QListWidget();
            QListWidget *listOfProducts = new QListWidget();

            QGridLayout *gridLayoutSpecial3 = new QGridLayout;
            QLabel *labelFunctions = new QLabel("Enter products methods:");
            QScrollArea *listOfProductsMethods = new QScrollArea;
            QWidget *contentOfListOfProductsMethods = new QWidget;
            QHBoxLayout *layoutProductsMethodsList = new QHBoxLayout;

            spinBoxNumFactories->setFixedSize(80, 40);
            spinBoxNumFactories->setMinimum(1);
            spinBoxNumFactories->setMaximum(99);
            spinBoxNumFactories->setObjectName("spinBoxNumFactories");
            spinBoxNumProducts->setFixedSize(80, 40);
            spinBoxNumProducts->setMinimum(1);
            spinBoxNumProducts->setMaximum(99);
            spinBoxNumProducts->setObjectName("spinBoxNumProducts");
            labelNumFactories->setMinimumSize(260, 40);
            labelNumProducts->setMinimumSize(260, 40);

            ptrTypeAndFactoryNameLayout->setContentsMargins(20, 0, 0, 0);
            lineEditFactoryName->setObjectName("lineEditFactoryName");
            btnRawPointer->setChecked(true);
            btnRawPointer->setObjectName("btnRawPointer");
            btnUniquePointer->setObjectName("btnUniquePointer");
            btnSharedPointer->setObjectName("btnSharedPointer");

            listOfFactories->setObjectName("listOfFactories");
            listOfFactories->setMaximumHeight(300);
            listOfFactories->setFont(QFont("MS Shell Dlg 2", 12));
            listOfFactories->setEditTriggers(QAbstractItemView::AllEditTriggers);
            addItemToListWidget(listOfFactories, "Factory1");
            listOfProducts->setObjectName("listOfProducts");
            listOfProducts->setMaximumHeight(300);
            listOfProducts->setFont(QFont("MS Shell Dlg 2", 12));
            listOfProducts->setEditTriggers(QAbstractItemView::AllEditTriggers);
            addItemToListWidget(listOfProducts, "Product1");

            labelFunctions->setAlignment(Qt::AlignCenter);
            contentOfListOfProductsMethods->setLayout(layoutProductsMethodsList);
            layoutProductsMethodsList->setObjectName("layoutProductsMethodsList");
            contentOfListOfProductsMethods->setFixedSize(16, TableOfMethodsHeight);
            listOfProductsMethods->setMinimumHeight(200);
            listOfProductsMethods->setWidget(contentOfListOfProductsMethods);
            listOfProductsMethods->setObjectName("listOfProductsMethods");
            addItemToLayoutMethodsList(layoutProductsMethodsList, "Product1", false);

            ui->gridLayoutSpecial->addLayout(gridLayoutSpecial1, 0, 0);
            gridLayoutSpecial1->addLayout(gridLayoutSpecial11, 0, 0);
            gridLayoutSpecial1->addLayout(ptrTypeAndFactoryNameLayout, 0, 2);
            gridLayoutSpecial11->addWidget(labelNumFactories, 0, 0);
            gridLayoutSpecial11->addWidget(spinBoxNumFactories, 0, 1);
            gridLayoutSpecial11->addWidget(labelNumProducts, 1, 0);
            gridLayoutSpecial11->addWidget(spinBoxNumProducts, 1, 1);
            ptrTypeAndFactoryNameLayout->addLayout(factoryNameLayout);
            factoryNameLayout->addWidget(lblFactoryName);
            factoryNameLayout->addWidget(lineEditFactoryName);
            ptrTypeAndFactoryNameLayout->addLayout(ptrTypeLayout);
            ptrTypeLayout->addWidget(btnRawPointer);
            ptrTypeLayout->addWidget(btnUniquePointer);
            ptrTypeLayout->addWidget(btnSharedPointer);
            ui->gridLayoutSpecial->addLayout(gridLayoutSpecial2, 1, 0);
            gridLayoutSpecial2->addWidget(listOfFactories, 0, 0);
            gridLayoutSpecial2->addWidget(listOfProducts, 0, 1);
            ui->gridLayoutSpecial->addLayout(gridLayoutSpecial3, 2, 0);
            gridLayoutSpecial3->addWidget(labelFunctions, 0, 0);
            gridLayoutSpecial3->addWidget(listOfProductsMethods, 1, 0);

            gridLayoutSpecial1->setColumnStretch(1, 1);
            gridLayoutSpecial1->setColumnStretch(3, 1);

            connect(spinBoxNumFactories, SIGNAL(valueChanged(int)), this, SLOT(spinBoxNumFactoriesChanged(int)));
            connect(spinBoxNumProducts, SIGNAL(valueChanged(int)), this, SLOT(spinBoxNumProductsChanged(int)));
            connect(listOfProducts, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(changeNameInTable(QListWidgetItem*)));
            break;
        } case BUILDER: {
            QHBoxLayout *layoutDirector = new QHBoxLayout;
            QLabel *lblDirectorConfigsNum = new QLabel("Director's config methods num:");
            QSpinBox *spinBoxDirectorMethodsNum = new QSpinBox;
            QLabel *lblDirectorName = new QLabel("Enter director name:");
            QLineEdit *lineEditDirectorName = new QLineEdit;

            lblDirectorConfigsNum->setFixedWidth(330);
            lblDirectorName->setFixedWidth(310);
            spinBoxDirectorMethodsNum->setFixedSize(80, 40);
            spinBoxDirectorMethodsNum->setMinimum(1);
            spinBoxDirectorMethodsNum->setMaximum(99);
            spinBoxDirectorMethodsNum->setObjectName("spinBoxDirectorMethodsNum");
            lblDirectorName->setContentsMargins(20, 0, 0, 0);
            lineEditDirectorName->setObjectName("lineEditDirectorName");

            layoutDirector->addWidget(lblDirectorConfigsNum);
            layoutDirector->addWidget(spinBoxDirectorMethodsNum);
            layoutDirector->addWidget(lblDirectorName);
            layoutDirector->addWidget(lineEditDirectorName);
            ui->gridLayoutSpecial->addLayout(layoutDirector, 1, 0);

            QHBoxLayout *layoutAbstractBuilder = new QHBoxLayout;
            QLabel *lblAbstractBuilderNumMethods = new QLabel("Abstract builder's methods num:");
            QSpinBox *spinBoxAbstractBuilderMethodsNum = new QSpinBox;
            QLabel *lblAbstractBuilderName = new QLabel("Enter abstract builder name:");
            QLineEdit *lineEditAbstractBuilderName = new QLineEdit;

            lblAbstractBuilderNumMethods->setFixedWidth(330);
            lblAbstractBuilderName->setFixedWidth(310);
            spinBoxAbstractBuilderMethodsNum->setFixedSize(80, 40);
            spinBoxAbstractBuilderMethodsNum->setMinimum(1);
            spinBoxAbstractBuilderMethodsNum->setMaximum(99);
            spinBoxAbstractBuilderMethodsNum->setObjectName("spinBoxAbstractBuilderMethodsNum");
            lblAbstractBuilderName->setContentsMargins(20, 0, 0, 0);
            lineEditAbstractBuilderName->setObjectName("lineEditAbstractBuilderName");

            layoutAbstractBuilder->addWidget(lblAbstractBuilderNumMethods);
            layoutAbstractBuilder->addWidget(spinBoxAbstractBuilderMethodsNum);
            layoutAbstractBuilder->addWidget(lblAbstractBuilderName);
            layoutAbstractBuilder->addWidget(lineEditAbstractBuilderName);
            ui->gridLayoutSpecial->addLayout(layoutAbstractBuilder, 2, 0);

            QWidget *buildersNumContent = new QWidget;
            QHBoxLayout *layoutBuildersNum = new QHBoxLayout;
            QLabel *lblBuildersNum = new QLabel("Builders (products) num:");
            QSpinBox *spinBoxBuildersNum = new QSpinBox;

            buildersNumContent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            layoutBuildersNum->setMargin(0);
            lblBuildersNum->setFixedWidth(330);
            spinBoxBuildersNum->setFixedSize(80, 40);
            spinBoxBuildersNum->setMinimum(1);
            spinBoxBuildersNum->setMaximum(99);
            spinBoxBuildersNum->setObjectName("spinBoxBuildersNum");

            buildersNumContent->setLayout(layoutBuildersNum);
            layoutBuildersNum->addWidget(lblBuildersNum, 0, Qt::AlignLeft);
            layoutBuildersNum->addWidget(spinBoxBuildersNum, 0, Qt::AlignLeft);
            ui->gridLayoutSpecial->addWidget(buildersNumContent, 3, 0);

            QGridLayout *layoutNames = new QGridLayout;
            QLabel *lblBuildersNames = new QLabel("Builders names:");
            QListWidget *listBuildersNames = new QListWidget;
            QLabel *lblProductsNames = new QLabel("Products names:");
            QListWidget *listProductsNames = new QListWidget;

            listBuildersNames->setFont(QFont("MS Shell Dlg 2", 12));
            listBuildersNames->setObjectName("listBuildersNames");
            listProductsNames->setFont(QFont("MS Shell Dlg 2", 12));
            listProductsNames->setObjectName("listProductsNames");
            addItemToListWidget(listBuildersNames, "Builder1");
            addItemToListWidget(listProductsNames, "Product1");

            layoutNames->addWidget(lblBuildersNames, 0, 0);
            layoutNames->addWidget(listBuildersNames, 1, 0);
            layoutNames->addWidget(lblProductsNames, 0, 1);
            layoutNames->addWidget(listProductsNames, 1, 1);
            ui->gridLayoutSpecial->addLayout(layoutNames, 4, 0);

            QLabel *labelMethods = new QLabel("Methods:");
            labelMethods->setAlignment(Qt::AlignCenter);
            ui->gridLayoutSpecial->addWidget(labelMethods, 5, 0);

            QTabWidget *tabWidgetMethods = new QTabWidget;
            QTableWidget *directorMethods = makeMethodsTable(false);
            QTableWidget *abstractBuilderMethods = makeMethodsTable(false);
            QScrollArea *productsMethods = new QScrollArea;
            QWidget *productsMethodsContent = new QWidget;
            QHBoxLayout *layoutProductsMethods = new QHBoxLayout;

            tabWidgetMethods->setMinimumHeight(220);
            directorMethods->setRowCount(1);
            addRowToMethodsTable(0, directorMethods);
            directorMethods->setObjectName("directorMethods");
            abstractBuilderMethods->setRowCount(1);
            addRowToMethodsTable(0, abstractBuilderMethods);
            abstractBuilderMethods->setObjectName("abstractBuilderMethods");
            productsMethods->setObjectName("productsMethods");
            productsMethodsContent->setFixedSize(16, TableOfMethodsHeight);
            layoutProductsMethods->setObjectName("layoutProductsMethods");

            tabWidgetMethods->addTab(directorMethods, "Director");
            tabWidgetMethods->addTab(abstractBuilderMethods, "Abstract builder");
            tabWidgetMethods->addTab(productsMethods, "Products");
            productsMethods->setWidget(productsMethodsContent);
            productsMethodsContent->setLayout(layoutProductsMethods);
            addItemToLayoutMethodsList(layoutProductsMethods, "Product1", true);
            ui->gridLayoutSpecial->addWidget(tabWidgetMethods, 6, 0);

            connect(spinBoxDirectorMethodsNum, SIGNAL(valueChanged(int)), this, SLOT(spinBoxDirectorMethodsNumChanged(int)));
            connect(spinBoxAbstractBuilderMethodsNum, SIGNAL(valueChanged(int)), this, SLOT(spinBoxAbstractBuilderMethodsNumChanged(int)));
            connect(spinBoxBuildersNum, SIGNAL(valueChanged(int)), this, SLOT(spinBoxNumBuildersChanged(int)));
            connect(listProductsNames, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(changeNameInTableAndConstructorName(QListWidgetItem*)));
            break;
        } default:
            qWarning() << "Unexpected pattern type";
            break;
    }
}

void MainWindow::changeNameInTableAndConstructorName(QListWidgetItem *classNameItem) {
    changeNameInTable(classNameItem);
    changeConstructorNameInTable(classNameItem);
}

void MainWindow::spinBoxAbstractBuilderMethodsNumChanged(int nextAbstractBuilderMethodsNum) {
    QTableWidget *abstractBuilderMethods = ui->centralwidget->findChild<QTableWidget *>("abstractBuilderMethods");
    if (!abstractBuilderMethods)
        qCritical() << "abstractBuilderMethods not found";
    const int currAbstractBuilderMethodsNum = abstractBuilderMethods->rowCount();
    abstractBuilderMethods->setRowCount(nextAbstractBuilderMethodsNum);
    if (nextAbstractBuilderMethodsNum > currAbstractBuilderMethodsNum) {
        for (int i = currAbstractBuilderMethodsNum; i < nextAbstractBuilderMethodsNum; ++i) {
            addRowToMethodsTable(i, abstractBuilderMethods);
        }
    }
}

void MainWindow::spinBoxDirectorMethodsNumChanged(int nextDirectorMethodsNum) {
    QTableWidget *directorMethods = ui->centralwidget->findChild<QTableWidget *>("directorMethods");
    if (!directorMethods)
        qCritical() << "directorMethods not found";
    const int currDirectorMethodsNum = directorMethods->rowCount();
    directorMethods->setRowCount(nextDirectorMethodsNum);
    if (nextDirectorMethodsNum > currDirectorMethodsNum) {
        for (int i = currDirectorMethodsNum; i < nextDirectorMethodsNum; ++i) {
            addRowToMethodsTable(i, directorMethods);
        }
    }
}

void MainWindow::spinBoxNumBuildersChanged(int nextBuildersNum) {
    QListWidget *listBuildersNames = ui->centralwidget->findChild<QListWidget *>("listBuildersNames");
    if (!listBuildersNames)
        qCritical() << "listBuildersNames not found";
    changeClassesNumInNamesList(listBuildersNames, nextBuildersNum, "Builder");

    //products num = builders num
    QListWidget *listProductsNames = ui->centralwidget->findChild<QListWidget *>("listProductsNames");
    if (!listBuildersNames)
        qCritical() << "listProductsNames not found";
    QScrollArea *productsMethods = ui->centralwidget->findChild<QScrollArea *>("productsMethods");
    if (!productsMethods)
        qCritical() << "productsMethods not found";
    QWidget *productsMethodsContent = productsMethods->widget();
    if (!productsMethodsContent)
        qCritical() << "productsMethodsContent widget not found";
    QHBoxLayout *layoutProductsMethods = qobject_cast<QHBoxLayout *>(productsMethodsContent->layout());
    if (!layoutProductsMethods)
        qCritical() << "layoutProductsMethods not found";
    changeClassesNumInNamesListAndMethodsList(listProductsNames, layoutProductsMethods, nextBuildersNum, "Product", true);
}

void MainWindow::on_actionExport_triggered() {
    ExportWindow *exportWindow = new ExportWindow(this, settings);
    exportWindow->exec();
}

bool MainWindow::makeParseData() {
    QHashIterator<QString, QStringList> i(importData);
    while (i.hasNext()) {
        i.next();
        const QStringList fileNamesList = i.value();
        QVector<ClassText *> classTexts;
        const int fileNamesNum = fileNamesList.count();
        classTexts.resize(fileNamesNum);
        for (int fileNameIndex = 0; fileNameIndex < fileNamesNum; ++fileNameIndex) {
            QString text;
            const QString fileName = fileNamesList[fileNameIndex];
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "Can't open file " + fileName;
                ui->statusBar->showMessage("Can't open file: " + fileName, 5000);
                return false;
            }
            QTextStream stream(&file);
            QString fileType;
            if (fileName[fileName.length()-1] == "h")
                fileType = ".h";
            else
                fileType = ".cpp";
            classTexts[fileNameIndex] = new ClassText(fileName, stream.readAll(), fileType);
            file.close();
        }
        parseData.insert(i.key(), classTexts);
    }
    return true;
}

void MainWindow::writeParsedSingletonToUi() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;
    QLineEdit *lineEditSngltn = nullptr;
    uiElements.append(reinterpret_cast<QObject **>(&lineEditSngltn));
    uiElementsName.append("lineEditSngltn");
    Element *className = nullptr;
    elements.append(reinterpret_cast<BaseElement **>(&className));
    elementsName.append("className");
    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);
    lineEditSngltn->setText(className->getText());
}

bool MainWindow::parseSingleton() {
    if (!makeParseData())
        return false;
    ParsedElements *parsedSingleton = new ParsedElements(SINGLETON, parseData);
    if (!parsedSingleton->isOk())
        return false;
    parsedPattern = parsedSingleton;
    writeParsedSingletonToUi();
    return true;
}

void MainWindow::freezeWidget(QWidget *widget) {
    widget->setEnabled(false);
    freezedWidgets.append(widget);
}

void MainWindow::writeParsedConstructorToTable(QTableWidget *tableClassMethods, ClassMethod<ElementPtr> *classConstructor) {
    QSpinBox *spinBoxNumArgs = getSpinBoxClassMethodArgsNum(tableClassMethods->cellWidget(0, 0));
    const int argsNum = classConstructor->getArgsNum();
    spinBoxNumArgs->setValue(argsNum);
    freezeWidget(spinBoxNumArgs);

    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
        Argument<ElementPtr> *arg = classConstructor->getArgument(argIndex);
        QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(0, 4+argIndex*3));
        bool isConst = arg->constFlag()->getText() == "const";
        checkBoxConst->setChecked(isConst);
        if (!isConst) {
            // same here
            freezeWidget(checkBoxConst);
        }

        const QString methodType = arg->getType()->getText();
        tableClassMethods->item(0, 5+argIndex*3)->setText(methodType);
        const QString methodName = arg->getName()->getText();
        tableClassMethods->item(0, 6+argIndex*3)->setText(methodName);
    }
}

void MainWindow::writeParsedMethodToTable(QTableWidget *tableClassMethods, int methodIndex, ClassMethod<ElementPtr> *classMethod) {
    QSpinBox *spinBoxNumArgs = getSpinBoxClassMethodArgsNum(tableClassMethods->cellWidget(methodIndex, 0));
    const int argsNum = classMethod->getArgsNum();
    spinBoxNumArgs->setValue(argsNum);
    freezeWidget(spinBoxNumArgs);

    QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(methodIndex, 1));
    bool isConst = classMethod->constFlag()->getText() == "const";
    checkBoxConst->setChecked(isConst);
    if (!isConst) {
        // I can't insert text from nothing. For example: (int x1...). If I will make missing const "" have same position as
        // "int", it will cause problems. So it has position one less, but then when "const" is inserted, it will appear to
        // the left of the brackets.
        freezeWidget(checkBoxConst);
    }

    const QString methodType = classMethod->getType()->getText();
    tableClassMethods->item(methodIndex, 2)->setText(methodType);
    const QString methodName = classMethod->getName()->getText();
    tableClassMethods->item(methodIndex, 3)->setText(methodName);

    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
        Argument<ElementPtr> *arg = classMethod->getArgument(argIndex);
        QCheckBox *checkBoxConst = getCheckBoxClassMethodConst(tableClassMethods->cellWidget(methodIndex, 4+argIndex*3));
        bool isConst = arg->constFlag()->getText() == "const";
        checkBoxConst->setChecked(isConst);
        if (!isConst) {
            // same here
            freezeWidget(checkBoxConst);
        }

        const QString methodType = arg->getType()->getText();
        tableClassMethods->item(methodIndex, 5+argIndex*3)->setText(methodType);
        const QString methodName = arg->getName()->getText();
        tableClassMethods->item(methodIndex, 6+argIndex*3)->setText(methodName);
    }
}

void MainWindow::writeParsedAbstractFactoryToUi() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;
    QLineEdit *lineEditFactoryName = nullptr;
    QSpinBox *spinBoxNumFactories = nullptr;
    QSpinBox *spinBoxNumProducts = nullptr;
    QListWidget *listOfFactories = nullptr;
    QListWidget *listOfProducts = nullptr;
    QHBoxLayout *layoutProductsMethodsList = nullptr;
    uiElements << reinterpret_cast<QObject **>(&lineEditFactoryName) << reinterpret_cast<QObject **>(&spinBoxNumFactories)
               << reinterpret_cast<QObject **>(&spinBoxNumProducts) << reinterpret_cast<QObject **>(&listOfFactories)
               << reinterpret_cast<QObject **>(&listOfProducts) << reinterpret_cast<QObject **>(&layoutProductsMethodsList);
    uiElementsName << "lineEditFactoryName" << "spinBoxNumFactories" << "spinBoxNumProducts" << "listOfFactories" << "listOfProducts"
                   << "layoutProductsMethodsList";
    Element *abstractFactoryName = nullptr;
    VectorElement *factoriesNames = nullptr;
    VectorElement *productsNames = nullptr;
    VectorElement *productsMethods = nullptr;
    elements << reinterpret_cast<BaseElement **>(&abstractFactoryName) << reinterpret_cast<BaseElement **>(&factoriesNames)
             << reinterpret_cast<BaseElement **>(&productsNames) << reinterpret_cast<BaseElement **>(&productsMethods);
    elementsName << "abstractFactoryName" << "factoriesNames" << "productsNames" << "productsMethods";
    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);
    freezeWidget(spinBoxNumProducts);
    freezeWidget(spinBoxNumFactories);
    QRadioButton *btnRawPointer = ui->centralwidget->findChild<QRadioButton *>("btnRawPointer");
    freezeWidget(btnRawPointer);
    QRadioButton *btnUniquePointer = ui->centralwidget->findChild<QRadioButton *>("btnUniquePointer");
    freezeWidget(btnUniquePointer);
    QRadioButton *btnSharedPointer = ui->centralwidget->findChild<QRadioButton *>("btnSharedPointer");
    freezeWidget(btnSharedPointer);

    lineEditFactoryName->setText(abstractFactoryName->getText());
    const int factoriesNum = factoriesNames->getCount();
    spinBoxNumFactories->setValue(factoriesNum);
    const int productsNum = productsNames->getCount();
    spinBoxNumProducts->setValue(productsNum);
    for (int i = 0; i < factoriesNum; ++i)
        listOfFactories->item(i)->setText(static_cast<Element *>((*factoriesNames)[i])->getText());
    for (int i = 0; i < productsNum; ++i) {
        listOfProducts->setCurrentRow(i);
        listOfProducts->item(i)->setText(static_cast<Element *>((*productsNames)[i])->getText());
    }

    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ClassMethods *productMethods = static_cast<ClassMethods *>((*productsMethods)[productIndex]);

        QSpinBox *spinBoxNumMethods;
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethodsList->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = productMethods->getCount();
        spinBoxNumMethods->setValue(methodsNum);
        freezeWidget(spinBoxNumMethods);
        for (int methodIndex = 0; methodIndex < methodsNum; ++methodIndex) {
            writeParsedMethodToTable(tableProductMethods, methodIndex, (*productMethods)[methodIndex]);
        }
    }
}

bool MainWindow::parseAbstractFactory() {
    if (!makeParseData())
        return false;
    ParsedElements *parsedAbstractFactory = new ParsedElements(ABSTRACT_FACTORY, parseData);
    if (!parsedAbstractFactory->isOk())
        return false;
    parsedPattern = parsedAbstractFactory;
    writeParsedAbstractFactoryToUi();
    return true;
}

void MainWindow::writeParsedBuilderToUi() {
    QVector<QObject **> uiElements;
    QStringList uiElementsName;
    QVector<BaseElement **> elements;
    QStringList elementsName;

    QLineEdit *lineEditDirectorName = nullptr;
    QSpinBox *spinBoxDirectorMethodsNum = nullptr;
    QLineEdit *lineEditAbstractBuilderName = nullptr;
    QSpinBox *spinBoxAbstractBuilderMethodsNum = nullptr;
    QSpinBox *spinBoxBuildersNum = nullptr;
    QListWidget *listBuildersNames = nullptr;
    QListWidget *listProductsNames = nullptr;
    QTableWidget *directorMethods = nullptr;
    QTableWidget *abstractBuilderMethods = nullptr;
    QHBoxLayout *layoutProductsMethods = nullptr;
    uiElements << reinterpret_cast<QObject **>(&lineEditDirectorName) << reinterpret_cast<QObject **>(&spinBoxDirectorMethodsNum)
               << reinterpret_cast<QObject **>(&lineEditAbstractBuilderName) << reinterpret_cast<QObject **>(&spinBoxAbstractBuilderMethodsNum)
               << reinterpret_cast<QObject **>(&spinBoxBuildersNum) << reinterpret_cast<QObject **>(&listBuildersNames)
               << reinterpret_cast<QObject **>(&listProductsNames) << reinterpret_cast<QObject **>(&directorMethods)
               << reinterpret_cast<QObject **>(&abstractBuilderMethods) << reinterpret_cast<QObject **>(&layoutProductsMethods);
    uiElementsName << "lineEditDirectorName" << "spinBoxDirectorMethodsNum" << "lineEditAbstractBuilderName" << "spinBoxAbstractBuilderMethodsNum"
                   << "spinBoxBuildersNum" << "listBuildersNames" << "listProductsNames" << "directorMethods" << "abstractBuilderMethods"
                   << "layoutProductsMethods";

    Element *directorNameElement = nullptr;
    ClassMethods *directorMethodsElement = nullptr;
    Element *abstractBuilderNameElement = nullptr;
    ClassMethods *abstractBuilderMethodsElement = nullptr;
    VectorElement *buildersNamesElement = nullptr;
    VectorElement *productsNamesElement = nullptr;
    VectorElement *productsMethodsElement = nullptr;
    elements << reinterpret_cast<BaseElement **>(&directorNameElement) << reinterpret_cast<BaseElement **>(&directorMethodsElement)
             << reinterpret_cast<BaseElement **>(&abstractBuilderNameElement) << reinterpret_cast<BaseElement **>(&abstractBuilderMethodsElement)
             << reinterpret_cast<BaseElement **>(&buildersNamesElement) << reinterpret_cast<BaseElement **>(&productsNamesElement)
             << reinterpret_cast<BaseElement **>(&productsMethodsElement);
    elementsName << "directorName" << "directorMethods" << "abstractBuilderName" << "abstractBuilderMethods"
                 << "buildersNames" << "productsNames" << "productsMethods";

    initParsedPatternAndUi(uiElements, uiElementsName, elements, elementsName);

    freezeWidget(spinBoxDirectorMethodsNum);
    freezeWidget(spinBoxAbstractBuilderMethodsNum);
    freezeWidget(spinBoxBuildersNum);

    const int directorMethodsNum = directorMethodsElement->getCount();
    const int abstractBuilderMethodsNum = abstractBuilderMethodsElement->getCount();
    const int buildersNum = buildersNamesElement->getCount();
    const int productsNum = productsNamesElement->getCount(); // == buildersNum
    spinBoxDirectorMethodsNum->setValue(directorMethodsNum);
    spinBoxAbstractBuilderMethodsNum->setValue(abstractBuilderMethodsNum);
    spinBoxBuildersNum->setValue(buildersNum);
    lineEditDirectorName->setText(directorNameElement->getText());
    lineEditAbstractBuilderName->setText(abstractBuilderNameElement->getText());

    for (int i = 0; i < buildersNum; ++i) {
        BaseElement *builderNameBaseElement = (*buildersNamesElement)[i];
        Element *builderNameElement = static_cast<Element *>(builderNameBaseElement);
        if (!builderNameElement)
            qCritical() << "builderNameElement is not an Element in buildersNamesElement";
        BaseElement *productNameBaseElement = (*productsNamesElement)[i];
        Element *productNameElement = static_cast<Element *>(productNameBaseElement);
        if (!productNameElement)
            qCritical() << "productNameElement is not an Element in productsNamesElement";

        listBuildersNames->item(i)->setText(builderNameElement->getText());
        listProductsNames->setCurrentRow(i);
        listProductsNames->item(i)->setText(productNameElement->getText());
    }

    for (int i = 0; i < directorMethodsNum; ++i) {
        writeParsedMethodToTable(directorMethods, i, (*directorMethodsElement)[i]);
    }

    for (int i = 0; i < abstractBuilderMethodsNum; ++i) {
        writeParsedMethodToTable(abstractBuilderMethods, i, (*abstractBuilderMethodsElement)[i]);
    }

    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ClassMethods *productMethods = static_cast<ClassMethods *>((*productsMethodsElement)[productIndex]);

        QSpinBox *spinBoxNumMethods;
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethods->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = productMethods->getCount() - 1; // don't count constructor
        spinBoxNumMethods->setValue(methodsNum);
        freezeWidget(spinBoxNumMethods);
        writeParsedConstructorToTable(tableProductMethods, (*productMethods)[0]);
        for (int methodIndex = 1; methodIndex <= methodsNum; ++methodIndex) {
            writeParsedMethodToTable(tableProductMethods, methodIndex, (*productMethods)[methodIndex]);
        }
    }
}

bool MainWindow::parseBuilder() {
    if (!makeParseData())
        return false;
    ParsedElements *parsedBuilder = new ParsedElements(BUILDER, parseData);
    if (!parsedBuilder->isOk())
        return false;
    parsedPattern = parsedBuilder;
    writeParsedBuilderToUi();
    return true;
}

void MainWindow::unfreezeUi() {
    const int objectsNum = freezedWidgets.count();
    for (int objectIndex = 0; objectIndex < objectsNum; ++objectIndex) {
        freezedWidgets[objectIndex]->setEnabled(true);
    }
    freezedWidgets.clear();
}

void MainWindow::on_checkBoxImport_toggled(bool checked)
{
    ui->pushBtnImport->setEnabled(checked);
    ui->pushBtnGenerate->setEnabled(not checked);
    if (!checked and parsedPattern)
        unfreezeUi();
}

void MainWindow::importAccepted(const QHash<QString, QStringList> &importData) {
    /*
     * 1) I get QHash<QString, QStringList> importData
     * 2) From it I make QHash<QString, QVector<ClassText *>> parseData
     * 3) From it I make ParsedElements *parsedPattern
     * 4) Change UI using data from parsedPattern
     * 5) If needed I disable some buttons in UI
     * 6) When the generate button is pressed, the data is overwritten from UI to parsedPattern
     *      and then from parsedPattern to files
     *
     * (I don't change filenames)
     *
    */

    this->importData = importData;
    const QString patternTypeName = ui->cmbBoxPatternName->currentText();
    const int patternType = patternTypesList->indexOf(patternTypeName);
    bool success = false;

    switch (patternType) {
        case NO_PATTERN:
            break;
        case SINGLETON:
            success = parseSingleton();
            break;
        case ABSTRACT_FACTORY:
            success = parseAbstractFactory();
            break;
        case BUILDER:
            success = parseBuilder();
            break;
        default:
            qWarning() << "Unexpected pattern type";
            break;
    }

    if (success)
        ui->pushBtnGenerate->setEnabled(true);
}

void MainWindow::on_pushBtnImport_clicked()
{
    const QString patternTypeName = ui->cmbBoxPatternName->currentText();
    const int patternType = patternTypesList->indexOf(patternTypeName);
    ImportWindow *importWindow = new ImportWindow(this, patternType);
    importWindow->exec();
}

MainWindow::~MainWindow()
{
    clearParseData(&parseData);
    delete parsedPattern;
    delete ui;
    delete codeGenerator;
    delete settings;
}
