#ifndef XMLDISPLAY_H
#define XMLDISPLAY_H

#include <QScrollArea>


class ObjectCacheFilter;
class ObjectUpdater;
class QDomDocument;
class QVBoxLayout;
class XMLForm;

class XMLDisplay: public QScrollArea
{
    Q_OBJECT
public:
    XMLDisplay(QWidget *parent = 0);
    ~XMLDisplay();

public slots:
    void setName(QString name);

protected slots:
    virtual void buildForm(QDomDocument* domDoc);

protected:
    virtual void newForm(QDomDocument* domDoc);

    ObjectCacheFilter* objectFilter;
    ObjectUpdater* objectDescriptionUpdater;
    XMLForm* form;
    QVBoxLayout *layout;
    QString getCmd;

};





#endif // XMLDISPLAY_H
