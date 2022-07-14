#include "indicator.h"

Indicator::Indicator(QWidget *parent, int width, int height, bool state)
    : QLabel(parent) {
    picOk = QPixmap(":/icons/images/ok.png");
    picNotOk = QPixmap(":/icons/images/not_ok.png");

    this->setFixedSize(width, height);
    this->setPixmap(state ? picOk : picNotOk);
}

void Indicator::setState(bool state) {
    this->setPixmap(state ? picOk : picNotOk);
    this->state = state;
}
