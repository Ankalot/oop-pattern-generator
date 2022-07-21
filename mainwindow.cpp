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
#include "productmethods.h"

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
    ui->gridLayoutSpecial->setRowStretch(0, 1);

    patternTypesList = new QStringList;
    *patternTypesList << "Select pattern" << "Singleton" << "Abstract factory";

    connect(ui->cmbBoxPatternName, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboBox_indexChanged()));

    settings = new QSettings("OOP-P-G", "Main settings");
    readSettings();
}

QSpinBox *getSpinBoxProductMethodArgsNum(QWidget *spinBoxNumArgsContent) {
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

int getProductMethodArgsNum(QWidget *spinBoxNumArgsContent) {
    QSpinBox *spinBoxNumArgs = getSpinBoxProductMethodArgsNum(spinBoxNumArgsContent);
    return spinBoxNumArgs->value();
}

QCheckBox *getCheckBoxProductMethodConst(QWidget *checkBoxConstContent) {
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
    QCheckBox *checkBoxConst = getCheckBoxProductMethodConst(checkBoxConstContent);
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
    if (!listOfFactories)
        qCritical() << "listOfFactories not found";
    QVector<QString> products(listOfProducts->count());
    const int productsNum = listOfProducts->count();
    for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
        products[productItemIndex] = listOfProducts->item(productItemIndex)->text();
    }

    QScrollArea *listOfProductsMethods = ui->centralwidget->findChild<QScrollArea *>("listOfProductsMethods");
    if (!listOfProductsMethods)
        qCritical() << "listOfProductsMethods not found";
    QWidget *listOfProductsMethodsWidget = listOfProductsMethods->widget();
    if (!listOfProductsMethodsWidget)
        qCritical() << "listOfProductsMethods widget not found";
    QHBoxLayout *layoutProductsMethodsList = qobject_cast<QHBoxLayout *>(listOfProductsMethodsWidget->layout());
    if (!layoutProductsMethodsList)
        qCritical() << "layoutProductsMethodsList not found";
    QVector<QVector<ClassMethod<QString> *>> productsMethods(productsNum);

    for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
        QWidget *productMethodsContent = layoutProductsMethodsList->itemAt(productItemIndex)->widget();
        QVBoxLayout *productMethods = qobject_cast<QVBoxLayout *>(productMethodsContent->layout());
        if (!productMethods)
            qCritical() << "productMethods not found";
        QLayoutItem *tableProductMethodsItem = productMethods->itemAt(1); //0 - labels and buttons widget, 1 - table
        if (!tableProductMethodsItem)
            qCritical() << "tableProductMethods item not found";
        QTableWidget *tableProductMethods = qobject_cast<QTableWidget *>(tableProductMethodsItem->widget());
        if (!tableProductMethods)
            qCritical() << "tableProductMethods not found";
        const int productMethodsNum = tableProductMethods->rowCount();
        assert(productsMethods.count() > productItemIndex);
        productsMethods[productItemIndex].resize(productMethodsNum);

        for (int productMethodIndex = 0; productMethodIndex < productMethodsNum; ++productMethodIndex) {
            const int argsNum = getProductMethodArgsNum(tableProductMethods->cellWidget(productMethodIndex, 0));
            const bool isConst = checkCheckBoxConst(tableProductMethods->cellWidget(productMethodIndex, 1));
            QTableWidgetItem *typeItem = tableProductMethods->item(productMethodIndex, 2);
            if (!typeItem)
                qCritical() << "type item not found";
            const QString type = typeItem->text();
            QTableWidgetItem *nameItem = tableProductMethods->item(productMethodIndex, 3);
            if (!nameItem)
                qCritical() << "name item not found";
            const QString name = nameItem->text();
            ClassMethod<QString> *productMethod = new ClassMethod<QString>(isConst, type, name, argsNum);

            for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
                QTableWidgetItem *typeItem = tableProductMethods->item(productMethodIndex, 5+argIndex*3);
                if (!typeItem)
                    qCritical() << "type item not found";
                QTableWidgetItem *nameItem = tableProductMethods->item(productMethodIndex, 6+argIndex*3);
                if (!nameItem)
                    qCritical() << "name item not found";
                Argument<QString> *arg = new Argument<QString>(checkCheckBoxConst(tableProductMethods->cellWidget(productMethodIndex, 4+argIndex*3)),
                                             typeItem->text(), nameItem->text());
                productMethod->setArgument(arg, argIndex);
            }

            assert(productsMethods[productItemIndex].count() > productMethodIndex);
            productsMethods[productItemIndex][productMethodIndex] = productMethod;
        }
    }

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

