#ifndef TRIALEDITOR_H
#define TRIALEDITOR_H

#include <QScrollArea>

class ObjectCatalogueFilter;
class DescriptionUpdater;
class QDomDocument;
class QVBoxLayout;
class TrialXML;

class TrialEditor: public QScrollArea
{
    Q_OBJECT
public:
    TrialEditor(QWidget *parent = 0);
    ~TrialEditor();

public slots:
    void setTrialName(QString name);

private slots:
    void buildForm(QDomDocument* domDoc);

private:
    ObjectCatalogueFilter* trialFilter;
    DescriptionUpdater* trialDescriptionUpdater;
    TrialXML* form;
    QVBoxLayout *layout;
};

#endif // TRIALEDITOR_H
