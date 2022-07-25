#ifndef IMPORTWINDOW_H
#define IMPORTWINDOW_H

#include <QDialog>
#include <QStatusBar>

namespace Ui { class ImportWindow; }

class ClassText;

class ImportWindow : public QDialog
{
    Q_OBJECT

public:
    ImportWindow(QWidget *parent = nullptr, int patternType = 0);
    ~ImportWindow();

private slots:
    void on_pushBtnAccept_clicked();

    void on_pushBtnCancel_clicked();

    void pushBtnPart_clicked();

signals:
    void sendImportsToMainWindow(const QHash<QString, QStringList> &importData);

private:
    Ui::ImportWindow *ui;
    QStatusBar *statusBar;

    int missingImportsNum = 0;
    QHash<QString, QStringList> importData;

    enum PATTERN_TYPE { NO_PATTERN, SINGLETON, ABSTRACT_FACTORY };

    void initSingletonImportUI();
    void initAbstractFactoryImportUI();
    void makeImportUIPart(const QString &name, const QString &text);

};

#endif // IMPORTWINDOW_H
