#include <QtWidgets>

#include "plotsettingsbasewidget.h"
//#include "lazynutjobparam.h"
//#include "sessionmanager.h"
//#include "lazynutlistwidget.h"
#include "lazynutlistcombobox.h"
#include "lazynutpairedlistwidget.h"


PlotSettingsBaseWidget::PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent)
    : settingsElement(settingsElement), QWidget(parent)
{
    createDisplay();
    editMode = RawEditMode;
    // debug
    connect(this, &PlotSettingsBaseWidget::valueChanged, [=](){
        qDebug() << "valueChanged emitted, value(): " << value();});
}



void PlotSettingsBaseWidget::createDisplay()
{
    mainLayout = new QGridLayout;
    nameLabel = new QLabel;
    QString labelText = settingsElement["pretty name"]().isEmpty() ? name() : settingsElement["pretty name"]();
    if (!hasDefault())
        labelText.append("*");

    nameLabel->setText(labelText);
    nameLabel->setStyleSheet("QLabel {"
                             "font-weight: bold;"
                             "}");
    editStackedLayout = new QStackedLayout;
    rawEdit = new QLineEdit;
    if (!hasDefault())
        rawEdit->setPlaceholderText("*mandatory field*");

    if (settingsElement["value"]().isEmpty())
    {
        if (hasDefault())
            rawEdit->setText(settingsElement["default"]());
    }
     else
        rawEdit->setText(settingsElement["value"]());


    connect(rawEdit, SIGNAL(editingFinished()), this, SLOT(emitValueChanged()));
    editStackedLayout->addWidget(rawEdit);
    editStackedLayout->setCurrentIndex(editStackedLayout->indexOf(rawEdit));
    commentLabel = new QLabel;
    commentLabel->setText(settingsElement["comment"]());
    commentLabel->setWordWrap(true);
    commentLabel->setStyleSheet("QLabel {"
                                "background-color: lightyellow;"
                                "border: 1px solid black;"
                                "padding: 4px;"
                                "font-family: Arial;"
                                "font-style: italic;"
                                "}");
    editModeButtonBox = new QGroupBox("Edit mode");
    editModeButtonBoxLayout = new QVBoxLayout;
//    debugButton = new QPushButton("value()");
//    connect(debugButton, &QPushButton::clicked, [=](){qDebug() << value();});
//    debugButton->setToolTip("Open edit window");
//    debugButton->setEnabled(false);
    widgetEditButton  = new QRadioButton("Quick input");
    widgetEditButton->setEnabled(false);
    connect(widgetEditButton, SIGNAL(toggled(bool)), this, SLOT(setWidgetMode(bool)));
    rawEditButton = new QRadioButton("Manual input");
    rawEditButton->setEnabled(false);
    rawEditButton->setChecked(true);
    connect(rawEditButton, SIGNAL(toggled(bool)), this, SLOT(setRawMode(bool)));
    editModeButtonBoxLayout->addWidget(widgetEditButton);
    editModeButtonBoxLayout->addWidget(rawEditButton);
    editModeButtonBox->setLayout(editModeButtonBoxLayout);


    if (hasDefault())
    {
        defaultButton = new QPushButton("Default");
        defaultButton->setToolTip(QString("Reset default value: %1").arg(settingsElement["default"]()));
    }
    else
    {
        defaultButton = new QPushButton("Unset");
        defaultButton->setToolTip("Leave the value unspecified");
    }
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(resetDefault()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(emitValueChanged()));

    mainLayout->addWidget(nameLabel, 0, 0);
    mainLayout->addLayout(editStackedLayout, 0, 1);
    mainLayout->addWidget(defaultButton, 0, 2);
    mainLayout->addWidget(commentLabel, 1, 0, 1, 3);
    mainLayout->addWidget(editModeButtonBox, 0, 3, 2, 1);
    mainLayout->setRowStretch(0,1);
    mainLayout->setRowStretch(1,1);
    setLayout(mainLayout);

    valueSet =  hasDefault() ?
                settingsElement["value"]() != settingsElement["default"]() :
                !settingsElement["value"]().isEmpty();

    currentValue = settingsElement["value"]();


    // set max size to fit text
    // http://stackoverflow.com/questions/6639012/minimum-size-width-of-a-qpushbutton-that-is-created-from-code
//    QSize textSize = nameButton->fontMetrics().size(Qt::TextShowMnemonic, nameButton->text());
//    QStyleOptionButton opt;
//    opt.initFrom(nameButton);
//    opt.rect.setSize(textSize);
//    nameButton->setMaximumSize(nameButton->style()->sizeFromContents(QStyle::CT_PushButton,
//                                        &opt, textSize, nameButton));


    //    setAttribute(Qt::WA_AlwaysShowToolTips);
}

