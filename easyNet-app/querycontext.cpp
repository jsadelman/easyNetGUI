#include "querycontext.h"
#include "treeitem.h"
#include "treemodel.h"
#include <vector>


QueryContext::QueryContext(TreeItem *rootItem) : begin_query(false), root(rootItem)

{
    if (root == nullptr)
    {
        QVector<QVariant> rootData;
        rootData << QString("Query tree root node");
        root = new TreeItem(rootData);
    }
}

QueryContext::~QueryContext()
{
    delete root;
}

bool QueryContext::clearQueries()
{
    return root->removeChildren(0,root->childCount());
}

