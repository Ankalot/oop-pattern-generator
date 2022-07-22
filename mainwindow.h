#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGridLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QTableWidget>
#include <QSettings>
#include <QSpinBox>

class CodeGenerator;
class ClassText;
class ParsedElements;
class Element;
class VectorElement;

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
    void spinBoxNumBuildersChanged(int nextBuildersNum);
    void spinBoxDirectorMethodsNumChanged(int nextDirectorMethodsNum);
    void spinBoxAbstractBuilderMethodsNumChanged(int nextAbstractBuilderMethodsNum);

    void changeMethodsCountInTable(int nextMethodsNum);
    void changeArgsCountInTable(int nextArgsNum);

    void changeNameInTable(QListWidgetItem *productNameItem);

    void on_actionExport_triggered();

    void on_checkBoxImport_toggled(bool checked);

    void on_pushBtnImport_clicked();

    void importAccepted(const QHash<QString, QStringList> &data);

private:
    Ui::MainWindow *ui;
    CodeGenerator *codeGenerator;
    QSettings *settings;
    QHash<QString, QStringList> importData;
    QHash<QString, QVector<ClassText *>> parseData;
    ParsedElements *parsedPattern = nullptr;
    QVector<QWidget *> freezedWidgets;

    void changeClassesNumInNamesList(QListWidget *listOfClasses, int nextClassesNum, const QString &className);
    void changeClassesNumInNamesListAndMethodsList(QListWidget *listOfClasses, QHBoxLayout *layoutMethodsList,
                                                   int nextProductsNum, const QString &className);
    void addItemToLayoutMethodsList(QHBoxLayout *gridLauoutProductsMethodsList, const QString &productName);
    void delItemFromLayoutProductsMethodsList(QHBoxLayout *layoutProductsMethodsList, int index);
    void addSpinBoxNumArgsToCell(QTableWidget *table, int rowIndex, int columnIndex);
    void addRowToMethodsTable(unsigned i, QTableWidget *tableMethods);

    void readSettings();
    void writeSettings();

    bool generateSingleton(int exportType);
    bool generateAbstractFactory(int exportType);

    bool makeParseData();
    bool parseSingleton();
    bool parseAbstractFactory();

    void unfreezeUi();
    void initParsedSingletonAndUi(QLineEdit **lineEditSnglt, Element **className);
    void initParsedAbstractFactoryAndUi(QSpinBox **spinBoxNumFactories, QSpinBox **spinBoxNumProducts,
                                        QLineEdit **lineEditFactoryName, QListWidget **listOfFactories,
                                        QListWidget **listOfProducts, QHBoxLayout **layoutProductsMethodsList,
                                        Element **abstractFactoryName, VectorElement **factoriesNames,
                                        VectorElement **productsNames, VectorElement **productsMethods);
    void writeParsedSingletonToUi();
    void writeParsedAbstractFactoryToUi();
    void writeUiToParsedSingleton();
    void writeUiToParsedAbstractFactory();

    QString getExportFolderPath();
    bool writeTextToFile(const QString &fileFullName, const QString &text);

    QStringList *patternTypesList;

    const int TableOfMethodsWidth = 700;
    const int TableOfMethodsHeight = 350;

    enum PATTERN_TYPE { NO_PATTERN, SINGLETON, ABSTRACT_FACTORY, BUILDER };
    enum EXPORT_TYPE { CLIPBOARD, CPP_FILE, H_AND_CPP_FILES };
};
#endif // MAINWINDOW_H
