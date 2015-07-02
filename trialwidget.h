#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>

class QComboBox;
class QLabel;
class QHBoxLayout;
class ObjectCatalogueFilter;
class DescriptionUpdater;
class QDomDocument;
class QAction;
class QToolButton;

class TrialWidget : public QWidget
{
    Q_OBJECT

public:
    TrialWidget(QWidget *parent=0);

    ~TrialWidget();

    QString getTrialCmd();
    bool checkIfReadyToRun();
private slots:
    void update(QString trialName);
    void buildComboBoxes(QDomDocument* domDoc);

    void buildComboBoxesTest(QStringList args);
    void setRunButtonIcon();
private:

    ObjectCatalogueFilter* trialFilter;
    DescriptionUpdater* trialDescriptionUpdater;
    QMap <QString, QComboBox*> argumentMap;
    QVector <QLabel*> labelList;

    QHBoxLayout*    layout;
    QAction*        runAction;
    QToolButton*    runButton;





};

#endif // TRIALWIDGET_H
