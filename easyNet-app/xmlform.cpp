#include "xmlform.h"

#include <QDebug>
#include <QtWidgets>


XMLForm::XMLForm(const QDomElement &domElem, QWidget *parent)
    : domElem(domElem), QFrame(parent)
{
}

XMLForm::~XMLForm()
{

}

void XMLForm::build()
{
    if (domElem.tagName() == "string")
            buildString();
    else if (domElem.tagName() == "integer")
        buildInteger();
    else if (domElem.tagName() == "real")
        buildReal();
    else if (domElem.tagName() == "list")
        buildList();
    else if (domElem.tagName() == "map")
        buildMap();
    else if (domElem.tagName() == "object")
        buildObject();
    else if (domElem.tagName() == "command")
        buildCommand();
    else if (domElem.tagName() == "parameter")
        buildParameter();
    else
        return;
}

void XMLForm::buildLabelValue()
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);
    nodeLayout = new QHBoxLayout();
    buildLabel();
    buildValue();
    if (domElem.hasChildNodes())
    {
        childrenLayout = new QVBoxLayout();
    }
}

void XMLForm::buildFrameLabel()
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    setLayout(frameLayout);
    nodeLayout = new QHBoxLayout();
    frameLayout->addLayout(nodeLayout);
    buildLabel();

    if (domElem.hasChildNodes())
    {
        childrenLayout = new QVBoxLayout();
        frameLayout->addLayout(childrenLayout);
    }

}

void XMLForm::indentChildren()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(nodeLayout);
    QHBoxLayout *indentLayout = new QHBoxLayout();
    indentLayout->addStretch();

    indentLayout->addLayout(childrenLayout);

    recursiveBuild();

    mainLayout->addLayout(indentLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

void XMLForm::frameChildren()
{
    recursiveBuild();
}

void XMLForm::recursiveBuild()
{
    QDomElement element = domElem.firstChildElement();
    while (!element.isNull())
    {
        XMLForm *childForm = new XMLForm(element);
        childForm->build();
        static_cast<QVBoxLayout*>(childrenLayout)->addWidget(childForm);
        element = element.nextSiblingElement();
    }
    static_cast<QVBoxLayout*>(childrenLayout)->addStretch();
}

void XMLForm::buildShallowList()
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    setLayout(frameLayout);
    nodeLayout = new QHBoxLayout();
    frameLayout->addLayout(nodeLayout);
    buildLabel();

    if (domElem.hasChildNodes())
    {
        QListWidget *listWidget = new QListWidget();
        frameLayout->addWidget(listWidget);
        QDomElement element = domElem.firstChildElement();
        while (!element.isNull())
        {
            QString valueString = element.attribute("value");
            listWidget->addItem(new QListWidgetItem(valueString));
            element = element.nextSiblingElement();
        }
        listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        listWidget->setMinimumWidth(listWidget->sizeHintForColumn(0)*1.1);
    }
}

void XMLForm::buildLabel()
{
    QString labelString = domElem.attribute("label");
    if (!labelString.isEmpty())
        labelString.append(":");
    QLabel *labelLabel = new QLabel(labelString);
//    labelLabel->setStyleSheet("QLabel {"
//                            "font-weight: bold;"
//                            "}");
    static_cast<QHBoxLayout*>(nodeLayout)->addWidget(labelLabel);
    static_cast<QHBoxLayout*>(nodeLayout)->addStretch();
}

void XMLForm::buildValue()
{
    QString valueString = domElem.attribute("value");
    QLineEdit *valueEdit = new QLineEdit();
    valueEdit->setReadOnly(true);
    valueEdit->setText(valueString);
    QFontMetrics fm = valueEdit->fontMetrics();
    int w = fm.boundingRect(valueString).width();
    valueEdit->setMinimumWidth(w+30);
    static_cast<QHBoxLayout*>(nodeLayout)->addWidget(valueEdit);
}

void XMLForm::buildParameterType()
{
    QString typeString = QString("parameter type: %1").arg(domElem.attribute("type"));
    QLineEdit *valueEdit = new QLineEdit();
    valueEdit->setReadOnly(true);
    valueEdit->setPlaceholderText(typeString);
    QFontMetrics fm = valueEdit->fontMetrics();
    int w = fm.boundingRect(typeString).width();
    valueEdit->setMinimumWidth(w+30);
    static_cast<QHBoxLayout*>(nodeLayout)->addWidget(valueEdit);
}

void XMLForm::buildNodeString()
{
    buildLabelValue();
}

void XMLForm::buildNodeInteger()
{
    buildLabelValue();
}

void XMLForm::buildNodeReal()
{
    buildLabelValue();
}

void XMLForm::buildNodeList()
{
    if (!domElem.hasChildNodes() || !domElem.firstChildElement().hasChildNodes())
        buildShallowList();
    else
        buildFrameLabel();
}

void XMLForm::buildNodeMap()
{
    buildFrameLabel();
}

void XMLForm::buildNodeObject()
{
    buildLabelValue();
}

void XMLForm::buildNodeCommand()
{
    buildLabelValue();
}

void XMLForm::buildNodeParameter()
{
//    buildLabelValue();
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);
    nodeLayout = new QHBoxLayout();
    buildLabel();
    buildParameterType();
    if (domElem.hasChildNodes())
    {
        childrenLayout = new QVBoxLayout();
    }
}

void XMLForm::buildChildrenString()
{
    indentChildren();
}

void XMLForm::buildChildrenInteger()
{
    indentChildren();
}

void XMLForm::buildChildrenReal()
{
    indentChildren();
}

void XMLForm::buildChildrenList()
{
    if (!domElem.hasChildNodes() || !domElem.firstChildElement().hasChildNodes())
        return;
    else
        frameChildren();
}

void XMLForm::buildChildrenMap()
{
    frameChildren();
}

void XMLForm::buildChildrenObject()
{
    indentChildren();
}

void XMLForm::buildChildrenCommand()
{
    indentChildren();
}

void XMLForm::buildChildrenParameter()
{
    indentChildren();
}

