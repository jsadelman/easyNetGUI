#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSet>

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
class QMessageBox;
class QCheckBox;

class TrialWidget : public QWidget
{
    Q_OBJECT

public:
    TrialWidget(QWidget *parent=0);

    ~TrialWidget();

    QString getTrialCmd();
    QString getStochasticCmd();
    bool checkIfReadyToRun();
    QString getStimulusSet();
public slots:
    QStringList getArguments();
signals:
    void runAllModeChanged(bool);
    void trialDescriptionUpdated(QDomDocument*);

private slots:

    void runTrial();
    void runSingleTrial();
    void runTrialList();

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
    void updateModelStochasticity(QDomDocument* modelDescription);
    void observerEnabled(QString name= QString());
    void observerDisabled(QString name= QString());
    void suspendObservers();
    void restoreObservers();
private:

    void setStochasticityVisible(bool isVisible);

    ObjectCacheFilter* trialFilter;
    ObjectUpdater* trialDescriptionUpdater;
    ObjectCacheFilter* modelFilter;
    ObjectUpdater* modelDescriptionUpdater;
    QMap <QString, myComboBox*> argumentMap;
    QVector <QLabel*> labelList;

    QComboBox*      setComboBox;
    QToolButton*    setCancelButton;
    QLabel*         repetitionsLabel;
    QComboBox*      repetitionsBox;
    QLabel*         strategyLabel;
    QComboBox*      strategyBox;

    QHBoxLayout*    layout1;
    QHBoxLayout*    layout2;
    QVBoxLayout*    layout3;
    QHBoxLayout*    layout;
    QAction*        runAction;
    QToolButton*    runButton;
    QAction*        hideSetComboBoxAction;
    QString         argChanged;
    QSet<QString>   enabledObservers;
    QMessageBox *disableObserversMsg;
    QCheckBox *dontAskAgainDisableObserverCheckBox;
    bool        askDisableObserver;
    bool        suspendingObservers;


    QMap<QString,QString>     defs;
    bool runAllMode;
    bool isStochastic;



    void clearLayout(QLayout *layout);
};

#endif // TRIALWIDGET_H
