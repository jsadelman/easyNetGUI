#ifndef TRIALWIDGET_H
#define TRIALWIDGET_H

#include <QWidget>
#include <QMap>

class QComboBox;
class QHBoxLayout;
class ObjectCatalogueFilter;
class DescriptionUpdater;
class QDomDocument;

class TrialWidget : public QWidget
{
    Q_OBJECT

public:
    TrialWidget(QWidget *parent=0);

    ~TrialWidget();

private slots:
    void update(QString trialName);
    void buildComboBoxes(QDomDocument* domDoc);

private:

    ObjectCatalogueFilter* trialFilter;
    DescriptionUpdater* trialDescriptionUpdater;
    QMap <QString, QComboBox*> argumentMap;

    QHBoxLayout* layout;




};

#endif // TRIALWIDGET_H
