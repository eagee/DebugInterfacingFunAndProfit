#pragma once
#ifndef LOCALSOCKETIPC_H
#define LOCALSOCKETIPC_H

#include <QObject>
#include <QtNetwork>
#include "IQProcessDebugger.h"
#include "ITestOMaticClient.h"

class TestOMaticClient : public QObject, public ITestOMaticClient
{
    Q_OBJECT
    Q_INTERFACES( ITestOMaticClient )

private:
    
    // Attributes
    bool m_WaitForSignal;
    quint16 m_BlockSize;
    QString m_ServerName;

    // Associations
    IQProcessDebugger *m_ProcessDebugger;
    QThread *m_Thread;
    QLocalSocket *m_Socket;
    QMutex m_Mutex;
    QWaitCondition m_WaitForTestOMaticServer;

public:

    TestOMaticClient(QString program, QString arguments, QObject *parent = 0);

    ~TestOMaticClient();

    virtual QString GetWidgetNameAt(int x, int y);

    virtual QStringList GetAllWidgetNames();

    virtual QMap<QString, QVariant> GetWidgetProperties(QString widgetName);

    virtual bool WaitForWidget(QString widgetName);

    virtual QVariant GetValueForProperty(QString widgetName, QString propertyName);

    virtual bool IsAttached();

    virtual QObject *GetQObject();


public slots:

    virtual bool StartAndAttachToProgram();

    virtual void OnDebugProcessIsReadyForConnections();

signals:

    void WidgetNameFound(QString widgetName);

    void WidgetNamesFound(QStringList allWidgets);

    void WidgetProperties(QString widgetName, QMap<QString, QVariant> properties);

private slots:

    void socketConnected();

    void socketDisconnected();

    void socketError(QLocalSocket::LocalSocketError);

private:
    
    void Connect();

    void Disconnect();

    bool Connected();

    void Abort();

    void Flush();
};

#endif // LOCALSOCKETIPC_H
