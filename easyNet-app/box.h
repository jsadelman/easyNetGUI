#ifndef BOX_H
#define BOX_H

#include "libdunnartcanvas/shape.h"

class ObjectCatalogueFilter;


class Box: public dunnart::ShapeObj
{
    Q_OBJECT
    Q_PROPERTY (QString name READ name WRITE setName)
    Q_PROPERTY (QString lazyNutType READ lazyNutType WRITE setLazyNutType NOTIFY lazyNutTypeChanged)
    // graphical parameters
    Q_PROPERTY (qreal widthMarginProportionToLongestLabel READ widthMarginProportionToLongestLabel WRITE setWidthMarginProportionToLongestLabel)
    Q_PROPERTY (QString longNameToDisplayIntact READ longNameToDisplayIntact WRITE setLongNameToDisplayIntact)
    Q_PROPERTY (qreal widthOverHeight READ widthOverHeight WRITE setWidthOverHeight)

public:
    Box();
    virtual QString name(void) const {return m_name;}
    virtual void setName(const QString& name) {m_name = name;}
    virtual QString lazyNutType(void) const {return m_lazyNutType;}
    virtual void setLazyNutType(const QString& lazyNutType);
    virtual qreal widthMarginProportionToLongestLabel() {return m_widthMarginProportionToLongestLabel;}
    virtual void setWidthMarginProportionToLongestLabel(qreal w) {m_widthMarginProportionToLongestLabel = w;}
    virtual QString longNameToDisplayIntact() {return m_longNameToDisplayIntact;}
    virtual void setLongNameToDisplayIntact(QString s) {m_longNameToDisplayIntact = s;}
    virtual qreal widthOverHeight() {return m_widthOverHeight;}
    virtual void setWidthOverHeight(qreal r) {m_widthOverHeight = r;}


    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    void autoSize();
    virtual void paintLabel(QPainter *painter);
    virtual QRectF labelBoundingRect(void) const;

    QString defaultPlotType();

signals:
    void createNewPlotOfType(QString, QString, QMap<QString, QString>);
    void lazyNutTypeChanged();
    void plotDestroyed(QString name);


protected:
    virtual QAction *buildAndExecContextMenu(
            QGraphicsSceneMouseEvent *event, QMenu& menu);

private slots:
//    void sendCreateNewPlotOfType();
    void setupDefaultDataframesFilter();

private:
    void defaultPlot(QString plotName, QString dataframe);
    void enableObserver(QString observer);
    void disableObserver(QString observer);

    QString m_name;
    QString m_lazyNutType;
    QString m_longNameToDisplayIntact;
    qreal m_widthMarginProportionToLongestLabel;
    qreal m_widthOverHeight;

    ObjectCatalogueFilter *defaultDataframesFilter;




};

extern const QColor layerCol;
extern const QColor representationCol;
extern const QColor observedCol;

#endif // BOX_H