QString PlotSettingsBaseWidget::getValue()
{
    switch(editMode)
    {
    case RawEditMode:
        return rawEdit->text();
    case WidgetEditMode:
        return widget2rawValue(getWidgetValue());
    }
}

void PlotSettingsBaseWidget::setValue(QString val)
{
    rawEdit->setText(val);
    setWidgetValue(raw2widgetValue(val));
}

bool PlotSettingsBaseWidget::isValueSet()
{
    return valueSet || rawEdit->isModified();
}

bool PlotSettingsBaseWidget::hasDefault()
{
    return settingsElement.listLabels().contains("default");
}


QString PlotSettingsBaseWidget::value()
{
    if (isValueSet())
        return getValue();

    return QString();
}


void PlotSettingsBaseWidget::resetDefault()
{

    QString defaultValue = hasDefault() ? settingsElement["default"]() : QString();
    if (valueSet)
    {
        setValue(defaultValue);
        valueSet = false;
        qDebug() << "resetDefault, value()" << value();
        if (currentValue != getValue())
        {
            currentValue = getValue();
            emit valueChanged();
        }
    }
}

void PlotSettingsBaseWidget::setWidgetMode(bool on)
{
    if (on)
    {
        setWidgetValue(raw2widgetValue(rawEdit->text()));
        editStackedLayout->setCurrentIndex(editStackedLayout->indexOf(editWidget));
        editMode = WidgetEditMode;
    }
}


void PlotSettingsBaseWidget::setRawMode(bool on)
{
    if (on)
    {
        rawEdit->setText(widget2rawValue(getWidgetValue()));
        editStackedLayout->setCurrentIndex(editStackedLayout->indexOf(rawEdit));
        editMode = RawEditMode;
    }
}


void PlotSettingsBaseWidget::emitValueChanged()
{
    if (currentValue != getValue())
    {
        currentValue = getValue();
        valueSet = true;
        qDebug() << "emitValueChanged: currentValue: " << currentValue << "value(): " << value();
        emit valueChanged();
    }


//    if (currentValue != value())
//    {
//        currentValue = value();
//        emit valueChanged();
//    }
}


//////////// PlotSettingsNumericWidget



PlotSettingsNumericWidget::PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}

void PlotSettingsNumericWidget::setWidgetValue(QString val)
{
    if (settingsElement["value"].isReal())
        static_cast<QDoubleSpinBox*>(editWidget)->setValue(val.toDouble());
    else
        static_cast<QSpinBox*>(editWidget)->setValue(val.toInt());
}

QString PlotSettingsNumericWidget::raw2widgetValue(QString val)
{
    return val;
}

QString PlotSettingsNumericWidget::widget2rawValue(QString val)
{
    return val;
}

void PlotSettingsNumericWidget::createEditWidget()
{
    if (settingsElement["value"].isReal())
    {
        editWidget = new QDoubleSpinBox;
        setWidgetValue(settingsElement["value"]());
//        connect(static_cast<QDoubleSpinBox*>(editWidget), SIGNAL(valueChanged(const QString&)),
//                this, SLOT(setValueSetTrue()));
        connect(static_cast<QDoubleSpinBox*>(editWidget), SIGNAL(editingFinished()),
                this, SLOT(emitValueChanged()));
    }
    else
    {
        editWidget = new QSpinBox;
        setWidgetValue(settingsElement["value"]());
//        connect(static_cast<QSpinBox*>(editWidget), SIGNAL(valueChanged(const QString&)),
//                this, SLOT(setValueSetTrue()));
        connect(static_cast<QSpinBox*>(editWidget), SIGNAL(editingFinished()),
                this, SLOT(emitValueChanged()));
    }
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
    editStackedLayout->addWidget(editWidget);
    widgetEditButton->setEnabled(true);
    rawEditButton->setEnabled(true);
    widgetEditButton->setChecked(true);
}

QString PlotSettingsNumericWidget::getWidgetValue()
{
    if (settingsElement["value"].isReal())
        return QString::number(static_cast<QDoubleSpinBox*>(editWidget)->value());
    else
        return QString::number(static_cast<QSpinBox*>(editWidget)->value());


}


