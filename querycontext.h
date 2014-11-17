#ifndef QUERYCONTEXT_H
#define QUERYCONTEXT_H

#include <vector>
#include <QWidget>


// fwd declaration
class TreeItem;
class TreeModel;

class QueryContext
{
public:
    QueryContext(TreeItem* rootItem=nullptr);
    ~QueryContext();
    bool clearQueries();
    // public data
    TreeItem*   root; // the root of the query tree
    bool        begin_query; // are we between "BEGIN: query" and "END: query"?
};

#endif // QUERYCONTEXT_H
