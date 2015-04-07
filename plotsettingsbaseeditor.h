#ifndef PLOTSETTINGSBASEEDITOR_H
#define PLOTSETTINGSBASEEDITOR_H

#include <QWidget>

class QLineEdit;
class QLabel;

class PlotSettingsBaseEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsBaseEditor(QWidget *parent = 0);
    virtual QString getValue();
    void setName(QString name);
    void setComment(QString comment) {m_comment = comment;}
    virtual void setValue(QVariant v);


signals:

public slots:

private slots:
    void displayComment();

private:
    QString m_comment;
    QLineEdit *valueEdit;
    QLabel *nameLabel;

};

#endif // PLOTSETTINGSBASEEDITOR_H
