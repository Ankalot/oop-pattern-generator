#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codegenerator.h"
#include "classmethod.h"
#include "argument.h"

#include <cassert>
#include <QCheckBox>
#include <QClipboard>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), codeGenerator(new CodeGenerator)
{
    ui->setupUi(this);
    ui->comboBox->addItem("Select pattern");
    ui->comboBox->addItem("Singleton");
    ui->comboBox->addItem("Abstract factory");
    ui->gridLayoutSpecial->setRowStretch(0, 1);

    patternTypesList = new QStringList;
    *patternTypesList << "Select pattern" << "Singleton" << "Abstract factory";

    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboBox_indexChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete codeGenerator;
}

int getProductMethodArgsNum(QWidget *spinBoxNumArgsContent) {
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
    if (!spinBoxNumArgsLayout)
        qCritical() << "spinBoxNumArgs not found";
    return spinBoxNumArgs->value();
}

bool checkCheckBoxConst(QWidget *checkBoxConstContent) {
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
    return checkBoxConst->isChecked();
}

void MainWindow::on_pushButton_clicked()
{
    enum { NO_PATTERN = 0, SINGLETON = 1, ABSTRACT_FACTORY = 2 };

    const QString patternType = ui->comboBox->currentText();
    const int patternTypeIndex = patternTypesList->indexOf(patternType);
    QString text = "";

    switch (patternTypeIndex) {
        case NO_PATTERN:
            break;
        case SINGLETON: {
            QLineEdit *lineEditSngltn = ui->centralwidget->findChild<QLineEdit *>("lineEditSngltn");
            if (!lineEditSngltn)
                qCritical() << "lineEditSngltn not found";
            const QString className = lineEditSngltn->text();

            text = codeGenerator->genSingleton(className);
            break;
        } case ABSTRACT_FACTORY: {
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
            QVector<QVector<ClassMethod *>> productsMethods(productsNum);

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
                    ClassMethod *productMethod = new ClassMethod(isConst, type, name, argsNum);

                    for (int argIndex = 0; argIndex < argsNum; ++argIndex) {
                        QTableWidgetItem *typeItem = tableProductMethods->item(productMethodIndex, 5+argIndex*3);
                        if (!typeItem)
                            qCritical() << "type item not found";
                        QTableWidgetItem *nameItem = tableProductMethods->item(productMethodIndex, 6+argIndex*3);
                        if (!nameItem)
                            qCritical() << "name item not found";
                        Argument *arg = new Argument(checkCheckBoxConst(tableProductMethods->cellWidget(productMethodIndex, 4+argIndex*3)),
                                                     typeItem->text(), nameItem->text());
                        productMethod->addArgument(arg, argIndex);
                    }

                    assert(productsMethods[productItemIndex].count() > productMethodIndex);
                    productsMethods[productItemIndex][productMethodIndex] = productMethod;
                }
            }

            text = codeGenerator->genAbstractFactory(pointerType, factories, products, productsMethods);

            for (int productItemIndex = 0; productItemIndex < productsNum; ++productItemIndex) {
                qDeleteAll(productsMethods[productItemIndex]);
                //productsMethodsList[productItemIndex].clear();
            }
            //productsMethodsList.clear();
            break;
        } default:
            qWarning() << "Unexpected pattern type";
            break;
    }

    QApplication::clipboard()->setText(text);
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
    const int currArgsNum = int((tableProductMethods->columnCount()-4)/3);
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
        const int currMethodIndex = QObject::sender()->objectName().toInt();
        const int minPossibleArgsNum = qMax(findMaxArgsNumInTable(tableProductMethods, currMethodIndex), nextArgsNum);
        tableProductMethods->setColumnCount(4+minPossibleArgsNum*3);
    }
}

void MainWindow::addSpinBoxNumArgsToCell(QTableWidget *table, const int rowIndex, const int columnIndex) {
    QWidget *spinBoxNumArgsContent = new QWidget;
    QHBoxLayout *spinBoxNumArgsLayout = new QHBoxLayout;
    QSpinBox *spinBoxNumArgs = new QSpinBox;

    spinBoxNumArgs->setFixedSize(60, 30);
    spinBoxNumArgs->setObjectName(QString::number(rowIndex)); // ???
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
    const int index = listOfProducts->currentRow(); // ???
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

void MainWindow::comboBox_indexChanged() {
    const QString patternType = ui->comboBox->currentText();
    const int patternTypeIndex = patternTypesList->indexOf(patternType);

    clearGridLayout(ui->gridLayoutSpecial);

    switch (patternTypeIndex) {
        case 0:
            ui->gridLayoutSpecial->setRowStretch(0, 1);
            break;
        case 1: {
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
        } case 2: {
            QGridLayout *gridLayoutSpecial1 = new QGridLayout;

            QGridLayout *gridLayoutSpecial11 = new QGridLayout;
            QSpinBox *spinBoxNumFactories = new QSpinBox;
            QSpinBox *spinBoxNumProducts = new QSpinBox;
            QLabel *labelNumFactories = new QLabel("Enter num of factories:");
            QLabel *labelNumProducts = new QLabel("Enter num of products:");

            QGridLayout *gridLayoutSpecial12 = new QGridLayout;
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
            spinBoxNumProducts->setFixedSize(80, 40);
            spinBoxNumProducts->setFont(QFont("MS Shell Dlg 2", 14));
            spinBoxNumProducts->setMinimum(1);
            spinBoxNumProducts->setMaximum(99);
            labelNumFactories->setMinimumSize(260, 40);
            labelNumFactories->setFont(QFont("MS Shell Dlg 2", 14));
            labelNumProducts->setMinimumSize(260, 40);
            labelNumProducts->setFont(QFont("MS Shell Dlg 2", 14));

            gridLayoutSpecial12->setContentsMargins(20, 0, 0, 0);
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
            gridLayoutSpecial1->addLayout(gridLayoutSpecial12, 0, 2);
            gridLayoutSpecial11->addWidget(labelNumFactories, 0, 0);
            gridLayoutSpecial11->addWidget(spinBoxNumFactories, 0, 1);
            gridLayoutSpecial11->addWidget(labelNumProducts, 1, 0);
            gridLayoutSpecial11->addWidget(spinBoxNumProducts, 1, 1);
            gridLayoutSpecial12->addWidget(btnRawPointer, 0, 0);
            gridLayoutSpecial12->addWidget(btnUniquePointer, 0, 1);
            gridLayoutSpecial12->addWidget(btnSharedPointer, 0, 2);
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