void MainWindow::initParsedSingletonAndUi(QLineEdit **lineEditSnglt, Element **className) {
    QLayoutItem *lineEditSngltItem = ui->gridLayoutSpecial->itemAtPosition(0, 1);
    if (!lineEditSngltItem)
        qCritical() << "lineEditSnglt item not found";
    *lineEditSnglt = qobject_cast<QLineEdit *>(lineEditSngltItem->widget());
    if (!*lineEditSnglt)
        qCritical() << "lineEditSnglt item not found";
    BaseElement *classNameBase = parsedPattern->getElements().value("className");
    if (!classNameBase)
        qCritical() << "className key not found in parsedPattern";
    *className = static_cast<Element *>(classNameBase);
    if (!*className)
        qCritical() << "className element not found in parsedPattern";
}

void MainWindow::writeUiToParsedSingleton() {
    QLineEdit *lineEditSnglt = nullptr;
    Element *className = nullptr;
    initParsedSingletonAndUi(&lineEditSnglt, &className);
    className->setText(lineEditSnglt->text());
}

void MainWindow::initParsedAbstractFactoryAndUi(QSpinBox **spinBoxNumFactories, QSpinBox **spinBoxNumProducts,
                                                QLineEdit **lineEditFactoryName, QListWidget **listOfFactories,
                                                QListWidget **listOfProducts, QHBoxLayout **layoutProductsMethodsList,
                                                Element **abstractFactoryName, VectorElement **factoriesNames,
                                                VectorElement **productsNames, VectorElement **productsMethods) {
    *spinBoxNumFactories = ui->centralwidget->findChild<QSpinBox *>("spinBoxNumFactories");
    if (!spinBoxNumFactories)
        qCritical() << "spinBoxNumFactories not found";
    *spinBoxNumProducts = ui->centralwidget->findChild<QSpinBox *>("spinBoxNumProducts");
    if (!spinBoxNumProducts)
        qCritical() << "spinBoxNumProducts not found";
    *lineEditFactoryName = ui->centralwidget->findChild<QLineEdit *>("lineEditFactoryName");
    if (!*lineEditFactoryName)
        qCritical() << "lineEditFactoryName not found";
    *listOfFactories = ui->centralwidget->findChild<QListWidget *>("listOfFactories");
    if (!*listOfFactories)
        qCritical() << "listOfFactories not found";
    *listOfProducts = ui->centralwidget->findChild<QListWidget *>("listOfProducts");
    if (!*listOfProducts)
        qCritical() << "listOfProducts not found";
    QScrollArea *listOfProductsMethods = ui->centralwidget->findChild<QScrollArea *>("listOfProductsMethods");
    if (!listOfProductsMethods)
        qCritical() << "listOfProductsMethods not found";
    QWidget *listOfProductsMethodsWidget = listOfProductsMethods->widget();
    if (!listOfProductsMethodsWidget)
        qCritical() << "listOfProductsMethods widget not found";
    *layoutProductsMethodsList = qobject_cast<QHBoxLayout *>(listOfProductsMethodsWidget->layout());
    if (!*layoutProductsMethodsList)
        qCritical() << "layoutProductsMethodsList not found";

    BaseElement *abstractFactoryNameBase = parsedPattern->getElements().value("abstractFactoryName");
    if (!abstractFactoryNameBase)
        qCritical() << "abstractFactoryName key not found in parsedPattern";
    *abstractFactoryName = static_cast<Element *>(abstractFactoryNameBase);
    if (!*abstractFactoryName)
        qCritical() << "abstractFactoryName element not found in parsedPattern";
    BaseElement *factoriesNamesBase = parsedPattern->getElements().value("factoriesNames");
    if (!factoriesNamesBase)
        qCritical() << "factoriesNames key not found in parsedPattern";
    *factoriesNames = static_cast<VectorElement *>(factoriesNamesBase);
    if (!*factoriesNames)
        qCritical() << "factoriesNames VectorElement not found in parsedPattern";
    BaseElement *productsNamesBase = parsedPattern->getElements().value("productsNames");
    if (!productsNamesBase)
        qCritical() << "productsNames key not found in parsedPattern";
    *productsNames = static_cast<VectorElement *>(productsNamesBase);
    if (!*productsNames)
        qCritical() << "productsNames VectorElement not found in parsedPattern";
    BaseElement *productsMethodsBase = parsedPattern->getElements().value("productsMethods");
    if (!productsMethodsBase)
        qCritical() << "productsMethods key not found in parsedPattern";
    *productsMethods = static_cast<VectorElement *>(productsMethodsBase);
    if (!*productsMethods)
        qCritical() << "productsMethods VectorElement not found in parsedPattern";
}

