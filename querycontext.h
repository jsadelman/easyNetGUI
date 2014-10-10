#ifndef QUERYCONTEXT_H
#define QUERYCONTEXT_H

#include <vector>
#include <QWidget>


// fwd declaration
//class ParseNode;
class TreeItem;
class TreeModel;

class QueryContext
{
public:
    //QueryContext(TreeModel* treeModelPtr);
    QueryContext(TreeItem* rootItem=nullptr);
    ~QueryContext();
    bool clearQueries();
    // public data
    TreeItem*   root; // the root of the query tree
    bool        begin_query; // are we between "BEGIN: query" and "END: query"?
    //TreeModel*  treeModelPtr;
};

#endif // QUERYCONTEXT_H
