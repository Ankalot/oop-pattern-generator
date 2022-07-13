#include "classtext.h"

ClassText::ClassText(const QString &fileName, const QString &text, const QString &fileType) {
    this->fileName = fileName;
    this->text = text;
    this->fileType = fileType;
}

const QString &ClassText::getFileName() const {
    return fileName;
}

const QString &ClassText::getText() const {
    return text;
}

const QString &ClassText::getFileType() const {
    return fileType;
}
