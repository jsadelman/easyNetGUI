#ifndef PLOTSETTINGSBASEWIDGET_H
#define PLOTSETTINGSBASEWIDGET_H

#include <QWidget>
#include <QLineEdit>

class QDomDocument;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QListWidget;
class QListWidgetItem;

class ValueEdit: public QLineEdit
{
    Q_OBJECT
public:
    explicit ValueEdit(QWidget *parent = 0);

signals:
    void doubleClicked();



protected:
    virtual void mouseDoubleClickEvent(QMouseEvent * mouseEvent) Q_DECL_OVERRIDE;
};

class PlotSettingsBaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsBaseWidget(QString name, QString value, QString comment,
                                    QString defaultValue, QWidget *parent = 0);
    QString getName() {return name;}
    virtual QString getValue();

signals:

public slots:

protected slots:
    void displayComment();

protected:
    virtual void createDisplay();
    virtual void addValueEdit();

    QString name;
    QString value;
    QString comment;
    QString defaultValue;
    QPushButton *nameButton;
    QHBoxLayout *displayLayout;
    QVBoxLayout *mainLayout;
//    QPushButton *commentButton;
    QWidget *valueEdit;

};

class PlotSettingsNumericWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsNumericWidget(QString name, QString value, QString comment,
                                    QString defaultValue, QWidget *parent = 0);
    virtual QString getValue();

protected:
    virtual void addValueEdit();
};


class PlotSettingsFactorWidget : public PlotSettingsBaseWidget
{
    Q_OBJECT
public:
    explicit PlotSettingsFactorWidget(QString name, QString value, QString comment,
                                    QString defaultValue, QWidget *parent = 0);
    virtual QString getValue();

signals:
    void addWidget(QWidget*);

public slots:
    void createlistEdit(QStringList list);


protected:
    virtual void addValueEdit();

private slots:
    void addItems();
    void removeItems();

private:
    QString formatFactorStringForR(QString factorString);
    QString formatFactorStringForDisplay(QString factorString);
    void moveItems(QListWidget *fromList, QListWidget *toList, QList<QListWidgetItem *> items);
    void updateValueFromEdit();

    QListWidget *factorList;
    QListWidget *selectedList;
};


#endif // PLOTSETTINGSBASEWIDGET_H