void findWidgetsInProductMethodsContentItem(QLayoutItem *productMethodsContentItem, QSpinBox **spinBoxNumMethods,
                                            QTableWidget **tableProductMethods) {
    if (!productMethodsContentItem)
        qCritical() << "productMethodsContent item not found";
    QWidget *productMethodsContent = productMethodsContentItem->widget();
    QVBoxLayout *productMethodsLayout = qobject_cast<QVBoxLayout *>(productMethodsContent->layout());
    if (!productMethodsLayout)
        qCritical() << "productMethods not found";
    QLayoutItem *labelAndBoxesContentItem = productMethodsLayout->itemAt(0); //0 - labels and buttons widget, 1 - table
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
    QLayoutItem *tableProductMethodsItem = productMethodsLayout->itemAt(1); //0 - labels and buttons widget, 1 - table
    if (!tableProductMethodsItem) {
        qCritical() << "tableProductMethods item not found";
    }
    *tableProductMethods = qobject_cast<QTableWidget *>(tableProductMethodsItem->widget());
    if (!*tableProductMethods)
        qCritical() << "tableProductMethods not found";
}

void MainWindow::writeUiToParsedAbstractFactory() {
    QLineEdit *lineEditFactoryName = nullptr;
    QSpinBox *spinBoxNumFactories = nullptr, *spinBoxNumProducts = nullptr;
    QListWidget *listOfFactories = nullptr, *listOfProducts = nullptr;
    QHBoxLayout *layoutProductsMethodsList = nullptr;
    Element *abstractFactoryName = nullptr;
    VectorElement *factoriesNames = nullptr, *productsNames = nullptr, *productsMethods = nullptr;
    initParsedAbstractFactoryAndUi(&spinBoxNumFactories, &spinBoxNumProducts,
                                   &lineEditFactoryName, &listOfFactories, &listOfProducts, &layoutProductsMethodsList,
                                   &abstractFactoryName, &factoriesNames, &productsNames, &productsMethods);
    abstractFactoryName->setText(lineEditFactoryName->text());
    const int factoriesNum = listOfFactories->count();
    const int productsNum = listOfProducts->count();
    for (int i = 0; i < factoriesNum; ++i)
        static_cast<Element *>((*factoriesNames)[i])->setText(listOfFactories->item(i)->text());
    for (int i = 0; i < productsNum; ++i)
        static_cast<Element *>((*productsNames)[i])->setText(listOfProducts->item(i)->text());

    for (int productIndex = 0; productIndex < productsNum; ++productIndex) {
        ProductMethods *productMethods = static_cast<ProductMethods *>((*productsMethods)[productIndex]);

        QSpinBox *spinBoxNumMethods; //don't need this actually
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethodsList->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = productMethods->getCount();
        for (int methodIndex = 0; methodIndex < methodsNum; ++methodIndex) {
            QCheckBox *checkBoxConst = getCheckBoxProductMethodConst(tableProductMethods->cellWidget(methodIndex, 1));
            const QString isConst = checkBoxConst->isChecked() ? "const" : "";
            (*productMethods)[methodIndex]->constFlag()->setText(isConst);

            const QString methodType = tableProductMethods->item(methodIndex, 2)->text();
            (*productMethods)[methodIndex]->getType()->setText(methodType);
            const QString methodName = tableProductMethods->item(methodIndex, 3)->text();
            (*productMethods)[methodIndex]->getName()->setText(methodName);

            const int argsNum = (*productMethods)[methodIndex]->getArgsNum();
            for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
                Argument<Element *> *arg = (*productMethods)[methodIndex]->getArgument(argIndex);
                QCheckBox *checkBoxConst = getCheckBoxProductMethodConst(tableProductMethods->cellWidget(methodIndex, 4+argIndex*3));

                const QString isConst = checkBoxConst->isChecked() ? "const" : "";
                arg->constFlag()->setText(isConst);

                const QString methodType = tableProductMethods->item(methodIndex, 5+argIndex*3)->text();
                arg->getType()->setText(methodType);
                const QString methodName = tableProductMethods->item(methodIndex, 6+argIndex*3)->text();
                arg->getName()->setText(methodName);
            }
        }
    }
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
        } default:
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

