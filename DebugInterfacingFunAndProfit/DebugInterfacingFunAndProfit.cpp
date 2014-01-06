#include "stdafx.h"
#include <QString>
#include "DebugInterfacingFunAndProfit.h"

DebugInterfacingFunAndProfit::DebugInterfacingFunAndProfit(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags), m_TestOMatic(nullptr)
{
    m_Ui.setupUi(this);
    m_Ui.progressBar->setVisible(false);
    m_Ui.listWidgets->setContextMenuPolicy(Qt::CustomContextMenu);
    Q_EXPECT( connect(m_Ui.listWidgets, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&))) );
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
    else if( action->objectName() == "actionWidget_from_Mouse_Click" )
    {
        if( (m_TestOMatic != nullptr) && (m_TestOMatic->IsAttached()) )
        {
            QString result = m_TestOMatic->GetNextWidgetNameClicked();
            QList<QListWidgetItem*> items = m_Ui.listWidgets->findItems(result, Qt::MatchExactly);
            if(items.count() > 0)
            {
                m_Ui.listWidgets->setCurrentItem(items[0], QItemSelectionModel::Select);
            }
            else
            {
                QListWidgetItem *newItem = new QListWidgetItem(result, m_Ui.listWidgets);
                m_Ui.listWidgets->setCurrentItem(newItem, QItemSelectionModel::Select);
            }
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
                QTableWidgetItem *nameItem = new QTableWidgetItem(name);
                nameItem->setTextAlignment(Qt::AlignTop);
                QTableWidgetItem *typeItem = new QTableWidgetItem(type);
                typeItem->setTextAlignment(Qt::AlignTop);
                QTableWidgetItem *valueItem = new QTableWidgetItem(value);
                valueItem->setTextAlignment(Qt::AlignTop);

                m_Ui.tableProperties->insertRow(targetRow);
                m_Ui.tableProperties->setItem(targetRow, 0, nameItem);
                m_Ui.tableProperties->setItem(targetRow, 1, typeItem);
                m_Ui.tableProperties->setItem(targetRow, 2, valueItem);
                
            }
        }
        UpdateUiForUserInteraction("Done");
        m_Ui.tableProperties->resizeColumnsToContents();
        m_Ui.tableProperties->resizeRowsToContents();
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
    m_Ui.progressBar->setVisible(true);
    m_Ui.progressBar->repaint();
    m_Ui.statusBar->repaint();
    m_Ui.listWidgets->repaint();
    m_Ui.tableProperties->repaint();
}


void DebugInterfacingFunAndProfit::UpdateUiForUserInteraction(QString message)
{
    m_Ui.statusBar->showMessage(message);
    m_Ui.listWidgets->setEnabled(true);
    m_Ui.tableProperties->setEnabled(true);
    m_Ui.progressBar->setVisible(false);
}

void DebugInterfacingFunAndProfit::ShowContextMenu(const QPoint& pos)
{
    if(m_Ui.listWidgets->count() > 0)
    {
        // for most widgets
        QPoint globalPos = m_Ui.listWidgets->mapToGlobal(pos);
        // for QAbstractScrollArea and derived classes you would use:
        // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
        QMenu myMenu;
        myMenu.addAction("Copy");

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(m_Ui.listWidgets->currentItem()->text());
        }
    }
}

