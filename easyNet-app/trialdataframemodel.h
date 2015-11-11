#ifndef TRIALDATAFRAMEMODEL_H
#define TRIALDATAFRAMEMODEL_H

#include <QIdentityProxyModel>
#include <QMap>

class QTableView;

class TrialDataFrameModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    TrialDataFrameModel(QObject *parent=0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    void addHeaderReplace(Qt::Orientation orientation, QString from, QString to);
    void addHeaderReplace(Qt::Orientation orientation, QList<QPair<QString, QString> > replaceList);
    QString name();
    QTableView *view();
    QMap<Qt::Orientation , QList<QPair<QString, QString> > > getHeaderReplace() {return headerReplace;}

private:
    QMap<Qt::Orientation , QList<QPair<QString, QString> > > headerReplace;
};

#endif // TRIALDATAFRAMEMODEL_H