//////////// PlotSettingsSingleChoiceWidget



PlotSettingsSingleChoiceWidget::PlotSettingsSingleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}


//void PlotSettingsDataframeWidget::createListEdit()
//{
//    factorList = new LazyNutListWidget(settingsElement["levels"](), this);
//    factorList->setSelectionMode(QAbstractItemView::SingleSelection);
//    connect(factorList, SIGNAL(selected(QString)),
//            static_cast<QLabel*>(valueEdit), SLOT(setText(QString)));
//    connect(factorList, SIGNAL(selected(QString)),
//            this, SIGNAL(valueChanged()));
//    QMainWindow *editWindow = new QMainWindow(this);
//    editWindow->setAttribute(Qt::WA_AlwaysShowToolTips);
//    QAction *hideAct = new QAction("Hide",editWindow);
//    connect(hideAct, SIGNAL(triggered()), editWindow, SLOT(hide()));
//    QToolBar *hideToolBar = editWindow->addToolBar("");
//    hideToolBar->addAction(hideAct);
//    QFrame *editBox = new QFrame(this);
//    editBox->setFrameShape(QFrame::Panel);
//    editBox->setFrameShadow(QFrame::Sunken);
//    QVBoxLayout *editBoxLayout = new QVBoxLayout;
//    editBoxLayout->addWidget(factorList);
//    editBox->setLayout(editBoxLayout);
//    editWindow->setCentralWidget(editBox);
//    editWindow->hide();
//    connect(editButton, SIGNAL(clicked()), editWindow, SLOT(show()));
//    extraWidgetsList.append(editWindow);
//}



void PlotSettingsSingleChoiceWidget::currentTextChangedFilter(QString value)
{
    if (!value.isEmpty() && value != settingsElement["value"]())
    {
        emit valueChanged();
        qDebug() << value << settingsElement["value"]();
    }
}

void PlotSettingsSingleChoiceWidget::createEditWidget()
{
    editWidget = new LazyNutListComboBox(settingsElement["levels"](), this);
    connect(static_cast<LazyNutListComboBox*>(editWidget),SIGNAL(listReady()),
            this, SLOT(setupEditWidget()));
    static_cast<LazyNutListComboBox*>(editWidget)->getList();
}

void PlotSettingsSingleChoiceWidget::setupEditWidget()
{
    if (!hasDefault())
        static_cast<LazyNutListComboBox*>(editWidget)->setEmptyItem(true);

    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();
//    connect(static_cast<LazyNutListComboBox*>(editWidget),SIGNAL(activated(int)),
//            this, SLOT(setValueSetTrue()));
    connect(static_cast<LazyNutListComboBox*>(editWidget),SIGNAL(activated(int)),
            this, SLOT(emitValueChanged()));
    editStackedLayout->addWidget(editWidget);
    widgetEditButton->setEnabled(true);
    rawEditButton->setEnabled(true);
    widgetEditButton->setChecked(true);

    valueSet = !settingsElement["value"]().isEmpty();
    currentValue = value();
}

QString PlotSettingsSingleChoiceWidget::getWidgetValue()
{
    return static_cast<QComboBox*>(editWidget)->currentText();
}

void PlotSettingsSingleChoiceWidget::setWidgetValue(QString val)
{
    static_cast<QComboBox*>(editWidget)->setCurrentIndex(
                static_cast<QComboBox*>(editWidget)->findText(val));
}

QString PlotSettingsSingleChoiceWidget::raw2widgetValue(QString val)
{
    if (settingsElement["type"]() == "factor")
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"|^\\s*NULL\\s*$")).simplified();

    return val;
}

QString PlotSettingsSingleChoiceWidget::widget2rawValue(QString val)
{
    if (settingsElement["type"]() == "factor")
    {
        QStringList list = val.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);
        return  QString("c(%1)").arg(list.replaceInStrings(QRegExp("^|$"),"\"").join(","));
    }
    return val;
}



//////////// PlotSettingsFactorWidget

