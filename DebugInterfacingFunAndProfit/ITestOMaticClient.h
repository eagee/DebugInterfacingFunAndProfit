#pragma once

class ITestOMaticClient
{
public:
    // Potential Operations
    virtual QString GetWidgetNameAt(int x, int y) = 0;
    virtual QStringList GetAllWidgetNames() = 0;
    virtual QMap<QString, QVariant> GetWidgetProperties(QString widgetName) = 0;
    virtual bool WaitForWidget(QString widgetName) = 0;
    virtual QVariant GetValueForProperty(QString widgetName, QString propertyName) = 0;
    virtual bool IsAttached() = 0;
    virtual QObject *GetQObject() = 0;

    // Potential Slots
    virtual bool StartAndAttachToProgram() = 0;
    virtual void OnDebugProcessIsReadyForConnections() = 0;

    // Potential Signals
    virtual void WidgetNameFound(QString widgetName) = 0;
    virtual void WidgetNamesFound(QStringList allWidgets) = 0;
    virtual void WidgetProperties(QString widgetName, QMap<QString, QVariant> properties) = 0;
};

Q_DECLARE_INTERFACE(ITestOMaticClient, "EaganRackley.ITestOMaticClient/1.0")