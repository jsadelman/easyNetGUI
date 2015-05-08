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
    virtual void formatAnswer(QString answer, QString cmd) = 0;

signals:
    void formattedAnswer(QString, QString);
    void formattedAnswer(QDomDocument*, QString);
    void formattedAnswer(QStringList, QString);
    void formattedAnswer(QByteArray, QString);

protected:
    AnswerFormatter(){}
};

class IdentityFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
//    void formatAnswer(QString answer) {emit formattedAnswer(QString("identity %1").arg(answer));} // test
    void formatAnswer(QString answer, QString cmd) Q_DECL_OVERRIDE {emit formattedAnswer(answer, cmd);}

    friend class AnswerFormatterFactory;

protected:
    IdentityFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QString, QString)),answerRecipient,answerProcessor);
    }

};

class XMLFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
    void formatAnswer(QString answer, QString cmd);

    friend class AnswerFormatterFactory;

protected:
    XMLFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QDomDocument*, QString)),answerRecipient,answerProcessor);
    }
};

class ListOfValuesFormatter: public AnswerFormatter
{
    Q_OBJECT
public:
    void formatAnswer(QString answer, QString cmd);

    friend class AnswerFormatterFactory;

protected:
    ListOfValuesFormatter(QObject *answerRecipient, char const *answerProcessor)
        : AnswerFormatter()
    {
        connect(this,SIGNAL(formattedAnswer(QStringList, QString)),answerRecipient,answerProcessor);
    }
};

class SVGFormatter: public AnswerFormatter
        {
            Q_OBJECT
        public:
            void formatAnswer(QString answer, QString cmd);

            friend class AnswerFormatterFactory;

        protected:
            SVGFormatter(QObject *answerRecipient, char const *answerProcessor)
                : AnswerFormatter()
            {
                connect(this,SIGNAL(formattedAnswer(QByteArray, QString)),answerRecipient,answerProcessor);
            }
        };


#endif // ANSWERFORMATTER_H
