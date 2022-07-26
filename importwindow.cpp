#include "importwindow.h"
#include "ui_importwindow.h"
#include "classtext.h"
#include "indicator.h"

#include <QFont>
#include <QDebug>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>

void ImportWindow::pushBtnPart_clicked(int importIndex) {
    QFileDialog fileDialog(this);
    fileDialog.setNameFilter(tr("Text files (*.cpp *.h)"));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList fileNames;

    if (!fileDialog.exec())
        return;
    fileNames = fileDialog.selectedFiles();
    QWidget *partContent = qobject_cast<QWidget *>(QObject::sender()->parent());
    if (!partContent)
        qCritical() << "partContent not found";
    const QString importName = partContent->objectName();
    importData.insert(importName, fileNames);

    QHBoxLayout *layoutPart = qobject_cast<QHBoxLayout *>(partContent->layout());
    if (!layoutPart)
        qCritical() << "layoutPart not found";
    QLayoutItem *indicatorItem = layoutPart->itemAt(2); // checkBoxPart in makeImportUIPart method
    if (!indicatorItem)
        qCritical() << "indicator item not found";
    Indicator *indicator = qobject_cast<Indicator *>(indicatorItem->widget());
    if (!indicator)
        qCritical() << "indicator not found";
    indicator->setState(true);
    readyImports[importIndex] = true;
}

void ImportWindow::makeImportUIPart(const QString &name, const QString &text, int importIndex) {
    readyImports[importIndex] = false;
    QWidget *partContent = new QWidget;
    partContent->setObjectName(name);
    QHBoxLayout *layoutPart = new QHBoxLayout;
    layoutPart->setMargin(0);
    QLabel *lblPart = new QLabel(text);
    QPushButton *pushBtnPart = new QPushButton("Import");
    pushBtnPart->setFixedWidth(100);
    Indicator *indicator = new Indicator;

    ui->vertLayoutImports->addWidget(partContent);
    partContent->setLayout(layoutPart);
    layoutPart->addWidget(lblPart);
    layoutPart->addWidget(pushBtnPart);
    layoutPart->addWidget(indicator);

    connect(pushBtnPart, &QPushButton::clicked, this, [this, importIndex]() {
        pushBtnPart_clicked(importIndex);
    });
}

void ImportWindow::initSingletonImportUI() {
    readyImports.resize(1);
    makeImportUIPart("singleton", "Singleton class .h and .cpp", 0);
}

void ImportWindow::initAbstractFactoryImportUI() {
    readyImports.resize(3);
    makeImportUIPart("abstractFactory", "Abstract factory class .h (1 file)", 0);
    makeImportUIPart("factories", "Factories classes .h and .cpp (1 class per file)", 1);
    makeImportUIPart("products", "Products classes .h and .cpp (1 class per file)", 2);
}

void ImportWindow::initBuilderImportUI() {
    readyImports.resize(4);
    makeImportUIPart("director", "Director class .h and .cpp (2 files)", 0);
    makeImportUIPart("abstractBuilder", "Abstract builder class .h (1 file)", 1);
    makeImportUIPart("builders", "Builders classes .h and .cpp (1 class per file)", 2);
    makeImportUIPart("products", "Products classes .h and .cpp (1 class per file)", 3);
}

ImportWindow::ImportWindow(QWidget *parent, int patternType) :
    QDialog(parent), ui(new Ui::ImportWindow) {
    ui->setupUi(this);

    statusBar = new QStatusBar(this);
    statusBar->setFont(QFont("Segoi UI", 10));
    ui->verticalLayout_2->setMargin(0);
    ui->verticalLayout_2->addWidget(statusBar);
    ui->vertLayoutCentral->setMargin(13);

    switch (patternType) {
        case NO_PATTERN:
            break;
        case SINGLETON:
            initSingletonImportUI();
            break;
        case ABSTRACT_FACTORY:
            initAbstractFactoryImportUI();
            break;
        case BUILDER:
            initBuilderImportUI();
            break;
        default:
            qWarning() << "Unexpected pattern type";
            break;
    }

    connect(this, SIGNAL(sendImportsToMainWindow(const QHash<QString, QStringList>&)),
            parent, SLOT(importAccepted(const QHash<QString, QStringList>&)));
}

ImportWindow::~ImportWindow() {
    delete ui;
}

void ImportWindow::on_pushBtnAccept_clicked() {
    if (readyImports.indexOf(false) == -1) {
        sendImportsToMainWindow(importData);
        close();
    } else {
        statusBar->showMessage("Import all necessary files!");
    }
}

void ImportWindow::on_pushBtnCancel_clicked() {
    this->close();
}
