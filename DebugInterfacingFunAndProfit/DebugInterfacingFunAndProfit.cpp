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
        UpdateUiForProcessing("Attempting to attach to executable... Please wait ...");
        m_TestOMatic->StartAndAttachToProgram();
        UpdateUiForUserInteraction("Done");

        return true;
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
        UpdateUiForProcessing("Obtaining Widget Properties...");
        while (m_Ui.tableProperties->rowCount() > 0)
        {
            m_Ui.tableProperties->removeRow(0);
        }        
        QMap<QString, WidgetPropertyData> properties = m_TestOMatic->GetWidgetProperties( item->text() );
        for(int index = 0; index < properties.size(); index++)
        {
            QString name = properties.keys()[index];
            QString type = properties[name].Type;
            QString value = properties[name].Value.toString();
            
            if( !name.isEmpty() && !type.isEmpty() && !value.isEmpty() )
            {
                int targetRow = m_Ui.tableProperties->rowCount();
                m_Ui.tableProperties->insertRow(targetRow);
                m_Ui.tableProperties->setItem(targetRow, 0, new QTableWidgetItem(name));
                m_Ui.tableProperties->setItem(targetRow, 1, new QTableWidgetItem(type));
                m_Ui.tableProperties->setItem(targetRow, 2, new QTableWidgetItem(value));
            }
        }
        UpdateUiForUserInteraction("Done");
        m_Ui.tableProperties->resizeColumnsToContents();
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
        m_Ui.listWidgets->clear();
        UpdateUiForProcessing("Refreshing widgets from server, please wait...");
        QStringList widgetNames = m_TestOMatic->GetAllWidgetNames();
        foreach(QString widget, widgetNames)
        {
            m_Ui.listWidgets->addItem(widget);
        }
        UpdateUiForUserInteraction("Done");
        return true;
    }
    else
    {
        QMessageBox::warning( this, "Program is not attached!", "Program either hasn't attached yet or has failed to attach to the target executable." );
        return false;
    }
}

void DebugInterfacingFunAndProfit::UpdateUiForProcessing(QString message)
{
    m_Ui.statusBar->showMessage(message);
    m_Ui.listWidgets->setEnabled(false);
    m_Ui.tableProperties->setEnabled(false);
    //m_Ui.progresBar->set
    m_Ui.statusBar->repaint();
    m_Ui.listWidgets->repaint();
    m_Ui.tableProperties->repaint();
}


void DebugInterfacingFunAndProfit::UpdateUiForUserInteraction(QString message)
{
    m_Ui.statusBar->showMessage(message);
    m_Ui.listWidgets->setEnabled(true);
    m_Ui.tableProperties->setEnabled(true);
}

