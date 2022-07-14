#ifndef CLASSTEXT_H
#define CLASSTEXT_H

#include <QString>

QT_BEGIN_NAMESPACE
namespace PatternGenerator { class ClassMethod; }
QT_END_NAMESPACE

class ClassText {
public:
    ClassText(const QString &fileName, const QString &text, const QString &fileType);

    const QString &getFileName() const;
    const QString &getText() const;
    const QString &getFileType() const;

private:
    QString fileName;
    QString text;
    QString fileType;

};

#endif // CLASSTEXT_H
