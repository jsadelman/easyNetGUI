#ifndef DEFAULTDIRS_H
#define DEFAULTDIRS_H

#include <QString>

class DefaultDirs
{
public:
    DefaultDirs() : lastOpenDir(""), lastSaveDir(""), m_defaultOpenDir(""), m_defaultSaveDir("") {}
    QString defaultOpenDir() {return m_defaultOpenDir;}
    QString defaultSaveDir() {return m_defaultSaveDir;}
    QString defaultDir() {return m_defaultOpenDir == m_defaultSaveDir ? m_defaultOpenDir : QString();}
    void setDefaultOpenDir(QString dir) {m_defaultOpenDir = dir;}
    void setDefaultSaveDir(QString dir) {m_defaultSaveDir = dir;}
    void setDefaultDir(QString dir) {setDefaultOpenDir(dir); setDefaultSaveDir(dir);}

protected:
    QString lastOpenDir;
    QString m_defaultOpenDir;
    QString lastSaveDir;
    QString m_defaultSaveDir;
};


#endif // DEFAULTDIRS_H
