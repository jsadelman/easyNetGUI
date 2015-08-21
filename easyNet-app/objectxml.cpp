#include "objectxml.h"

#include <QLabel>
#include <QFrame>

ObjectXML::ObjectXML(const QDomElement &domElem, QWidget *parent)
    : XMLForm(domElem, parent)
{
}

ObjectXML::~ObjectXML()
{
}

void ObjectXML::buildNodeObject()
{
    if (domElem.attribute("label") == "this" && domElem.parentNode().parentNode().isNull())
    {
        QFrame *labelFrame = new QFrame();
        labelFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
        labelFrame->setLineWidth(1);
        nodeLayout = new QVBoxLayout();
        nodeLayout->addWidget(labelFrame);

        QHBoxLayout *labelLayout = new QHBoxLayout();
        QString nameString = domElem.attribute("value");
        QLabel *nameLabel = new QLabel(nameString);
        nameLabel->setStyleSheet("QLabel {"
                                "font-weight: bold;"
                                "}");
        labelLayout->addWidget(nameLabel);
        labelFrame->setLayout(labelLayout);
        if (domElem.hasChildNodes())
        {
            childrenLayout = new QVBoxLayout();
        }
    }
    else
    {
        XMLForm::buildNodeObject();
    }
}

