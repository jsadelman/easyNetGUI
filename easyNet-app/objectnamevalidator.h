#ifndef OBJECTNAMEVALIDATOR_H
#define OBJECTNAMEVALIDATOR_H

#include <QValidator>
#include <QString>

class ObjectNameValidator : public QValidator {
    Q_OBJECT
public:
    explicit ObjectNameValidator(QObject* parent = 0, QStringList lazyNutkeywords=QStringList{""});
    QValidator::State validate(QString& input, int& pos) const Q_DECL_OVERRIDE;
    bool isValid(QString name);
    QString makeValid(QString name);

private:
    QRegExp forbiddenRex;

};

#endif // OBJECTNAMEVALIDATOR_H
