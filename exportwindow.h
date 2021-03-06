#ifndef EXPORTWINDOW_H
#define EXPORTWINDOW_H

#include <QDialog>
#include <QSettings>

namespace Ui { class ExportWindow; }

class ExportWindow : public QDialog
{
    Q_OBJECT

public:
    ExportWindow(QWidget *parent = nullptr, QSettings *settings = nullptr);
    ~ExportWindow();

private slots:
    void on_pshBtnAccept_clicked();

    void on_pshBtnCancel_clicked();

    void on_radBtnClipboard_clicked();

    void on_radBtnCppFile_clicked();

    void on_radBtnCppAndHFiles_clicked();

private:
    Ui::ExportWindow *ui;
    QSettings *settings;

    void readSettings();
    void writeSettings();

    enum EXPORT_TYPE { CLIPBOARD, CPP_FILE, H_AND_CPP_FILES };

};

#endif // EXPORTWINDOW_H
