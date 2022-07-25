#ifndef INDICATOR_H
#define INDICATOR_H

#include <QLabel>
#include <QPixmap>

QT_BEGIN_NAMESPACE
namespace Ui { class Indicator; }
QT_END_NAMESPACE

class Indicator : public QLabel {

    Q_OBJECT

public:
    Indicator(QWidget *parent = nullptr, int width = 32, int height = 32, bool state = false);

    void setState(bool state);

private:
    bool state;
    QPixmap picOk;
    QPixmap picNotOk;

};

#endif // INDICATOR_H