void MainWindow::spinBoxNumFactoriesChanged(const int nextMethodsNum) {
    QListWidget *listOfFactories = ui->centralwidget->findChild<QListWidget *>("listOfFactories");
    if (!listOfFactories)
        qCritical() << "listOfFactories not found";
    const int currFactoriesNum = listOfFactories->count();

    if (nextMethodsNum > currFactoriesNum) {
        for (int i = currFactoriesNum; i < nextMethodsNum; ++i) {
            addItemToListWidget(listOfFactories, QString("Factory") + QString::number(i + 1));
        }
    } else {
        for (int i = currFactoriesNum; i > nextMethodsNum; --i) {
            delete listOfFactories->item(i-1);
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
    const int currProductsNum = listOfProducts->count();

    if (nextProductsNum > currProductsNum) {
        for (int i = currProductsNum; i < nextProductsNum; ++i) {
            addItemToListWidget(listOfProducts, QString("Product") + QString::number(i + 1));
            addItemToLayoutProductsMethodsList(layoutProductsMethodsList, QString("Product") + QString::number(i + 1));
        }
    } else {
        for (int i = currProductsNum; i > nextProductsNum; --i) {
            delete listOfProducts->item(i-1);
            delItemFromLayoutProductsMethodsList(layoutProductsMethodsList, i-1);
        }
    }
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
        const int argsNum = getProductMethodArgsNum(tableProductMethods->cellWidget(i, 0));
        if (argsNum > maxArgsNum)
            maxArgsNum = argsNum;
    }
    for (int i = currMethodIndex+1; i < methodsNum; ++i) {
        const int argsNum = getProductMethodArgsNum(tableProductMethods->cellWidget(i, 0));
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

void MainWindow::changeMethodsCountInTable(const int nextMethodsNum) {
    QWidget *productMethodsContent = qobject_cast<QWidget *>(QObject::sender()->parent()->parent());
    if (!productMethodsContent)
        qCritical() << "productMethodsContent not found";
    QVBoxLayout *productMethods = qobject_cast<QVBoxLayout *>(productMethodsContent->layout());
    if (!productMethods)
        qCritical() << "productMethods not found";
    QLayoutItem *tableProductMethodsItem = productMethods->itemAt(1); //0 - labels and buttons widget, 1 - table
    if (!tableProductMethodsItem)
        qCritical() << "tableProductMethods item not found";
    QTableWidget *tableProductMethods = qobject_cast<QTableWidget *>(tableProductMethodsItem->widget());
    if (!tableProductMethods)
        qCritical() << "tableProductMethods not found";
    const int currMethodsNum = tableProductMethods->rowCount();
    const int currArgsNum = int((tableProductMethods->columnCount()-4)/3);

    tableProductMethods->setRowCount(nextMethodsNum);
    if (nextMethodsNum > currMethodsNum) {
        for (int i = currMethodsNum; i < nextMethodsNum; ++i) {
            addSpinBoxNumArgsToCell(tableProductMethods, i, 0);
            addCheckBoxConstToCell(tableProductMethods, i, 1);
            tableProductMethods->setItem(i, 2, new QTableWidgetItem(""));
            tableProductMethods->setItem(i, 3, new QTableWidgetItem(""));
            for (int j = 0; j < currArgsNum; ++j) {
                addCheckBoxConstToCell(tableProductMethods, i, 4+j*3);
                tableProductMethods->setItem(i, 5+j*3, new QTableWidgetItem(""));
                tableProductMethods->setItem(i, 6+j*3, new QTableWidgetItem(""));
            }
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
    widget->setFixedWidth(widget->minimumWidth()-TableOfProductMethodsWidth);
}

void MainWindow::addItemToLayoutProductsMethodsList(QHBoxLayout *layoutProductsMethodsList, const QString &productName) {
    QWidget *productMethodsContent = new QWidget;
    QVBoxLayout *productMethods = new QVBoxLayout;
    QWidget *labelAndBoxesContent = new QWidget;
    QHBoxLayout *labelAndBoxes = new QHBoxLayout;
    QLabel *labelProductMethods = new QLabel(productName + " methods:");
    QLabel *lblNumMethods = new QLabel("Num methods:");
    QSpinBox *spinBoxNumMethods = new QSpinBox;
    QTableWidget *tableProductMethods = new QTableWidget;

    productMethodsContent->setLayout(productMethods);
    productMethods->setMargin(0);
    productMethods->addWidget(labelAndBoxesContent);
    productMethods->addWidget(tableProductMethods);
    labelAndBoxesContent->setLayout(labelAndBoxes);
    labelAndBoxes->setMargin(0);
    labelAndBoxes->addWidget(labelProductMethods);
    labelAndBoxes->addWidget(lblNumMethods);
    labelAndBoxes->addWidget(spinBoxNumMethods);
    labelProductMethods->setFont(QFont("MS Shell Dlg 2", 12));
    labelProductMethods->setFixedHeight(30);
    lblNumMethods->setFont(QFont("MS Shell Dlg 2", 12));
    lblNumMethods->setFixedSize(130, 30);
    spinBoxNumMethods->setFont(QFont("MS Shell Dlg 2", 12));
    spinBoxNumMethods->setFixedSize(60, 30);
    spinBoxNumMethods->setRange(0, 99);

    tableProductMethods->setFont(QFont("MS Shell Dlg 2", 12));
    tableProductMethods->setEditTriggers(QAbstractItemView::AllEditTriggers);
    tableProductMethods->setRowCount(0);
    tableProductMethods->setColumnCount(4);
    tableProductMethods->setHorizontalHeaderItem(0, new QTableWidgetItem("Num args"));
    tableProductMethods->setColumnWidth(0, 100);
    tableProductMethods->setHorizontalHeaderItem(1, new QTableWidgetItem("Const"));
    tableProductMethods->setColumnWidth(1, 80);
    tableProductMethods->setHorizontalHeaderItem(2, new QTableWidgetItem("Output type"));
    tableProductMethods->setHorizontalHeaderItem(3, new QTableWidgetItem("Name"));

    QWidget *widget = layoutProductsMethodsList->parentWidget();
    widget->setFixedWidth(widget->minimumWidth()+TableOfProductMethodsWidth);

    layoutProductsMethodsList->addWidget(productMethodsContent);

    connect(spinBoxNumMethods, SIGNAL(valueChanged(int)), this, SLOT(changeMethodsCountInTable(int)));
}

void MainWindow::changeProductNameInTable(QListWidgetItem *productNameItem) {
    QListWidget *listOfProducts = ui->centralwidget->findChild<QListWidget *>("listOfProducts");
    if (!listOfProducts)
        qCritical() << "listOfProducts not found";
    const int index = listOfProducts->currentRow();
    QScrollArea *listOfProductsMethods = ui->centralwidget->findChild<QScrollArea *>("listOfProductsMethods");
    if (!listOfProductsMethods)
        qCritical() << "listOfProductsMethods not found";
    QWidget *contentOfListOfProductsMethods = listOfProductsMethods->widget();
    QHBoxLayout *layoutProductsMethodsList = qobject_cast<QHBoxLayout *>(contentOfListOfProductsMethods->layout());
    if (!layoutProductsMethodsList)
        qCritical() << "layoutProductsMethodsList not found";
    QLayoutItem *productMethodsContentItem = layoutProductsMethodsList->itemAt(index);
    if (!productMethodsContentItem)
        qCritical() << "productMethodsContent item not found";
    QWidget *productMethodsContent = productMethodsContentItem->widget();
    QVBoxLayout *productMethods = qobject_cast<QVBoxLayout *>(productMethodsContent->layout());
    if (!productMethods)
        qCritical() << "productMethods not found";
    QLayoutItem *labelAndButtonsContentItem = productMethods->itemAt(0); //0 - labels and buttons widget, 1 - table
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
    labelProductMethods->setText(productNameItem->text() + " methods:");
}

void clearParseData(QHash<QString, QVector<ClassText *>> *parseData) {
    foreach (QVector<ClassText *> classTexts, *parseData) {
        qDeleteAll(classTexts);
    }
    parseData->clear();
}

void MainWindow::comboBox_indexChanged() {
    ui->pushBtnGenerate->setEnabled(not ui->checkBoxImport->isChecked());
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
            labelSngltn->setFont(QFont("MS Shell Dlg 2", 14));
            lineEditSngltn->setMinimumSize(200, 40);
            lineEditSngltn->setFont(QFont("MS Shell Dlg 2", 14));
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
            spinBoxNumFactories->setFont(QFont("MS Shell Dlg 2", 14));
            spinBoxNumFactories->setMinimum(1);
            spinBoxNumFactories->setMaximum(99);
            spinBoxNumFactories->setObjectName("spinBoxNumFactories");
            spinBoxNumProducts->setFixedSize(80, 40);
            spinBoxNumProducts->setFont(QFont("MS Shell Dlg 2", 14));
            spinBoxNumProducts->setMinimum(1);
            spinBoxNumProducts->setMaximum(99);
            spinBoxNumProducts->setObjectName("spinBoxNumProducts");
            labelNumFactories->setMinimumSize(260, 40);
            labelNumFactories->setFont(QFont("MS Shell Dlg 2", 14));
            labelNumProducts->setMinimumSize(260, 40);
            labelNumProducts->setFont(QFont("MS Shell Dlg 2", 14));

            ptrTypeAndFactoryNameLayout->setContentsMargins(20, 0, 0, 0);
            lblFactoryName->setFont(QFont("MS Shell Dlg 2", 14));
            lineEditFactoryName->setFont(QFont("MS Shell Dlg 2", 14));
            lineEditFactoryName->setObjectName("lineEditFactoryName");
            btnRawPointer->setFont(QFont("MS Shell Dlg 2", 14));
            btnRawPointer->setChecked(true);
            btnRawPointer->setObjectName("btnRawPointer");
            btnUniquePointer->setFont(QFont("MS Shell Dlg 2", 14));
            btnUniquePointer->setObjectName("btnUniquePointer");
            btnSharedPointer->setFont(QFont("MS Shell Dlg 2", 14));
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

            labelFunctions->setFont(QFont("MS Shell Dlg 2", 14));
            labelFunctions->setAlignment(Qt::AlignCenter);
            contentOfListOfProductsMethods->setLayout(layoutProductsMethodsList);
            contentOfListOfProductsMethods->setFixedSize(16, TableOfProductMethodsHeight);
            listOfProductsMethods->setMinimumHeight(200);
            listOfProductsMethods->setWidget(contentOfListOfProductsMethods);
            listOfProductsMethods->setObjectName("listOfProductsMethods");
            addItemToLayoutProductsMethodsList(layoutProductsMethodsList, "Product1");

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
            connect(listOfProducts, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(changeProductNameInTable(QListWidgetItem*)));
            break;
        } default:
            qWarning() << "Unexpected pattern type";
            break;
    }
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
    QLineEdit *lineEditSnglt = nullptr;
    Element *className = nullptr;
    initParsedSingletonAndUi(&lineEditSnglt, &className);
    lineEditSnglt->setText(className->getText());
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

void MainWindow::writeParsedAbstractFactoryToUi() {
    QLineEdit *lineEditFactoryName = nullptr;
    QSpinBox *spinBoxNumFactories = nullptr, *spinBoxNumProducts = nullptr;
    QListWidget *listOfFactories = nullptr, *listOfProducts = nullptr;
    QHBoxLayout *layoutProductsMethodsList = nullptr;
    Element *abstractFactoryName = nullptr;
    VectorElement *factoriesNames = nullptr, *productsNames = nullptr, *productsMethods = nullptr;
    initParsedAbstractFactoryAndUi(&spinBoxNumFactories, &spinBoxNumProducts,
                                   &lineEditFactoryName, &listOfFactories, &listOfProducts, &layoutProductsMethodsList,
                                   &abstractFactoryName, &factoriesNames, &productsNames, &productsMethods);
    spinBoxNumProducts->setEnabled(false);
    freezedWidgets.append(spinBoxNumProducts);
    spinBoxNumFactories->setEnabled(false);
    freezedWidgets.append(spinBoxNumFactories);
    QRadioButton *btnRawPointer = ui->centralwidget->findChild<QRadioButton *>("btnRawPointer");
    btnRawPointer->setEnabled(false);
    freezedWidgets.append(btnRawPointer);
    QRadioButton *btnUniquePointer = ui->centralwidget->findChild<QRadioButton *>("btnUniquePointer");
    btnUniquePointer->setEnabled(false);
    freezedWidgets.append(btnUniquePointer);
    QRadioButton *btnSharedPointer = ui->centralwidget->findChild<QRadioButton *>("btnSharedPointer");
    btnSharedPointer->setEnabled(false);
    freezedWidgets.append(btnSharedPointer);

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
        ProductMethods *productMethods = static_cast<ProductMethods *>((*productsMethods)[productIndex]);

        QSpinBox *spinBoxNumMethods;
        QTableWidget *tableProductMethods;
        findWidgetsInProductMethodsContentItem(layoutProductsMethodsList->itemAt(productIndex), &spinBoxNumMethods,
                                               &tableProductMethods);

        const int methodsNum = productMethods->getCount();
        spinBoxNumMethods->setValue(methodsNum);
        spinBoxNumMethods->setEnabled(false);
        freezedWidgets.append(spinBoxNumMethods);
        for (int methodIndex = 0; methodIndex < methodsNum; ++methodIndex) {
            QSpinBox *spinBoxNumArgs = getSpinBoxProductMethodArgsNum(tableProductMethods->cellWidget(methodIndex, 0));
            const int argsNum = (*productMethods)[methodIndex]->getArgsNum();
            spinBoxNumArgs->setValue(argsNum);
            spinBoxNumArgs->setEnabled(false);
            freezedWidgets.append(spinBoxNumArgs);

            QCheckBox *checkBoxConst = getCheckBoxProductMethodConst(tableProductMethods->cellWidget(methodIndex, 1));
            bool isConst = (*productMethods)[methodIndex]->constFlag()->getText() == "const";
            checkBoxConst->setChecked(isConst);
            if (!isConst) {
                // I can't insert text from nothing. For example: (int x1...). If I will make missing const "" have same position as
                // "int", it will cause problems. So it has position one less, but then when "const" is inserted, it will appear to
                // the left of the brackets.
                checkBoxConst->setEnabled(false);
                freezedWidgets.append(checkBoxConst);
            }

            const QString methodType = (*productMethods)[methodIndex]->getType()->getText();
            tableProductMethods->item(methodIndex, 2)->setText(methodType);
            const QString methodName = (*productMethods)[methodIndex]->getName()->getText();
            tableProductMethods->item(methodIndex, 3)->setText(methodName);

            for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
                Argument<Element *> *arg = (*productMethods)[methodIndex]->getArgument(argIndex);
                QCheckBox *checkBoxConst = getCheckBoxProductMethodConst(tableProductMethods->cellWidget(methodIndex, 4+argIndex*3));
                bool isConst = arg->constFlag()->getText() == "const";
                checkBoxConst->setChecked(isConst);
                if (!isConst) {
                    // same here
                    checkBoxConst->setEnabled(false);
                    freezedWidgets.append(checkBoxConst);
                }

                const QString methodType = arg->getType()->getText();
                tableProductMethods->item(methodIndex, 5+argIndex*3)->setText(methodType);
                const QString methodName = arg->getName()->getText();
                tableProductMethods->item(methodIndex, 6+argIndex*3)->setText(methodName);
            }
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

void MainWindow::unfreezeUi() {
    const int objectsNum = freezedWidgets.count();
    for (int objectIndex = 0; objectIndex < objectsNum; ++objectIndex) {
        freezedWidgets[objectIndex]->setEnabled(true);
    }
    freezedWidgets.clear();
}

void MainWindow::on_checkBoxImport_clicked(bool checked)
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
