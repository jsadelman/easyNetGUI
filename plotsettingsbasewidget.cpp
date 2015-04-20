#include <QtWidgets>

#include "plotsettingsbasewidget.h"
#include "lazynutjobparam.h"
#include "sessionmanager.h"
#include "lazynutlistwidget.h"
#include "lazynutlistcombobox.h"

ValueEdit::ValueEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

void ValueEdit::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;
    emit doubleClicked();
}




//PlotSettingsBaseWidget::PlotSettingsBaseWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
//    :  name(name), value(value), comment(comment), defaultValue(defaultValue), QWidget(parent)
//{
//    createDisplay();
//}

PlotSettingsBaseWidget::PlotSettingsBaseWidget(XMLelement settingsElement, QWidget *parent)
    :settingsElement(settingsElement), QWidget(parent)
{
    createDisplay();
}



void PlotSettingsBaseWidget::createDisplay()
{
    displayLayout = new QHBoxLayout;
    nameButton = new QPushButton(getName(), this);
    nameButton->setFlat(true);
    nameButton->setToolTip("Click for info");
    // set max size to fit text
    // http://stackoverflow.com/questions/6639012/minimum-size-width-of-a-qpushbutton-that-is-created-from-code
    QSize textSize = nameButton->fontMetrics().size(Qt::TextShowMnemonic, nameButton->text());
    QStyleOptionButton opt;
    opt.initFrom(nameButton);
    opt.rect.setSize(textSize);
    nameButton->setMaximumSize(nameButton->style()->sizeFromContents(QStyle::CT_PushButton,
                                        &opt, textSize, nameButton));

    connect(nameButton, SIGNAL(clicked()), this, SLOT(displayComment()));
    displayLayout->addWidget(nameButton);
    displayLayout->addStretch();
    createValueEdit();
    setLayout(displayLayout);
    setAttribute(Qt::WA_AlwaysShowToolTips);
}

void PlotSettingsBaseWidget::createValueEdit()
{
    valueEdit = new QLineEdit;
    static_cast<QLineEdit*>(valueEdit)->setText(settingsElement["value"]());
    connect(static_cast<QLineEdit*>(valueEdit), SIGNAL(textChanged(const QString&)),
            this, SIGNAL(valueChanged()));
    displayLayout->addWidget(valueEdit);
}

void PlotSettingsBaseWidget::displayComment()
{
    QMessageBox::information(this, "Setting description", settingsElement["comment"]());
}

QString PlotSettingsBaseWidget::getValue()
{
    return static_cast<QLineEdit*>(valueEdit)->text().simplified();
}

//////////// PlotSettingsNumericWidget


//PlotSettingsNumericWidget::PlotSettingsNumericWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
//    : PlotSettingsBaseWidget(name, value, comment, defaultValue, parent)
//{
//    createValueEdit();
//}

PlotSettingsNumericWidget::PlotSettingsNumericWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createValueEdit();
}

QString PlotSettingsNumericWidget::getValue()
{
//    if (defaultValue.contains("."))
    if (settingsElement["value"].isReal())
        return QString::number(static_cast<QDoubleSpinBox*>(valueEdit)->value());

    else // isInteger()
        return QString::number(static_cast<QSpinBox*>(valueEdit)->value());
}

void PlotSettingsNumericWidget::createValueEdit()
{
    delete valueEdit;
//    if (defaultValue.contains("."))
    if (settingsElement["value"].isReal())
    {
        valueEdit = new QDoubleSpinBox;
        static_cast<QDoubleSpinBox*>(valueEdit)->setValue(settingsElement["value"]().toDouble());
        connect(static_cast<QDoubleSpinBox*>(valueEdit), SIGNAL(valueChanged(const QString&)),
                this, SIGNAL(valueChanged()));
    }
    else
    {
        valueEdit = new QSpinBox;
        static_cast<QSpinBox*>(valueEdit)->setValue(settingsElement["value"]().toInt());
        connect(static_cast<QSpinBox*>(valueEdit), SIGNAL(valueChanged(const QString&)),
                this, SIGNAL(valueChanged()));
    }
    displayLayout->addWidget(valueEdit);
}

//////////// PlotSettingsDataframeWidget


PlotSettingsDataframeWidget::PlotSettingsDataframeWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createValueEdit();
    createListEdit();
}

