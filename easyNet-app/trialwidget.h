#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>

class QComboBox;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;
class QAction;
class QToolButton;
class myComboBox;
class LazyNutJob;

class TrialWidget : public QWidget
{
    Q_OBJECT

public:
    TrialWidget(QWidget *parent=0);

    ~TrialWidget();

    QString getTrialCmd();
    bool checkIfReadyToRun();
    QString getStimulusSet();
    QString defaultDataframe();
public slots:
    QStringList getArguments();
signals:
    void runAllModeChanged(bool);
    void trialDescriptionUpdated(QDomDocument*);

private slots:

    void runTrial();


    void update(QString trialName);
    void buildComboBoxes(QDomDocument* domDoc);

    void buildComboBoxesTest(QStringList args);
    void setRunButtonIcon();
    void hideSetComboBox();
    void showSetComboBox();
    void showSetLabel(QString set);
    void restoreComboBoxText();
    void argWasChanged(QString arg);
    void clearArgumentBoxes();
    void clearDollarArgumentBoxes();

private:

    QDomDocument * createTrialRunInfo();
    void clearLayout(QLayout *layout);
    void runSingleTrial(LazyNutJob *job);
    void runTrialList(LazyNutJob *job);

    ObjectCacheFilter* trialFilter;
    ObjectUpdater* trialDescriptionUpdater;
    QMap <QString, myComboBox*> argumentMap;
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
    QString         argChanged;

    QMap<QString,QString>     defs;
    bool runAllMode;


};

#endif // TRIALWIDGET_H
