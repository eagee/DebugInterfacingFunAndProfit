#include "stdafx.h"
#include <QString>
#include "DebugInterfacingFunAndProfit.h"

DebugInterfacingFunAndProfit::DebugInterfacingFunAndProfit(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags), m_TestOMatic(nullptr)
{
    m_Ui.setupUi(this);
}

bool DebugInterfacingFunAndProfit::SelectAndAttachToProcess()
{
    //QString targetExe = "E:\\MBAM\\mbamui\\Vs2010\\Debug\\MbamUI-vc100-x86-gd-0_4_13_388.exe";
    
    QString targetExe = QFileDialog::getOpenFileName(this, tr("Qt Executable"), "", tr("Qt Executable File (*.exe)"));

    if(targetExe.isEmpty() == false)
    {
        m_TestOMatic = new TestOMaticClient( targetExe, "", this );

        m_Ui.statusBar->showMessage("Attempting to attach to executable... Please wait ...");
        m_Ui.statusBar->repaint();
        m_TestOMatic->StartAndAttachToProgram();

        return RefreshWidgets();
    }

    return false;
};

DebugInterfacingFunAndProfit::~DebugInterfacingFunAndProfit()
{
    if( m_TestOMatic != nullptr )
    {
        m_TestOMatic->GetQObject()->deleteLater();
    }
}

void DebugInterfacingFunAndProfit::OnMenuTriggered(QAction *action)
{
    if( action->objectName() == "actionAttach_to_Exe")
    {
        if( SelectAndAttachToProcess() == true )
        {
            action->setDisabled(true);
        }
    }
    else if( action->objectName() == "actionRefresh_Widgets")
    {
        if( (m_TestOMatic != nullptr) && (m_TestOMatic->IsAttached()) )
        {
            RefreshWidgets();
        }
    }
}

void DebugInterfacingFunAndProfit::OnItemClicked(QListWidgetItem *item)
{
    if( (m_TestOMatic != nullptr) && (m_TestOMatic->IsAttached()) )
    {
        QMap<QString, QVariant> properties = m_TestOMatic->GetWidgetProperties( item->text() );
    }
    else
    {
        QMessageBox::warning( this, "Program is not attached!", "Program either hasn't attached yet or has failed to attach to the target executable." );
    }
}

bool DebugInterfacingFunAndProfit::RefreshWidgets()
{
    if( (m_TestOMatic != nullptr) && (m_TestOMatic->IsAttached()) )
    {
        m_Ui.statusBar->showMessage("Refreshing Widgets...");
        m_Ui.statusBar->repaint();
        m_Ui.listWidgets->clear();
        QStringList widgetNames = m_TestOMatic->GetAllWidgetNames();
        foreach(QString widget, widgetNames)
        {
            m_Ui.listWidgets->addItem(widget);
        }
        m_Ui.statusBar->showMessage("Done");
        return true;
    }
    else
    {
        QMessageBox::warning( this, "Program is not attached!", "Program either hasn't attached yet or has failed to attach to the target executable." );
        return false;
    }
}
