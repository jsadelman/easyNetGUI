#include "maxminpanel.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QPoint>
#include <QHBoxLayout>
#include <QMouseEvent>

class Titlebar : public QWidget
{
    private:
        QWidget *parent;
        QPoint cursor;

        QSize sizeHint() const;


    public:
        Titlebar( QWidget *parent ) : parent( parent )
        {
            QLabel *label = new QLabel( parent->windowTitle() );
            QPushButton *buttonMinimise = new QPushButton(QIcon(":/images/minimize.png"),
                                                          tr(""), this);
            QPushButton *buttonMaximise = new QPushButton(QIcon(":/images/maximizeSelected.png"),
                                                          tr(""), this);

            QHBoxLayout *layout = new QHBoxLayout( this );
                layout->addWidget( label, 1 );
                layout->addWidget( buttonMinimise );
                layout->addWidget( buttonMaximise );

            connect( buttonMinimise, SIGNAL( clicked() ), parent, SLOT( minimiseCodePanel() ) );
//            connect( buttonMaximise, SIGNAL( clicked() ), parent, SLOT( maximiseCodePanel() ) );
        }

    protected:
        void mousePressEvent( QMouseEvent *event )
        {
            if( event->button() == Qt::LeftButton )
                cursor = event->globalPos() - geometry().topLeft();
        }

        void mouseMoveEvent( QMouseEvent *event )
        {
            if( event->buttons() & Qt::LeftButton )
                parent->move( event->globalPos() - cursor );
        }
};

QSize Titlebar::sizeHint() const
{
   return QSize(200, 40);
}


MaxMinPanel::MaxMinPanel(QString title, QWidget* parent)
{
    panelMinimised = false;
    setWindowTitle(title);
    setTitleBarWidget(new Titlebar(this));

}

MaxMinPanel::~MaxMinPanel()
{

}

void MaxMinPanel::minimiseCodePanel()
{
    if (panelMinimised)
//        setFixedHeight(userHeight);
        resize(width(),userHeight);
//          height=(userHeight);
    else
    {
        userHeight = height();
//        setFixedHeight(30);
        resize(width(),30);
    }
    panelMinimised = !panelMinimised;

}


