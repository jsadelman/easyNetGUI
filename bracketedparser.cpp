#include "bracketedparser.h"


QStringList BracketedParser::parse(QString text)
{
    // from text = "badger (snake badger) (badger (mushroom)) "
    // return ("badger", "(snake badger)", "(badger (mushroom))")
    // or empty list if unmatched () found.

    QStringList tokens;
    int level = 0;
    QString token;
    for (QString::const_iterator i = text.begin(); i != text.end(); ++i)
    {
        if ((*i) == ' ')
        {
            if (!token.isEmpty())
            {
                if (level == 0)
                {
                    tokens.append(token);
                    token.clear();
                }
                else
                    token.append(*i);
            }
        }
        else if (*i == '(')
        {
            ++level;
            token.append(*i);
        }
        else if (*i == ')')
        {
            --level;
            if (level < 0)
                break;
            token.append(*i);
        }
        else
        {
            token.append(*i);
        }
    }
    if (!token.isEmpty())
        tokens.append(token);
    if (level !=0)
        tokens.clear();

    return tokens;
}
