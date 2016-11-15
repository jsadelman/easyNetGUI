#ifndef BOX_H
#define BOX_H

#include <QGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QDomDocument>
#include <QSet>
#include <QString>
#include <QAction>
#include <QFont>
#include <QtCore>
#include "diagramitem.h"
class ObjectCacheFilter;
class ObjectUpdater;
QString limitString(const QString& aString, int maxLength);


inline QString limitString(const QString& aString, int maxLength) {
    static const QString ELLIPSIS("..");

    if (aString.length() <= maxLength)
        return aString;


    qreal spacePerPart = (maxLength - ELLIPSIS.length()) / 2.0;
    QString beforeEllipsis = aString.left(qCeil(spacePerPart));
    QString afterEllipsis = aString.right(qFloor(spacePerPart));

    return beforeEllipsis + ELLIPSIS + afterEllipsis;
}

class Box: public QObject, public DiagramItem//QGraphicsPolygonItem
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName)
    Q_PROPERTY (QString colour READ colour WRITE setColour)
    Q_PROPERTY (QString lazyNutType READ lazyNutType WRITE setLazyNutType NOTIFY lazyNutTypeChanged)
    // graphical parameters
    Q_PROPERTY(int labelPointSize READ labelPointSize WRITE setLabelPointSize)
    Q_PROPERTY (qreal widthMarginProportionToLongestLabel READ widthMarginProportionToLongestLabel WRITE setWidthMarginProportionToLongestLabel)
    Q_PROPERTY (QString longNameToDisplayIntact READ longNameToDisplayIntact WRITE setLongNameToDisplayIntact)
    Q_PROPERTY (qreal widthOverHeight READ widthOverHeight WRITE setWidthOverHeight)

public:
    Box();
    virtual QString name(void) const {return m_name;}
    virtual void setName(const QString& name) {m_name = name;/* setLabel();*/}
    virtual QString colour(void) const {return m_colour;}
    virtual void setColour(const QString& colour);
    virtual QString lazyNutType(void) const {return m_lazyNutType;}
    virtual void setLazyNutType(const QString& lazyNutType);
    virtual int labelPointSize(void) const {return m_labelPointSize;}
    virtual void setLabel(QString label="");

    virtual void setLabelPointSize(int labelPointSize);
    virtual qreal widthMarginProportionToLongestLabel() {return m_widthMarginProportionToLongestLabel;}
    virtual void setWidthMarginProportionToLongestLabel(qreal w) {m_widthMarginProportionToLongestLabel = w;}
    virtual QString longNameToDisplayIntact() {return m_longNameToDisplayIntact;}
    virtual void setLongNameToDisplayIntact(QString s) {m_longNameToDisplayIntact = s;}
    virtual qreal widthOverHeight() {return m_widthOverHeight;}
    virtual void setWidthOverHeight(qreal r) {m_widthOverHeight = r;}

    void setPorts(QMap <QString, QString> ports) {m_ports = ports;}
    void read(const QJsonObject &json, qreal boxWidth=0);
    void write(QJsonObject &json) const;
    void autoSize();
    virtual void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void paintLabel(QPainter *painter);
    virtual QRectF labelBoundingRect(void) const;
    void setCentrePos(const QPointF&p)
    {
        auto x=p-center();
        moveBy(x.x(),x.y());
//        myCenter=p;
    }
    QPointF centrePos()const{return center();}
    qreal height()const{return size().height();}
    qreal width()const{return size().width();}
    qreal autoWidth();
    void autoFontSize();
    void setFillColour(QColor);
    void setDashedStroke(bool);
    void cmd_setSize(QSizeF f){myheight=f.height();mywidth=f.width(); setGeometry();}
    QSizeF size()const{return QSizeF(mywidth,myheight);}

public slots:
    void disableAllObservers();

signals:
    void createDataViewRequested(QString, QString, QString, QMap<QString, QString>, bool);
    void lazyNutTypeChanged();
    void plotDestroyed(QString name);
    void propertiesRequested();
    void focusOnPlotRequested(QString name);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);


private slots:
    void setupDefaultObserverFilter();

private:
    void enableObserver(QString observer, bool enable);
    void setLesioned(bool lesion);
    QStringList dataViewTypesPath(QString dataView, QString port = "");
    QString layerTransfer();
    bool anyEnabled;

    QString m_name,m_label,m_colour;
    QColor m_qColor;
    QString m_lazyNutType;
    int m_labelPointSize;
    QFont labelFont;
    QString m_longNameToDisplayIntact;
    qreal m_widthMarginProportionToLongestLabel;
    qreal m_widthOverHeight;

    ObjectCacheFilter *defaultObserverFilter;
    ObjectUpdater     *defaultObserverUpdater;
    QMap<QString, bool> defaultObservers; // <name, enabled>
    QSet<QString>     enabledObserverSet;
    ObjectCacheFilter *dataViewFilter;
    QMap <QString, QString> observerOfDataView;
    ObjectCacheFilter *defaultDataframesFilter;
    QMap <QString, QString> m_ports;
    QRegExp default_input_observer_Rex;
};

extern const QColor layerCol;
extern const QColor representationCol;
extern const QColor observedCol;

#endif // BOX_H
