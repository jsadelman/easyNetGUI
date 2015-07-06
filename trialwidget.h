#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>

class QComboBox;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class ObjectCatalogueFilter;
class DescriptionUpdater;
class QDomDocument;
class QAction;
class QToolButton;
class myComboBox;

class TrialWidget : public QWidget
{
    Q_OBJECT

public:
    TrialWidget(QWidget *parent=0);

    ~TrialWidget();

    QString getTrialCmd();
    bool checkIfReadyToRun();
    QString getStimulusSet();
signals:
    runAllModeChanged(bool);

private slots:
    void update(QString trialName);
    void buildComboBoxes(QDomDocument* domDoc);

    void buildComboBoxesTest(QStringList args);
    void setRunButtonIcon();
    void hideSetComboBox();
    void showSetComboBox();
    void showSetLabel(QString set);
private:

    ObjectCatalogueFilter* trialFilter;
    DescriptionUpdater* trialDescriptionUpdater;
    QMap <QString, QComboBox*> argumentMap;
    QVector <QLabel*> labelList;

    QComboBox*      setComboBox;
    QToolButton*    setCancelButton;

    QHBoxLayout*    layout1;
    QHBoxLayout*    layout2;
    QVBoxLayout*    layout3;
    QHBoxLayout*    layout;
    QAction*        runAction;
    QToolButton*    runButton;
    QAction*        hideSetComboBoxAction;





    void clearLayout(QLayout *layout);
};

#endif // TRIALWIDGET_H
