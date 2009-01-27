/***************************************************************************
 *   Copyright (C) 2008 by Heiko Koehn   *
 *   KoehnHeiko@googlemail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
e *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QInputDialog>
#include <QMessageBox>

#include "dlgConnectionProfiles.h"
#include "Host.h"
#include "HostManager.h"
#include "mudlet.h"

dlgConnectionProfiles::dlgConnectionProfiles(QWidget * parent) : QDialog(parent)
{
    setupUi( this );
    //    active_profile.clear();
    //active_item = NULL;
    QPushButton *connect_button = dialog_buttonbox->addButton(tr("Connect"), QDialogButtonBox::AcceptRole);
    connect_button->setIcon(QIcon(":/dialog-ok-apply.png"));
    connect( new_profile_button, SIGNAL( pressed() ), this, SLOT( slot_addProfile() ) );
    connect( remove_profile_button, SIGNAL( pressed() ), this, SLOT( slot_deleteProfile() ) );
    connect( mud_info_groupbox, SIGNAL ( clicked ( bool ) ), this, SLOT ( slot_showmudlist_clicked ( bool ) )) ;
    connect( profile_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_name(const QString)));
    connect( host_name_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_url(const QString)));
    connect( port_entry, SIGNAL(textEdited(const QString)), this, SLOT(slot_update_port(const QString)));    
    connect( this, SIGNAL( update() ), this, SLOT( slot_update() ) );
    connect( profiles_tree_widget, SIGNAL( itemClicked(QTreeWidgetItem *, int) ), SLOT( slot_item_clicked(QTreeWidgetItem *) ) );
    connect( profiles_tree_widget, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT ( slot_connection_dlg_finnished() ) );
    connect( this, SIGNAL (accept()), this, SLOT (slot_connection_dlg_finnished()));
    connect( this, SIGNAL (finished(int)), this, SLOT (slot_finished(int)));
    
    notificationArea->hide();
    notificationAreaIconLabelWarning->hide();
    notificationAreaIconLabelError->hide();
    notificationAreaIconLabelInformation->hide();
    notificationAreaMessageBox->hide();
    
    mRegularPalette.setColor(QPalette::Text,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mRegularPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mRegularPalette.setColor(QPalette::Base,QColor(255,255,225));
    
    mOKPalette.setColor(QPalette::Text,QColor(0,0,192));
    mOKPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mOKPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mOKPalette.setColor(QPalette::Base,QColor(235,255,235));    
    
    mErrorPalette.setColor(QPalette::Text,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::Highlight,QColor(0,0,192));
    mErrorPalette.setColor(QPalette::HighlightedText, QColor(255,255,255));
    mErrorPalette.setColor(QPalette::Base,QColor(255,235,235));    
    
    
    
}

void dlgConnectionProfiles::slot_update_url( const QString url )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    QString profile = pItem->text(0);
    if( pItem )
    {
        QUrl check;
        check.setHost( url );//, QUrl::StrictMode );            
        if( check.isValid() )
        {
            host_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "url", url );
        }
        else
        {
            host_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText( QString("Please enter the URL or IP address of the MUD server.")+check.errorString() );
        }
    }
}

void dlgConnectionProfiles::slot_update_port( const QString port )
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    QString profile = pItem->text(0);
    if( pItem )
    {
        int num = port.toInt();
        if( (num > 20) && (num < 65536) )
        {
            port_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
            writeProfileData( profile, "port", port );
        }
        else
        {
            port_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText( QString("Please enter the port number of the server.") );
        }
    }
}

void dlgConnectionProfiles::slot_update_name( const QString name )             
{
    QTreeWidgetItem * pItem = profiles_tree_widget->currentItem();
    
    if( pItem )
    {
        if( ! mProfileList.contains( name ) ) 
            mEditOK = true;
        else
            mEditOK = false;
        

        if( ! mSavedNewName )
        {
            // keep track of the new profile name that isnt yet valid
            // and thus hasnt been written to disc yet
            mUnsavedProfileName = name;
            pItem->setText( 0, name );
        }
        else
        {
            mCurrentProfileEditName = pItem->text( 0 );
            int row = mProfileList.indexOf( mCurrentProfileEditName );
            if( (row >= 0) && ( row < mProfileList.size() ) )
            {
                mProfileList[row] = name;
                pItem->setText( 0, name );
            }
        }
        
        if( mEditOK )
        {
            QDir dir(QDir::homePath()+"/.config/mudlet/profiles");
            if( ! mSavedNewName )
            {
                dir.mkpath( QDir::homePath()+"/.config/mudlet/profiles/"+mUnsavedProfileName );
                mProfileList << name;
                pItem->setText( 0, name );
                mSavedNewName = true;
                mUnsavedProfileName = "";
            }
            else     
            {
                dir.rename( mCurrentProfileEditName, name );
            }
            
            profile_name_entry->setPalette( mOKPalette );
            notificationArea->hide();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->hide();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->hide();
        }
        else
        {
            profile_name_entry->setPalette( mErrorPalette );
            notificationArea->show();
            notificationAreaIconLabelWarning->hide();
            notificationAreaIconLabelError->show();
            notificationAreaIconLabelInformation->hide();
            notificationAreaMessageBox->show();
            notificationAreaMessageBox->setText(tr("A profile with the current name already exists. Please use a longer name or a different name."));
        }
    }
}
             
void dlgConnectionProfiles::slot_showmudlist_clicked ( bool checked )
{
    if (checked)
        mud_treewidget->show();
    else
        mud_treewidget->hide();
}

void dlgConnectionProfiles::slot_addProfile()
{
    fillout_form();
    
    welcome_message->hide();
    basic_info_groupbox->show();
    autologin_groupbox->show();
    mud_info_groupbox->show();

    QStringList newname;
    mUnsavedProfileName = tr("new profile name");
    newname << mUnsavedProfileName;
    
    QTreeWidgetItem * pItem = new QTreeWidgetItem( (QTreeWidgetItem *)0, newname);
    if( ! pItem )
    {
        return;
    }
    mSavedNewName = false;
    profiles_tree_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    profiles_tree_widget->insertTopLevelItem( profiles_tree_widget->topLevelItemCount(), pItem );    
    
    profiles_tree_widget->setItemSelected(profiles_tree_widget->currentItem(), false); // Unselect previous item
    profiles_tree_widget->setCurrentItem( pItem );
    profiles_tree_widget->setItemSelected( pItem, true );
        
    //profiles_tree_widget->sortByColumn(0, Qt::AscendingOrder);
    
    profile_name_entry->setText( mUnsavedProfileName );
    profile_name_entry->setFocus();
    profile_name_entry->selectAll();
    
}

void dlgConnectionProfiles::slot_deleteProfile()
{
    if( ! profiles_tree_widget->currentItem() ) return;

    QString profile = profiles_tree_widget->currentItem()->text( 0 );
    if ( QMessageBox::question(this, tr("Confirmation"), tr("Are you sure you want to delete %1 ?").arg( profile ), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No )
        return;
    
    profiles_tree_widget->takeTopLevelItem( profiles_tree_widget->currentIndex().row() );
    QDir dir( QDir::homePath()+"/.config/mudlet/profiles/"+profile );
    QStringList deleteList = dir.entryList();
    for( int i=0; i<deleteList.size(); i++ )
    {
        dir.remove( deleteList[i] );
    }
    dir.rmpath( dir.path());
    
    if( ! mProfileList.size() )
    {
        welcome_message->show();
        basic_info_groupbox->hide();
        autologin_groupbox->hide();
        mud_info_groupbox->hide();
    }
    fillout_form();
    profiles_tree_widget->setFocus();
}

QString dlgConnectionProfiles::readProfileData( QString profile, QString item )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    
    file.open( QIODevice::ReadOnly );
    QString fname=QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item;
    QDataStream ifs( & file ); 
    QString ret;
    ifs >> ret;
    file.close();
    return ret;
}

void dlgConnectionProfiles::writeProfileData( QString profile, QString item, QString what )
{
    QFile file( QDir::homePath()+"/.config/mudlet/profiles/"+profile+"/"+item );
    file.open( QIODevice::WriteOnly | QIODevice::Unbuffered );
    QDataStream ofs( & file ); 
    ofs << what;
    file.close();
}

void dlgConnectionProfiles::slot_item_clicked(QTreeWidgetItem *pItem) 
{
    if( pItem )
    {
        QString profile_name = pItem->text( 0 );
        
        profile_name_entry->setPalette( mRegularPalette );        
        host_name_entry->setPalette( mRegularPalette );
        port_entry->setPalette( mRegularPalette );
        
        profile_name_entry->setText( profile_name );
        QString profile = profile_name;
        QString item = "url";
        QString val = readProfileData( profile, item );
        host_name_entry->setText( val );
        item = "port";
        val = readProfileData( profile, item );
        port_entry->setText( val );
        //character_password_entry->setText(  );
        //login_entry->setText(  );
    }
}

void dlgConnectionProfiles::fillout_form()
{
    profiles_tree_widget->clear();
    profile_name_entry->clear();
    host_name_entry->clear();
    port_entry->clear();
    
    mProfileList = QDir(QDir::homePath()+"/.config/mudlet/profiles").entryList(QDir::Dirs, QDir::Time);
    
    if( mProfileList.size() < 1 ) 
    {
        welcome_message->show();
        profiles_tree_widget->hide();
        basic_info_groupbox->hide();
        autologin_groupbox->hide();
        mud_info_groupbox->hide();
    }
    else
    {
        profiles_tree_widget->show();
        welcome_message->hide();
        basic_info_groupbox->show();
        autologin_groupbox->show();
        mud_info_groupbox->show();
    }
    for( int i=0; i<mProfileList.size(); i++ )
    {
        QString s = mProfileList[i];
        if( s.size() < 1 ) 
            continue;
        if( (mProfileList[i] == ".") || (mProfileList[i] == ".." ) )
            continue;
        QStringList sList;
        sList << mProfileList[i];
        QTreeWidgetItem * pItem = new QTreeWidgetItem( (QTreeWidgetItem *)0, sList);
        profiles_tree_widget->insertTopLevelItem( profiles_tree_widget->topLevelItemCount(), pItem );    
    }
}

void dlgConnectionProfiles::slot_connection_dlg_finnished()
{
    QDialog::accept();
}

void dlgConnectionProfiles::save()
{
    /*   if (active_profile.isEmpty())
        return;

    QString profile_name = profile_name_entry->text().trimmed();
    QString url = host_name_entry->text().trimmed();
    QString pass = character_password_entry->text().trimmed();
    QString login = login_entry->text().trimmed();
    int port = port_entry->text().trimmed().toInt();
    QString website = website_entry->text().trimmed();

    if (Host *pHost = HostManager::self()->getHost( active_profile )) 
    {
        pHost->setName( profile_name );
        pHost->setUrl( url );
        pHost->setPass( pass );
        pHost->setLogin( login );
        pHost->setPort( port );
    }

    if ( profile_name != active_profile ) 
    {
        HostManager::self()->renameHost(active_profile);
        active_item->setText( 0, profile_name);
    } */
}

void dlgConnectionProfiles::slot_finished(int f)
{
    if (f == 1)
    {
        QString profile_name = profile_name_entry->text().trimmed();
        bool ok = HostManager::self()->addHost( profile_name, port_entry->text().trimmed(), "", "" );
        Host * pHost = HostManager::self()->getHost( profile_name );
        if( pHost )
        {
            pHost->setUrl( host_name_entry->text().trimmed() );
        }
        else 
            return;
        
        if( profile_name.size() < 1 ) return;
        emit signal_establish_connection( profile_name );
    }
}

void dlgConnectionProfiles::slot_update()
{
    update();      
}

