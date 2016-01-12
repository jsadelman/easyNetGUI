#ifndef PRETTYHEADERSMODEL_H
#define PRETTYHEADERSMODEL_H

#include <QIdentityProxyModel>
#include <QMap>

class QTableView;

class PrettyHeadersModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    PrettyHeadersModel(QObject *parent=0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    void addHeaderReplaceRules(Qt::Orientation orientation, QString from, QString to);
    void setHeadeReplaceRules(QMap<Qt::Orientation , QList<QPair<QString, QString> > > rules);
    QString name();
    QTableView *view();
    bool hasHeaderReplaceRules() {return !headerReplaceRules.isEmpty();}
    QMap<Qt::Orientation , QList<QPair<QString, QString> > > getHeaderReplace() {return headerReplaceRules;}

private:
    QMap<Qt::Orientation , QList<QPair<QString, QString> > > headerReplaceRules;
};

#endif // PRETTYHEADERSMODEL_H
