#ifndef XMLFORM_H
#define XMLFORM_H


#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QFrame>
#include <QVBoxLayout>

#define buildMethods(Tag) virtual void build ## Tag()\
{\
    buildNode ## Tag();\
    if (domElem.hasChildNodes())\
    {\
        buildChildren ## Tag();\
    }\
    else\
    {\
        if (!layout())\
            setLayout(nodeLayout);\
    }\
}\
    virtual void buildNode ## Tag();\
    virtual void buildChildren ## Tag();\


class QVBoxLayout;

class XMLForm: public QFrame
{
//    Q_OBJECT

public:
    XMLForm(const QDomElement& domElem, QWidget *parent = 0);
    ~XMLForm();
    void build();


protected:
    buildMethods(String)
    buildMethods(Integer)
    buildMethods(Real)
    buildMethods(List)
    buildMethods(Map)
    buildMethods(Object)
    buildMethods(Command)
    buildMethods(Parameter)

    //----- auxiliary methods -------//

    // buildNodeTag
    virtual void buildLabelValue();
    virtual void buildFrameLabel(); // for list and map

    // buildChildrenTag
    virtual void indentChildren();
    virtual void frameChildren(); // for list and map

    virtual void recursiveBuild();
    void buildShallowList();

    virtual void buildLabel();
    virtual void buildValue();
    virtual void buildParameterType();

    QDomElement domElem;
    QLayout *nodeLayout;
    QLayout *childrenLayout;
};

#endif // XMLFORM_H