/*

PlotSettingsFactorWidget::PlotSettingsFactorWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createValueEdit();
    createListEdit();
}

QString PlotSettingsFactorWidget::value()
{
    return formatFactorStringForR(static_cast<QLineEdit*>(valueEdit)->text().simplified());
}

void PlotSettingsFactorWidget::setFactorList(QStringList list)
{
    factorList->clear();
    factorList->addItems(list);
    updateListEdit();
}

void PlotSettingsFactorWidget::createValueEdit()
{
    delete valueEdit;
    valueEdit = new QLineEdit;
    static_cast<QLineEdit*>(valueEdit)->setText(formatFactorStringForDisplay(settingsElement["value"]()));
    static_cast<QLineEdit*>(valueEdit)->setReadOnly(true);
    connect(static_cast<QLineEdit*>(valueEdit), SIGNAL(textChanged(const QString&)),
            this, SIGNAL(valueChanged()));

    editButton = new QPushButton("Edit");

    mainLayout->addWidget(valueEdit);
    mainLayout->addWidget(editButton);

}

void PlotSettingsFactorWidget::createListEdit()
{
    factorList = new QListWidget(this);
    factorList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedList = new QListWidget(this);
    selectedList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    getLevels();

    QMainWindow *editWindow = new QMainWindow(this);
    editWindow->setAttribute(Qt::WA_AlwaysShowToolTips);
    QAction *hideAct = new QAction("Hide",editWindow);
    connect(hideAct, SIGNAL(triggered()), editWindow, SLOT(hide()));
    QToolBar *hideToolBar = editWindow->addToolBar("");
    hideToolBar->addAction(hideAct);
    QFrame *editBox = new QFrame(this);
    editBox->setFrameShape(QFrame::Panel);
    editBox->setFrameShadow(QFrame::Sunken);
    QGridLayout *editBoxLayout = new QGridLayout;
    QPushButton *addButton = new QPushButton("==>", this);
    addButton->setToolTip("Add to selected factors");
    connect(addButton, SIGNAL(clicked()), this, SLOT(addItems()));
    QPushButton *removeButton = new QPushButton("<==", this);
    removeButton->setToolTip("Remove from selected factors");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeItems()));
    QLabel *availableLabel = new QLabel("Available factors", editWindow);
    QLabel *selectedLabel = new QLabel("Selected factors", editWindow);
    editBoxLayout->addWidget(availableLabel, 0,0,Qt::AlignHCenter);
    editBoxLayout->addWidget(selectedLabel, 0,2,Qt::AlignHCenter);
    editBoxLayout->addWidget(factorList, 1,0,3,1);
    editBoxLayout->addWidget(selectedList, 1,2,3,1);
    editBoxLayout->addWidget(addButton,1,1);
    editBoxLayout->addWidget(removeButton,2,1);
    editBox->setLayout(editBoxLayout);
    editWindow->setCentralWidget(editBox);
    editWindow->hide();
    connect(editButton, SIGNAL(clicked()), editWindow, SLOT(show()));

    extraWidgetsList.append(editWindow);
}

void PlotSettingsFactorWidget::updateListEdit()
{
    QList<QListWidgetItem*> selectedItems;
    foreach(QString factor, factorString2list(settingsElement["value"]()))
    {
        if (!factor.isEmpty())
            selectedItems.append(factorList->findItems(factor, Qt::MatchFixedString|Qt::MatchCaseSensitive));
    }
    moveItems(factorList, selectedList, selectedItems);
}

void PlotSettingsFactorWidget::getLevels()
{
    QString cmd = settingsElement["levels"]();
    if (!cmd.isEmpty())
    {
        LazyNutJobParam *param = new LazyNutJobParam;
        param->logMode |= ECHO_INTERPRETER; // debug purpose
        param->cmdList = QStringList({QString("xml %1").arg(cmd)});
        param->answerFormatterType = AnswerFormatterType::ListOfValues;
        param->setAnswerReceiver(this, SLOT(setFactorList(QStringList)));
        SessionManager::instance()->setupJob(param, sender());
    }
}

void PlotSettingsFactorWidget::addItems()
{
    moveItems(factorList, selectedList, factorList->selectedItems());
}

void PlotSettingsFactorWidget::removeItems()
{
    moveItems(selectedList, factorList, selectedList->selectedItems());
}

QString PlotSettingsFactorWidget::formatFactorStringForR(QString factorString)
{
    // from: aaa, bbb, ccc
    // to: c("aaa","bbb","ccc")
    QStringList selectedList = factorString.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);
    if (selectedList.size() > 1)
        return QString("c(%1)").arg(selectedList.replaceInStrings(QRegExp("^|$"),"\"").join(","));
    else if (selectedList.size() == 1)
        return QString("\"%1\"").arg(selectedList.at(0));
    else
        return "\"\"";
}

QString PlotSettingsFactorWidget::formatFactorStringForDisplay(QString factorString)
{
    // from: c("aaa","bbb","ccc")
    // to: aaa, bbb, ccc
    return factorString.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"|^\\s*NULL\\s*$")).simplified();
}

QStringList PlotSettingsFactorWidget::factorString2list(QString factorString)
{
    return formatFactorStringForDisplay(factorString).split(", ");
}

void PlotSettingsFactorWidget::moveItems(QListWidget *fromList, QListWidget *toList, QList<QListWidgetItem*> items)
{
    foreach(QListWidgetItem* item, items)
        toList->addItem(fromList->takeItem(fromList->row(item)));

    fromList->sortItems();
    toList->sortItems();
    updateValueFromEdit();
}

void PlotSettingsFactorWidget::updateValueFromEdit()
{
    QStringList items;
    for(int row = 0; row < selectedList->count(); ++row)
        items.append(selectedList->item(row)->text());

    static_cast<QLineEdit*>(valueEdit)->setText(items.join(", "));
}


*/



