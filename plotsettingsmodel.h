#ifndef PLOTSETTINGSMODEL_H
#define PLOTSETTINGSMODEL_H

#include <QAbstractListModel>
class QDomDocument;

class PlotSettingsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PlotSettingsModel(QDomDocument *plotSettingsXML, QObject *parent = 0);
    virtual ~PlotSettingsModel();
    void setXML(QDomDocument* domDoc);
    int	rowCount(const QModelIndex & = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    enum {
        PlotSettingsNameRole = Qt::UserRole,
        PlotSettingsTypeRole,
        PlotSettingsCommentRole,
        PlotSettingsDefaultRole
         };

signals:

public slots:

private:
    QDomDocument *plotSettingsXML;
    QMap<int,QString> rolesMap;

};

#endif // PLOTSETTINGSMODEL_H
