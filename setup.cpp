#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>

#include "setup.h"
#include "udp.h"

setup::setup()
{
   setWindowTitle( tr( "Setup" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   // Start layout   
   layout = new QVBoxLayout;

   // ListWidget
   listBox = new QListWidget();

   QSettings settings( "LinuxFromScratch/hosts" );
   
   // Populate the listBox with known servers
   int size = settings.beginReadArray( "hosts" );
   if ( size == 0 )
      new QListWidgetItem( tr( "localhost" ), listBox );
   else
   {   
      for ( int i = 0; i < size; i++ )
      {
         settings.setArrayIndex(i);
         new QListWidgetItem( settings.value( "hosts" ).toString(), listBox );
      }
   }

   settings.endArray();

   listBox->setCurrentRow( 0 );

   layout->addWidget( listBox );

   // LineEdit
   editLine = new QLineEdit();
   connect( editLine, &QLineEdit::returnPressed, this, &setup::add );
   layout->addWidget( editLine );

   // Pushbuttons Row 1
   QHBoxLayout* row1 = new QHBoxLayout();

   QPushButton* pb_add = new QPushButton( tr( "Add" ) );
   connect( pb_add, &QPushButton::clicked, this, &setup::add );
   row1->addWidget( pb_add  );

   QPushButton* pb_delete = new QPushButton( tr( "Delete" ) );
   connect( pb_delete, &QPushButton::clicked, this, &setup::deleteEntry );
   row1->addWidget( pb_delete  );

   QPushButton* pb_save = new QPushButton( tr( "Save" ) );
   connect( pb_save, &QPushButton::clicked, this, &setup::save );
   row1->addWidget( pb_save  );

   layout->addLayout( row1 );

   // Pushbuttons Row 2
   QHBoxLayout* row2 = new QHBoxLayout();

   QPushButton* pb_test = new QPushButton( tr( "Test" ) );
   connect( pb_test, &QPushButton::clicked, this, &setup::test );
   row2->addWidget( pb_test  );

   QPushButton* pb_exit = new QPushButton( tr( "Exit" ) );
   connect( pb_exit, &QPushButton::clicked, this, &QCoreApplication::quit );
   row2->addWidget( pb_exit  );

   QPushButton* pb_start = new QPushButton( tr( "Start" ) );
   connect( pb_start, &QPushButton::clicked, this, &setup::start );
   row2->addWidget( pb_start  );

   layout->addLayout( row2 );

   this->setLayout( layout );
}

void setup::test( void )
{
   QMessageBox msgBox( this );

   QListWidgetItem* item = listBox->currentItem();
   if ( item == nullptr ) return;   // Empty list or nothing selected

   QString s = item->text(); 

   // At this point, request a response from the server.
   sysmonUDP u( &s );
   QString   result = u.getData();

   if ( result == "Timeout" )
   {
     QString detail = "Is the sysmond running on the target system?\n";
     detail        += "Is the network to the target system up?";

     msgBox.setText( "The test timed out. " );
     msgBox.setInformativeText( detail );
   }
   else if ( result == "Bad IP lookup" )
     msgBox.setText( "Could not get an ip address." );
   
   else if ( ! result.contains( ";time:" ) )
     msgBox.setText( "Unexpected reply: " + result.left( 80 ) );

   else
     msgBox.setText( "The network and sysmond are OK." );

   msgBox.exec();
}

void setup::add( void )
{
   // See if anything is there.
   QString text = editLine->text().trimmed();
   if ( text.size() == 0 ) return;

   // See if it matches something in the list box
   for ( int i = 0; i < listBox->count(); i++  )
      if ( listBox->item( i )->text() == text ) 
      {
         QMessageBox msgBox( this );
         msgBox.setText( tr( "That entry is already present." ) );
         msgBox.exec();
         
         return;
      }

   // Add the new entry and clear the entry box.
   listBox->addItem( text );
   listBox->setCurrentRow( listBox->count() - 1 );
   editLine->clear();
}

void setup::deleteEntry( void )
{
   // nullptr if empty; delete is safe
   delete listBox->takeItem( listBox->currentRow() );
}

void setup::save( void )
{
   QSettings settings( "LinuxFromScratch/hosts" );

   settings.remove         ( "hosts" );
   settings.beginWriteArray( "hosts" );

   for ( int i = 0; i < listBox->count(); i++  )
   {
      settings.setArrayIndex(i);
      settings.setValue( "hosts", listBox->item( i )->text() );
   }
   settings.endArray();
}

void setup::start( void )
{
   QListWidgetItem* item = listBox->currentItem();
   if ( item == nullptr ) return;   // Empty list or nothing selected

   // Make sure the current list is saved
   save();

   // Send signal
   emit showMainWindow( item->text() );
   close();
}