PlotSettingsMultipleChoiceWidget::PlotSettingsMultipleChoiceWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createEditWidget();
}

QString PlotSettingsMultipleChoiceWidget::value()
{
    // this is a hack
    if (isValueSet())
    {
        if (settingsElement["type"]() == "dataframe")
            return getValue().replace(",", " ");

        return getValue();
    }
    return QString();
}

void PlotSettingsMultipleChoiceWidget::setupEditWidget()
{
    setWidgetValue(raw2widgetValue(settingsElement["value"]()));
    updateEditWidget();
    currentValue = settingsElement["value"]();
    valueSet = !settingsElement["value"]().isEmpty();

//    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
//            this, SLOT(setValueSetTrue()));
    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(emitValueChanged()));
    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(valueChanged()),
            this, SLOT(updateEditWidget()));
    editStackedLayout->addWidget(editWidget);
    widgetEditButton->setEnabled(true);
    rawEditButton->setEnabled(true);
    widgetEditButton->setChecked(true);


}

void PlotSettingsMultipleChoiceWidget::setRawMode(bool on)
{
    PlotSettingsBaseWidget::setRawMode(on);
    if (on)
        editExtraWidget->hide();
}

void PlotSettingsMultipleChoiceWidget::setWidgetMode(bool on)
{
    PlotSettingsBaseWidget::setWidgetMode(on);
    if (on)
        editExtraWidget->show();
}

void PlotSettingsMultipleChoiceWidget::updateEditWidget()
{
    static_cast<QLineEdit*>(editWidget)->setText(getWidgetValue());
}

void PlotSettingsMultipleChoiceWidget::resetDefault()
{
    PlotSettingsBaseWidget::resetDefault();
    updateEditWidget();
}

void PlotSettingsMultipleChoiceWidget::createEditWidget()
{
    editWidget = new QLineEdit();
    static_cast<QLineEdit*>(editWidget)->setReadOnly(true);

    editExtraWidget = new LazyNutPairedListWidget(settingsElement["levels"]());
    connect(static_cast<LazyNutPairedListWidget*>(editExtraWidget),SIGNAL(listReady()),
            this, SLOT(setupEditWidget()));
    static_cast<LazyNutPairedListWidget*>(editExtraWidget)->getList();
    extraWidgetsList.append(editExtraWidget);
    //editExtraWidget->show();
}

QString PlotSettingsMultipleChoiceWidget::getWidgetValue()
{
    return static_cast<LazyNutPairedListWidget*>(editExtraWidget)->getValue().join(", ");
}

void PlotSettingsMultipleChoiceWidget::setWidgetValue(QString val)
{
    static_cast<LazyNutPairedListWidget*>(editExtraWidget)->setValue(val.simplified().split(QRegExp("\\s*,\\s*")));
}

QString PlotSettingsMultipleChoiceWidget::raw2widgetValue(QString val)
{
    if (settingsElement["type"]() == "factor")
        return val.remove(QRegExp("^\\s*c\\(|\\)\\s*$|\"|^\\s*NULL\\s*$")).simplified();

    return val;
}

QString PlotSettingsMultipleChoiceWidget::widget2rawValue(QString val)
{
    if (settingsElement["type"]() == "factor")
    {
        QStringList list = val.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);
        return  QString("c(%1)").arg(list.replaceInStrings(QRegExp("^|$"),"\"").join(","));
    }
    return val;
}
