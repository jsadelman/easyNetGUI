#include "querycontext.h"
//#include "parsenode.h"
#include "treeitem.h"
#include "treemodel.h"
#include <vector>


//QueryContext::QueryContext(TreeModel *treeModelPtr) : treeModelPtr(treeModelPtr), begin_query(false)
QueryContext::QueryContext(TreeItem *rootItem) : begin_query(false), root(rootItem)

{
    if (root == nullptr)
    {
        QVector<QVariant> rootData;
        rootData << QString("Query tree root node");
        root = new TreeItem(rootData);
    }
    //root = treeModelPtr->getRootItem();
}

QueryContext::~QueryContext()
{
    delete root;
}

bool QueryContext::clearQueries()
{
    return root->removeChildren(0,root->childCount());
}

