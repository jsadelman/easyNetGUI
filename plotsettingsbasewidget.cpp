#include <QtWidgets>

#include "plotsettingsbasewidget.h"



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




PlotSettingsBaseWidget::PlotSettingsBaseWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
    :  name(name), value(value), comment(comment), defaultValue(defaultValue), QWidget(parent)
{
    createDisplay();
}



void PlotSettingsBaseWidget::createDisplay()
{
    displayLayout = new QHBoxLayout;
    nameButton = new QPushButton(name, this);
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
    addValueEdit();
    setLayout(displayLayout);
    setAttribute(Qt::WA_AlwaysShowToolTips);
}

void PlotSettingsBaseWidget::addValueEdit()
{
    valueEdit = new QLineEdit;
    static_cast<QLineEdit*>(valueEdit)->setText(value.isEmpty() ? defaultValue : value);
    displayLayout->addWidget(valueEdit);
}

void PlotSettingsBaseWidget::displayComment()
{
    QMessageBox::information(this, "Setting description", comment);
}

QString PlotSettingsBaseWidget::getValue()
{
    return static_cast<QLineEdit*>(valueEdit)->text().simplified();
}

//////////// PlotSettingsNumericWidget


PlotSettingsNumericWidget::PlotSettingsNumericWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
    : PlotSettingsBaseWidget(name, value, comment, defaultValue, parent)
{
    addValueEdit();
}

QString PlotSettingsNumericWidget::getValue()
{
    if (defaultValue.contains("."))
        return QString::number(static_cast<QDoubleSpinBox*>(valueEdit)->value());

    else
        return QString::number(static_cast<QSpinBox*>(valueEdit)->value());
}

void PlotSettingsNumericWidget::addValueEdit()
{
    delete valueEdit;
    if (defaultValue.contains("."))
    {
        valueEdit = new QDoubleSpinBox;
        static_cast<QDoubleSpinBox*>(valueEdit)->setValue(value.isEmpty() ? defaultValue.toDouble() : value.toDouble());
    }
    else
    {
        valueEdit = new QSpinBox;
        static_cast<QSpinBox*>(valueEdit)->setValue(value.isEmpty() ? defaultValue.toInt() : value.toInt());
    }
    displayLayout->addWidget(valueEdit);
}

//////////// PlotSettingsFactorWidget


PlotSettingsFactorWidget::PlotSettingsFactorWidget(QString name, QString value, QString comment, QString defaultValue, QWidget *parent)
    : PlotSettingsBaseWidget(name, value, comment, defaultValue, parent)
{
    addValueEdit();
}

QString PlotSettingsFactorWidget::getValue()
{
    return formatFactorStringForR(static_cast<QLineEdit*>(valueEdit)->text().simplified());
}

void PlotSettingsFactorWidget::addValueEdit()
{
    delete valueEdit;
    valueEdit = new ValueEdit;
    static_cast<ValueEdit*>(valueEdit)->setText(
                formatFactorStringForDisplay(value).isEmpty() ?
                formatFactorStringForDisplay(defaultValue) :
                formatFactorStringForDisplay(value));
    valueEdit->setToolTip("double-click to open edit window");
    displayLayout->addWidget(valueEdit);
}

void PlotSettingsFactorWidget::createlistEdit(QStringList list)
{
    factorList = new QListWidget(this);
    factorList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedList = new QListWidget(this);
    selectedList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    factorList->addItems(list);

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
    connect(static_cast<ValueEdit*>(valueEdit), SIGNAL(doubleClicked()),
            editWindow, SLOT(show()));
    emit addWidget(editWindow);
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
    // or from: aaa bbb ccc
    // to: c("aaa","bbb","ccc")
    QStringList selectedList = factorString.split(QRegExp("\\s|\\s*(,|;)\\s*"), QString::SkipEmptyParts);
    return QString("c(%1)").arg(selectedList.replaceInStrings(QRegExp("\\b"),"\"").join(","));
}

QString PlotSettingsFactorWidget::formatFactorStringForDisplay(QString factorString)
{
    // from: c("aaa","bbb","ccc")
    // to: aaa, bbb, ccc
    return factorString.remove(QRegExp("c\\(|\\)|\"|NULL")).simplified();
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



