#include "exportwindow.h"
#include "ui_exportwindow.h"

#include <QDebug>

void ExportWindow::readSettings() {
    settings->beginGroup("Export");
    const int exportType = settings->value("type", CLIPBOARD).toInt();
    switch (exportType) {
        case CLIPBOARD:
            ui->radBtnClipboard->setChecked(true);
            break;
        case CPP_FILE:
            ui->radBtnCppFile->setChecked(true);
            break;
        case H_AND_CPP_FILE:
            ui->radBtnCppAndHFile->setChecked(true);
            break;
        case H_AND_CPP_SEP_FILES:
            ui->radBtnCppAndHSepFiles->setChecked(true);
            break;
        default:
            qWarning() << "Wrong export type setting";
            break;
    }
    ui->lblFileName->setEnabled(exportType == CPP_FILE or exportType == H_AND_CPP_FILE);
    ui->lineEditFileName->setEnabled(exportType == CPP_FILE or exportType == H_AND_CPP_FILE);
    ui->lblFolderPath->setEnabled(exportType != CLIPBOARD);
    ui->lineEditFolderPath->setEnabled(exportType != CLIPBOARD);

    const QString fileName = settings->value("fileName").toString();
    ui->lineEditFileName->setText(fileName);
    const QString folderPath = settings->value("folderPath").toString();
    ui->lineEditFolderPath->setText(folderPath);
    settings->endGroup();
}

void ExportWindow::writeSettings() {
    settings->beginGroup("Export");
    if (ui->radBtnClipboard->isChecked())
        settings->setValue("type", CLIPBOARD);
    else if (ui->radBtnCppFile->isChecked())
        settings->setValue("type", CPP_FILE);
    else if (ui->radBtnCppAndHFile->isChecked())
        settings->setValue("type", H_AND_CPP_FILE);
    else if (ui->radBtnCppAndHSepFiles->isChecked())
        settings->setValue("type", H_AND_CPP_SEP_FILES);
    settings->setValue("folderPath", ui->lineEditFolderPath->text());
    settings->setValue("fileName", ui->lineEditFileName->text());
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

void ExportWindow::on_radBtnClipboard_clicked()
{
    ui->lblFileName->setEnabled(false);
    ui->lineEditFileName->setEnabled(false);
    ui->lblFolderPath->setEnabled(false);
    ui->lineEditFolderPath->setEnabled(false);
}

void ExportWindow::on_radBtnCppFile_clicked()
{
    ui->lblFileName->setEnabled(true);
    ui->lineEditFileName->setEnabled(true);
    ui->lblFolderPath->setEnabled(true);
    ui->lineEditFolderPath->setEnabled(true);
}

void ExportWindow::on_radBtnCppAndHFile_clicked()
{
    ui->lblFileName->setEnabled(true);
    ui->lineEditFileName->setEnabled(true);
    ui->lblFolderPath->setEnabled(true);
    ui->lineEditFolderPath->setEnabled(true);
}

void ExportWindow::on_radBtnCppAndHSepFiles_clicked()
{
    ui->lblFileName->setEnabled(false);
    ui->lineEditFileName->setEnabled(false);
    ui->lblFolderPath->setEnabled(true);
    ui->lineEditFolderPath->setEnabled(true);
}
