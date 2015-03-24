#ifndef ANSWERFORMATTER_H
#define ANSWERFORMATTER_H


#include <QString>
#include <QStringList>
#include <QObject>


class QDomDocument;

class AnswerFormatter: public QObject
{
    Q_OBJECT
public slots:
    virtual void formatAnswer(QString answer) = 0;

signals:
    void formattedAnswer(QString);
    void formattedAnswer(QDomDocument*);
    void formattedAnswer(QStringList);
    void formattedAnswer(QByteArray);

protected:
    AnswerFormatter(){}
};

class IdentityFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
//    void formatAnswer(QString answer) {emit formattedAnswer(QString("identity %1").arg(answer));} // test
    void formatAnswer(QString answer) {emit formattedAnswer(answer);}

    friend class AnswerFormatterFactory;

protected:
    IdentityFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QString)),answerRecipient,answerProcessor);
    }

};

class XMLFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
    void formatAnswer(QString answer);

    friend class AnswerFormatterFactory;

protected:
    XMLFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QDomDocument*)),answerRecipient,answerProcessor);
    }
};

class ListOfValuesFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
    void formatAnswer(QString answer);

    friend class AnswerFormatterFactory;

protected:
    ListOfValuesFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QStringList)),answerRecipient,answerProcessor);
    }
};

class SVGFormatter: public AnswerFormatter
        {
            Q_OBJECT
        public:
            void formatAnswer(QString answer);

            friend class AnswerFormatterFactory;

        protected:
            SVGFormatter(QObject *answerRecipient, char const *answerProcessor)
                : AnswerFormatter()
            {
                connect(this,SIGNAL(formattedAnswer(QByteArray)),answerRecipient,answerProcessor);
            }
        };


#endif // ANSWERFORMATTER_H
