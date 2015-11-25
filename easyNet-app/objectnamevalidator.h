#ifndef OBJECTNAMEVALIDATOR_H
#define OBJECTNAMEVALIDATOR_H

#include <QValidator>
#include <QString>

class ObjectNameValidator : public QValidator {
    Q_OBJECT
public:
    explicit ObjectNameValidator(QObject* parent = 0);
    QValidator::State validate(QString& input, int& pos) const Q_DECL_OVERRIDE;
    bool isValid(QString name);
    QString makeValid(QString name);

private:
    QStringList forbiddenNames;

};

#endif // OBJECTNAMEVALIDATOR_H
