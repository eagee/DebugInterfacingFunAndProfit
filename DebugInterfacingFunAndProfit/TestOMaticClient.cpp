#include "stdafx.h"
#include "TestOMaticClient.h"
#include "QProcessDebuggerFactory.h"

TestOMaticClient::TestOMaticClient(QString program, QString arguments, QObject *parent /* = 0*/) : QObject(parent), m_ServerName("TestOMaticServer") 
{

    m_Socket = new QLocalSocket(this);
    Q_EXPECT( connect(m_Socket, SIGNAL(connected()), this, SLOT(socketConnected())) );
    Q_EXPECT( connect(m_Socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected())) );
    Q_EXPECT( connect(m_Socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError))) );

    m_Thread = new QThread(this);
    m_Thread->setObjectName( "WinTestOMaticClientThread" );

    m_ProcessDebugger = QProcessDebuggerFactory::GetDebuggerOfType(program, arguments, ProcessDebugger::Windows, nullptr);
    Q_EXPECT( connect(m_ProcessDebugger->GetQObject(), SIGNAL(DebugProcessIsReadyForConnections()), SLOT(OnDebugProcessIsReadyForConnections())) );
}

TestOMaticClient::~TestOMaticClient() {
    
    m_Socket->abort();
    m_Socket->deleteLater();
    
    m_Thread->quit();
    //m_Thread->wait();
    m_Thread->deleteLater();
}

bool TestOMaticClient::StartAndAttachToProgram()
{
    bool success = true;
    // If the debugger was able to attach to the process successfully we start a thread, move the debugger
    // over to it, and then use invokeMethod to execute the debug loop. @see WinProcessDebugger::ExecuteDebugLoop()
    // Once the debug loop executes it injects our dll and sets up an IPC server. Once it knows the server is 
    // listening it'll trigger our emit DebugProcessIsReadyForConnections() (@see TestOMaticClient::OnDebugProcessIsReadyForConnections)
    
    m_ProcessDebugger->MoveToThread( m_Thread );
    if(m_Thread->isRunning() == false)
    {
        m_Thread->start();
    }
    
    success = QMetaObject::invokeMethod(m_ProcessDebugger->GetQObject(), "StartAndAttachToProgram", Qt::QueuedConnection);
    if( success == false )
    {
        qDebug() << Q_FUNC_INFO << ": Failed to attach and start debug loop!";
        return false;
    }

    m_Mutex.lock();
    m_WaitForSignal = true;
    while( m_WaitForSignal == true )
    {
        if( m_WaitForTestOMaticServer.wait(&m_Mutex, 1000) == true ) 
        {
            success = true;
        }
        else
        {
            qDebug() << Q_FUNC_INFO << ": Waiting for TestOMaticServer...";
        }
        QCoreApplication::processEvents();
    }
    m_Mutex.unlock();

    // If we were signaled successfully from the debug process we can go ahead and attempt to connect to the IPC server
    // and handle any errors accordingly...
    if(success == true )
    {
        this->Connect();
        success = this->Connected();
    }
    
    return success;
}

void TestOMaticClient::OnDebugProcessIsReadyForConnections()
{
    // see TestOMaticClient::StartAndAttachToProgram - once this signal is received we know we have the server set up and ready to go.
    m_WaitForSignal = false;
    m_WaitForTestOMaticServer.wakeAll();
}

QString TestOMaticClient::GetWidgetNameAt(int x, int y)
{
    QString returnValue;
    Q_ASSERT_X(false, Q_FUNC_INFO, "The method or operation is not implemented.");
    return returnValue;
}

QStringList TestOMaticClient::GetAllWidgetNames()
{
    QStringList returnValue;
    Q_ASSERT( m_Socket->isOpen() );

    if( m_Socket->isWritable() )
    {
        m_Socket->write("GetAllWidgets");
        m_Socket->waitForReadyRead();
        while( m_Socket->waitForReadyRead(1000) )
        {
            QByteArray bResponse = m_Socket->readLine();
            QString response( bResponse.data() );
            qDebug() << Q_FUNC_INFO << ": Test O Matic Response: " << response;
            returnValue.push_back(response);
        }
    }

    qDebug() << Q_FUNC_INFO << "Sorting Widget Names...";
    returnValue.sort();

    emit WidgetNamesFound(returnValue);

    return returnValue;
}

QMap<QString, WidgetPropertyData> TestOMaticClient::GetWidgetProperties(QString widgetName)
{
    const int PROPERTY_NAME = 0;
    const int PROPERTY_TYPE = 1;
    const int PROPERTY_VALUE = 2;
    QMap<QString, WidgetPropertyData> returnValue;
    //returnValue.setInsertInOrder(true);

    Q_ASSERT( m_Socket->isOpen() );

    if( m_Socket->isWritable() )
    {
        QString request = "GetWidgetProperties," + widgetName;
        qDebug() << Q_FUNC_INFO << " Writing: " << request;
        m_Socket->write(request.toStdString().c_str());
        m_Socket->waitForReadyRead();
        while( m_Socket->waitForReadyRead(1000) )
        {
            QByteArray bPropertyInfo = m_Socket->readLine();
            QStringList propertyInfo = QString(bPropertyInfo).split(",");
            if( propertyInfo.size() == 3 )
            {
                qDebug() << Q_FUNC_INFO << ": Test O Matic Response: " << propertyInfo[PROPERTY_NAME] << "->" << propertyInfo[PROPERTY_VALUE];
                WidgetPropertyData data;
                data.Type = propertyInfo[PROPERTY_TYPE];
                data.Value = QVariant(propertyInfo[PROPERTY_VALUE]);
                returnValue.insert(propertyInfo[PROPERTY_NAME], data);
            }
            
        }
    }

    emit WidgetProperties(widgetName, returnValue);
    
    return returnValue;
}

bool TestOMaticClient::WaitForWidget(QString widgetName)
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "The method or operation is not implemented.");
    return false;
}

QVariant TestOMaticClient::GetValueForProperty(QString widgetName, QString propertyName)
{
    QVariant returnValue;
    Q_ASSERT_X(false, Q_FUNC_INFO, "The method or operation is not implemented.");
    return returnValue;
}    

void TestOMaticClient::Connect()
{
    m_Socket->abort();
    m_Socket->connectToServer(m_ServerName);
}

void TestOMaticClient::Disconnect()
{
    m_Socket->abort();
    m_Socket->disconnect();
}

bool TestOMaticClient::Connected()
{
    return m_Socket->isOpen();
}

void TestOMaticClient::Abort()
{
    m_Socket->abort();
}

void TestOMaticClient::Flush()
{
    m_Socket->flush();
}

void TestOMaticClient::socketConnected(){
    qDebug() << Q_FUNC_INFO << ": Socket connected!";
}

void TestOMaticClient::socketDisconnected() {
    qDebug() << Q_FUNC_INFO << ": socket_disconnected";
}

void TestOMaticClient::socketError(QLocalSocket::LocalSocketError) {
    qDebug() << Q_FUNC_INFO << ": socket_error";
}

bool TestOMaticClient::IsAttached()
{
    return this->Connected();
}

QObject * TestOMaticClient::GetQObject()
{
    return this;
}

