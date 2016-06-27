#ifndef ENUMCLASSES_H
#define ENUMCLASSES_H

#include <Qt>
#include <QHash>
#include <QMap>
#include <QRegExp>
#include <QDebug>

class AsLazyNutObject;
typedef QHash<QString,AsLazyNutObject*> LazyNutObjectCatalogue;

// model/view data roles
enum : unsigned int {ExpandToFillRole = Qt::UserRole};
enum : unsigned int {NameRole = Qt::UserRole, PrettyNameRole};

enum : unsigned int {ViewState_Static = 0, ViewState_Stale, ViewState_Fresh};


// flags for logMode to be set up in LazyNutJobParam
enum : unsigned int
{
    ECHO_INTERPRETER    = 0x01,
};

enum class AnswerFormatterType
{
    NotInitialised,
    Identity,
    XML,
    ListOfValues,
    SVG
};

enum {Dispatch_New=0, Dispatch_Overwrite, Dispatch_Append, MAX_DISPATCH_MODE};


enum : unsigned int
{
    Plot_AnyTrial           = 0x01,
    Plot_Backup             = 0x02
};

enum class MouseState {Normal, Hovered, Pressed};

enum {TrialRunMode_Single=0, TrialRunMode_List};
static QMap<int, QString> trialRunModeName
{
    {TrialRunMode_Single, "single trial"},
    {TrialRunMode_List, "list trial"}
};


#ifdef __APPLE__
  #define EN_FONT "Helvetica Neue"
#else
  #define EN_FONT "Georgia"
#endif

#ifdef __APPLE__
    #define EN_FONT_SMALL 12
#else
    #define EN_FONT_SMALL 10
#endif

#ifdef __APPLE__
    #define EN_FONT_MEDIUM 14
#else
    #define EN_FONT_MEDIUM 12
#endif

#ifdef __APPLE__
    #define EN_FONT_LARGE 18
#else
    #define EN_FONT_LARGE 16
#endif

#define eNerror qDebug() << "ERROR:" << Q_FUNC_INFO
#define eNwarning qDebug() << "WARNING:" << Q_FUNC_INFO

template <typename T>
static void matchListFromMap(QMap<T, T> map, T k, QSet<T>& set)
{
    if (!map.contains(k))
        return;
    set.insert(k);
    foreach (T v, map.values(k))
    {
        if (!set.contains(v))
        {
            set.insert(v);
            matchListFromMap<T>(map, v, set);
        }
    }
}

template <typename T>
static QSet<T> matchListFromMap(QMap<T, T> map, T k)
// works on QMap and QMultiMap, must have key and val of same type
// returns a list of values present in either side starting from k, then values matching k,
// then using those values recursively as keys. Avoids infinite loops. E.g.:
// map.insert("a","b");
// map.insert("b","c");
// map.insert("c","c");
// QString k = "b";
// matchListFromMap(map, k);
// returns ("c", "b")
// QString k = "a";
// matchListFromMap(map, k);
// returns ("c", "a", "b")
{
    QSet<T> set;
    matchListFromMap(map, k, set);
    return set;
}



template <typename K, typename V>
static QSet<K> allKeysOfValue(QMap<K, V> map, V v)
{
    QSet<K> set;
    foreach (K k, map.keys())
    {
        if (map.values(k).contains(v))
            set.insert(k);
    }
    return set;
}

static QString normalisedName(QString name)
{
    name.replace(QRegExp("[()]"), "");
    name = name.simplified();
    name.replace(" ", "_");
    return name;
}

#endif // ENUMCLASSES_H
