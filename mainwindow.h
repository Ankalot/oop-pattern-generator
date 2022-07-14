#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGridLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QTableWidget>
#include <QSettings>

class CodeGenerator;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_pushBtnGenerate_clicked();
    void comboBox_indexChanged();

    void spinBoxNumFactoriesChanged(int nextFactoriesNum);
    void spinBoxNumProductsChanged(int nextProductsNum);

    void changeMethodsCountInTable(int nextMethodsNum);
    void changeArgsCountInTable(int nextArgsNum);

    void changeProductNameInTable(QListWidgetItem *productNameItem);

    void on_actionExport_triggered();

    void on_checkBoxImport_clicked(bool checked);

    void on_pushBtnImport_clicked();

    void importAccepted(const QHash<QString, QStringList> &data);

private:
    Ui::MainWindow *ui;
    CodeGenerator *codeGenerator;
    QSettings *settings;
    QHash<QString, QStringList> importData;

    void addItemToLayoutProductsMethodsList(QHBoxLayout *gridLauoutProductsMethodsList, const QString &productName);
    void delItemFromLayoutProductsMethodsList(QHBoxLayout *layoutProductsMethodsList, int index);
    void addSpinBoxNumArgsToCell(QTableWidget *table, int rowIndex, int columnIndex);

    void readSettings();
    void writeSettings();

    bool generateSingleton(int exportType);
    bool generateAbstractFactory(int exportType);

    QString getExportFolderPath();
    bool writeTextToFile(const QString &fileFullName, const QString &text);

    QStringList *patternTypesList;

    const int TableOfProductMethodsWidth = 700;
    const int TableOfProductMethodsHeight = 350;

    enum PATTERN_TYPE { NO_PATTERN, SINGLETON, ABSTRACT_FACTORY };
    enum EXPORT_TYPE { CLIPBOARD, CPP_FILE, H_AND_CPP_FILES };
};
#endif // MAINWINDOW_H
