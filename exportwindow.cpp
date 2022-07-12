#include "exportwindow.h"
#include "ui_exportwindow.h"

#include <QDebug>

void ExportWindow::readSettings() {
    settings->beginGroup("Export");
    const int exportType = settings->value("exportType", CLIPBOARD).toInt();
    switch (exportType) {
        case CLIPBOARD:
            ui->radBtnClipboard->setChecked(true);
            break;
        case CPPFILE:
            ui->radBtnCppFile->setChecked(true);
            break;
        case HANDCPPFILE:
            ui->radBtnCppAndHFile->setChecked(true);
            break;
        default:
            qWarning() << "Wrong export type setting";
            break;
    }
    const QString exportFileName = settings->value("exportFileName").toString();
    ui->lineEditFileName->setText(exportFileName);
    const QString exportFolderPath = settings->value("exportFolderPath").toString();
    ui->lineEditFolderPath->setText(exportFolderPath);
    settings->endGroup();
}

void ExportWindow::writeSettings() {
    settings->beginGroup("Export");
    if (ui->radBtnClipboard->isChecked())
        settings->setValue("exportType", CLIPBOARD);
    else if (ui->radBtnCppFile->isChecked())
        settings->setValue("exportType", CPPFILE);
    else if (ui->radBtnCppAndHFile->isChecked())
        settings->setValue("exportType", HANDCPPFILE);
    settings->setValue("exportFolderPath", ui->lineEditFolderPath->text());
    settings->setValue("exportFileName", ui->lineEditFileName->text());
    settings->endGroup();
}

ExportWindow::ExportWindow(QWidget *parent, QSettings *settings) :
    QDialog(parent),
    ui(new Ui::ExportWindow) {
    ui->setupUi(this);
    this->settings = settings;

    readSettings();
}

ExportWindow::~ExportWindow() {
    delete ui;
}

void ExportWindow::on_pshBtnAccept_clicked() {
    writeSettings();
    this->close();
}

void ExportWindow::on_pshBtnCancel_clicked() {
    this->close();
}
