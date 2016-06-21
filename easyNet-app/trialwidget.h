#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSet>
#include <QSharedPointer>


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
    QString defaultDataframe();
public slots:
    QStringList getArguments();
signals:
//    void trialDescriptionUpdated(QDomDocument*);
    void aboutToRunTrial(QSharedPointer<QDomDocument>);
    void trialRunModeChanged(int);

private slots:
    void runTrial();
    void update(QString trialName);
    void buildComboBoxes(QDomDocument* domDoc);
    void buildComboBoxesTest(QStringList args = QStringList());
    void setRunButtonIcon();
    void hideSetComboBox();
    void showSetComboBox();
    void showSetLabel(QString set);
    void restoreComboBoxText();
    void argWasChanged(QString arg);
    void clearArgumentBoxes();
    void clearDollarArgumentBoxes();
    void updateModelStochasticity(QDomDocument* modelDescription);
    void addParamExploreDf(QString name);
    void initParamExplore(QString name);
    void runParamExplore(QDomDocument *df, QString name);

private:
    QSharedPointer<QDomDocument> createTrialRunInfo();
    void clearLayout(QLayout *layout);
    void runSingleTrial(LazyNutJob *job);
    void runTrialList(LazyNutJob *job);
    void setStochasticityVisible(bool isVisible);

    ObjectCacheFilter* trialFilter;
    ObjectUpdater* trialDescriptionUpdater;
    ObjectCacheFilter* modelFilter;
    ObjectUpdater* modelDescriptionUpdater;
    ObjectCacheFilter *paramExploreFilter;
    ObjectUpdater* paramExploreDescriptionUpdater;
    ObjectCacheFilter *paramExploreDataframeFilter;
    ObjectUpdater* paramExploreDataframeUpdater;
    QString currentParamExplore;

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
    QMessageBox *disableObserversMsg;
    QCheckBox *dontAskAgainDisableObserverCheckBox;
    bool        askDisableObserver;
    bool        suspendingObservers;


    QMap<QString,QString>     defs;
    int trialRunMode;
    bool isStochastic;


};

#endif // TRIALWIDGET_H