QString PlotSettingsDataframeWidget::getValue()
{
    return static_cast<QLineEdit*>(valueEdit)->text().simplified();
//    return static_cast<QLabel*>(valueEdit)->text();
//    return static_cast<LazyNutListWidget*>(valueEdit)->currentValue();

}

void PlotSettingsDataframeWidget::createValueEdit()
{
    delete valueEdit;
//    valueEdit = new QLabel(settingsElement["value"]());
    valueEdit = new QLineEdit;
    static_cast<QLineEdit*>(valueEdit)->setText(settingsElement["value"]());
    connect(static_cast<QLineEdit*>(valueEdit), SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    editButton = new QPushButton("Edit");




//    valueEdit = new LazyNutListComboBox(settingsElement["levels"]());
//    static_cast<LazyNutListComboBox*>(valueEdit)->setCurrentText(settingsElement["value"]());
//    connect(static_cast<LazyNutListComboBox*>(valueEdit), SIGNAL(currentTextChanged(QString)),
//            this, SLOT(currentTextChangedFilter(QString)));

//    valueEdit = new LazyNutListWidget(settingsElement["levels"]());
//    static_cast<LazyNutListWidget*>(valueEdit)->setCurrentValue(settingsElement["value"]());
//    connect(static_cast<LazyNutListWidget*>(valueEdit), SIGNAL(itemSelectionChanged()),
//            this, SIGNAL(valueChanged()));
    displayLayout->addWidget(valueEdit);
    displayLayout->addWidget(editButton);
}

void PlotSettingsDataframeWidget::createListEdit()
{
    factorList = new LazyNutListWidget(settingsElement["levels"](), this);
    factorList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(factorList, SIGNAL(selected(QString)),
            static_cast<QLabel*>(valueEdit), SLOT(setText(QString)));
    connect(factorList, SIGNAL(selected(QString)),
            this, SIGNAL(valueChanged()));
    QMainWindow *editWindow = new QMainWindow(this);
    editWindow->setAttribute(Qt::WA_AlwaysShowToolTips);
    QAction *hideAct = new QAction("Hide",editWindow);
    connect(hideAct, SIGNAL(triggered()), editWindow, SLOT(hide()));
    QToolBar *hideToolBar = editWindow->addToolBar("");
    hideToolBar->addAction(hideAct);
    QFrame *editBox = new QFrame(this);
    editBox->setFrameShape(QFrame::Panel);
    editBox->setFrameShadow(QFrame::Sunken);
    QVBoxLayout *editBoxLayout = new QVBoxLayout;
    editBoxLayout->addWidget(factorList);
    editBox->setLayout(editBoxLayout);
    editWindow->setCentralWidget(editBox);
    editWindow->hide();
    connect(editButton, SIGNAL(clicked()), editWindow, SLOT(show()));
    extraWidgetsList.append(editWindow);
}



void PlotSettingsDataframeWidget::currentTextChangedFilter(QString value)
{
    if (!value.isEmpty() && value != settingsElement["value"]())
    {
        emit valueChanged();
        qDebug() << value << settingsElement["value"]();
    }
}



//////////// PlotSettingsFactorWidget


//PlotSettingsFactorWidget::PlotSettingsFactorWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
//    : PlotSettingsBaseWidget(name, value, comment, defaultValue, parent)
//{
//    createValueEdit();
//}

PlotSettingsFactorWidget::PlotSettingsFactorWidget(XMLelement settingsElement, QWidget *parent)
    : PlotSettingsBaseWidget(settingsElement, parent)
{
    createValueEdit();
    createListEdit();
}

QString PlotSettingsFactorWidget::getValue()
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
//    valueEdit->setToolTip("double-click to open edit window");
    connect(static_cast<QLineEdit*>(valueEdit), SIGNAL(textChanged(const QString&)),
            this, SIGNAL(valueChanged()));

    editButton = new QPushButton("Edit");

    displayLayout->addWidget(valueEdit);
    displayLayout->addWidget(editButton);

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
//    connect(static_cast<ValueEdit*>(valueEdit), SIGNAL(doubleClicked()),
//            editWindow, SLOT(show()));
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
    return QString("c(%1)").arg(selectedList.replaceInStrings(QRegExp("\\b"),"\"").join(","));
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




