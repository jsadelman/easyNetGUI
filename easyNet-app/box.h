#ifndef BOX_H
#define BOX_H

#include <QGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QDomDocument>
#include <QSet>
#include <QString>
#include <QAction>
#include <QFont>

class ObjectCacheFilter;
class ObjectUpdater;


class Box: public QGraphicsPolygonItem, public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName)
    Q_PROPERTY (QString lazyNutType READ lazyNutType WRITE setLazyNutType NOTIFY lazyNutTypeChanged)
    // graphical parameters
    Q_PROPERTY(int labelPointSize READ labelPointSize WRITE setLabelPointSize)
    Q_PROPERTY (qreal widthMarginProportionToLongestLabel READ widthMarginProportionToLongestLabel WRITE setWidthMarginProportionToLongestLabel)
    Q_PROPERTY (QString longNameToDisplayIntact READ longNameToDisplayIntact WRITE setLongNameToDisplayIntact)
    Q_PROPERTY (qreal widthOverHeight READ widthOverHeight WRITE setWidthOverHeight)

public:
    Box();
    virtual QString name(void) const {return m_name;}
    virtual void setName(const QString& name) {m_name = name;}
    virtual QString lazyNutType(void) const {return m_lazyNutType;}
    virtual void setLazyNutType(const QString& lazyNutType);
    virtual int labelPointSize(void) const {return m_labelPointSize;}
    virtual void setLabel(const QString& label);

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
    virtual void paintLabel(QPainter *painter);
    virtual QRectF labelBoundingRect(void) const;
    void setCentrePos(const QPointF&p){m_centrePos=p;}
    QPointF centrePos()const{return m_centrePos;}
    QStringList defaultPlotTypes();
    qreal height(){return m_height;}
    qreal width(){return m_width;}
    qreal autoWidth();
    QSet<Box *> neighbours();

signals:
    void createDataViewRequested(QString, QString, QString, QMap<QString, QString>, bool);
    void lazyNutTypeChanged();
    void plotDestroyed(QString name);


protected:
    virtual QAction *buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);

private slots:
//    void sendCreateNewPlotOfType();
    void setupDefaultObserverFilter();
    void cacheFromDescription(QDomDocument* description, QString name);

//    void updateObservedState();

private:
    void enableObserver(QString observer, bool enable);
    void lesionBox(bool lesion);


    QString m_name;
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
    ObjectCacheFilter *plotFilter;
    QMap <QString, QString> observerOfPlot;

    ObjectCacheFilter *defaultDataframesFilter;
//    ObjectCacheFilter   *layerFilter;
//    ObjectUpdater       *layerUpdater;

    QMap <QString, QString> m_ports;
    QRegExp default_input_observer_Rex;
    QStringList m_defaultPlotTypes;
    QString m_layerTransfer;


    QPointF m_centrePos;
    qreal m_height,m_width;

};

extern const QColor layerCol;
extern const QColor representationCol;
extern const QColor observedCol;

#endif // BOX_H
