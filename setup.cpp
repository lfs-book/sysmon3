#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QHostInfo>
#include <QRegularExpression>
#include <QTime>
#include <QTimer>
#include <QUdpSocket>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

//#include "sysmon3.h"
#include "setup.h"
#include "udp.h"

setup::setup()
{
   setWindowTitle( tr( "Setup" ) );

   // Start layout   
   layout = new QVBoxLayout;

   // ListWidget
   listBox = new QListWidget();

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
   layout->addWidget( listBox );

   // LineEdit
   editLine = new QLineEdit();
   layout->addWidget( editLine );

   // Pushbuttons Row 1
   QHBoxLayout* row1 = new QHBoxLayout();

   QPushButton* pb_add = new QPushButton( tr( "Add" ) );
   connect( pb_add, SIGNAL( clicked() ), this, SLOT( add() ) );
   row1->addWidget( pb_add  );

   QPushButton* pb_delete = new QPushButton( tr( "Delete" ) );
   connect( pb_delete, SIGNAL( clicked() ), this, SLOT( deleteEntry() ) );
   row1->addWidget( pb_delete  );

   QPushButton* pb_save = new QPushButton( tr( "Save" ) );
   connect( pb_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   row1->addWidget( pb_save  );

   layout->addLayout( row1 );

   // Pushbuttons Row 2
   QHBoxLayout* row2 = new QHBoxLayout();

   QPushButton* pb_test = new QPushButton( tr( "Test" ) );
   connect( pb_test, SIGNAL( clicked() ), this, SLOT( test() ) );
   row2->addWidget( pb_test  );

   QPushButton* pb_exit = new QPushButton( tr( "Exit" ) );
   connect( pb_exit, SIGNAL( clicked() ), this, SLOT( close() ) );
   row2->addWidget( pb_exit  );

   QPushButton* pb_start = new QPushButton( tr( "Start" ) );
   connect( pb_start, SIGNAL( clicked() ), this, SLOT( close() ) );
   connect( pb_start, SIGNAL( clicked() ), this, SLOT( start() ) );
   row2->addWidget( pb_start  );

   layout->addLayout( row2 );

   this->setLayout( layout );
}

setup::~setup()
{
}

void setup::test( void )
{
   QMessageBox msgBox;

   QString s = listBox->currentItem()->text(); 

   // At this point, request a response from the server.
   sysmonUDP* u      = new sysmonUDP( &s );
   QString    result = u->getData();

   if ( result == "Timeout" )
   {
     QString detail = "Is the sysmond running on the target system?\n";
     detail        += "Is the network to the target system up?";

     msgBox.setText( "The test timed out. " );
     msgBox.setInformativeText( detail );
   }
   else if ( result == "Bad IP lookup" )
     msgBox.setText( "Could not get an ip address." );
   
   else
     msgBox.setText( "The network and sysmond are OK." );

   msgBox.exec();
}

void setup::add( void )
{
   // See if anything is there.
   QString text = editLine->text();
   if ( text.size() == 0 ) return;

   // See if it matches something in the list box
   for ( int i = 0; i < listBox->count(); i++  )
      if ( listBox->item( i )->text() == editLine->text() ) 
      {
         QMessageBox msgBox;
         msgBox.setText( tr( "That entry is already present." ) );
         msgBox.exec();
         
         return;
      }

   // Add the new entry and clear the entry box.
   listBox->addItem( editLine->text() );
   listBox->setCurrentRow( listBox->count() - 1 );
   editLine->clear();
}

void setup::deleteEntry( void )
{
   listBox->takeItem( listBox->currentRow() );
}

void setup::save( void )
{
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
   // Send signal
   emit showMainWindow( listBox->currentItem()->text() );
}


