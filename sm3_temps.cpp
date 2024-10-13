//! \file sm_font.cpp
#include "sysmon3.h"
#include "sm3_temps.h"
#include "udp.h"

SM3_Temps::SM3_Temps( QString system, QSettings* baseSettings ) 
{
   server   = system;
   settings = baseSettings;

   // Set up widgets
   setWidgetData( server, settings );

   // Get current font; second parameter is default
   QString family  = settings->value( server + "-fontFamily", "DejaVu Sans" ).toString();
   int     size    = settings->value( server + "-fontSize"  , 12 ).toInt();
   QFont   font    = QFont( family, size, QFont::Normal );

   // Frame layout
   setWindowTitle( "Temperature Selection Dialog" );
 
   QVBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   get_saved_temp_config();
   get_temp_data();

   int row = 0;
   tempsLayout = new QGridLayout();

   QLabel* lblInterface = sm_banner( "Interface" );
   QLabel* lblSensor    = sm_banner( "Sensor" );
   QLabel* lblSelected  = sm_banner( "Selected" );
   QLabel* lblLabel     = sm_banner( "Label" );
   
   tempsLayout->addWidget( lblInterface, row,   0 );
   tempsLayout->addWidget( lblSensor,    row,   1 );
   tempsLayout->addWidget( lblSelected,  row,   2 );
   tempsLayout->addWidget( lblLabel,     row++, 3 );

   // A line looks like:
   // interface:<interface name>,<sensor name:value[,<sensor name:value...]

   // Add temperature data
   for ( int i = 1; i < sensors.size(); i++ )  // Skip first line -- not temps
   {
     // line[ 0 ] is "interface"
     // line[ 1 ] is rest of line
     QStringList line  = sensors[ i ].split( ';' ); 

     QStringList items     = line[ 1 ].split( ',' ); 
     QString     interface = items[ 0 ];             // interface name
      
     for ( int j = 1; j < items.size(); j++ )
     {
       QString sensor = items.at( j ).split( ":" )[ 0 ];
      
       // Check with settings to see if the checkbox is should be set
       // and the label cusomized
      
                   lblInterface = sm_label( interface );
                   lblSensor    = sm_label( sensor );
        QCheckBox* cbBox        = new QCheckBox();
                   cbBox->setFont( font );
        QLineEdit* leLabel      = sm_lineedit( sensor, 0 );

        // See if we have the entry in settings
        QString    key          = interface + "," + sensor;

        foreach (const QString value, config )
        {
           if ( value.contains( key ) )
           {
              cbBox->setCheckState( Qt::Checked );

              QStringList entries = value.split( "," );
              leLabel->setText( entries[ 2 ] );
              break;
           }
        }

        tempsLayout->addWidget( lblInterface, row,   0 );
        tempsLayout->addWidget( lblSensor,    row,   1 );
        tempsLayout->addWidget( cbBox,        row,   2, Qt::AlignHCenter );
        tempsLayout->addWidget( leLabel,      row++, 3 );
     }
   }
   
   // Buttons
   pb_apply = sm_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

   //pb_help = sm_pushbutton( tr( "Help" ) );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_exit = sm_pushbutton( tr( "Exit" ) );
   connect( pb_exit, SIGNAL( clicked() ), SLOT( close() ) );

   QBoxLayout* buttons = new QHBoxLayout();

   buttons->addWidget( pb_apply  );
   //buttons->addWidget( pb_help   );
   buttons->addWidget( pb_exit );

   topbox->addLayout( tempsLayout );
   topbox->addLayout( buttons );
}

void SM3_Temps::get_temp_data( void )
{  
   sysmonUDP* udp  = new sysmonUDP( &server );
   QString    data = udp->getData();
   delete udp;
   
   sensors = data.split( '\n', Qt::SkipEmptyParts );
}

void SM3_Temps::get_saved_temp_config()
{  
   QString group = QString( server + "-temperatures" );

   settings->beginGroup( group );

   QStringList keys = settings->childKeys();
     
   foreach (const QString &key, keys)
       config << settings->value( key ).toString();
   settings->endGroup();
}

void SM3_Temps::apply()
{
   QString group = QString( server + "-temperatures" );

   // Removes the group, and all it keys
   settings->beginGroup( group );
   settings->remove( "" );              
   settings->endGroup();

   // Start over
   settings->beginGroup( group );    

   int index = tempsLayout->rowCount();

   // Write settings
   // key = interface,sensor,label 
   for ( int i = 1; i < index ; i++ )    
   {
      QWidget*   item;
      QString    interface;
      QString    sensor;
      QCheckBox* cb;
      QString    label;
      QString    key;

      item      = tempsLayout->itemAtPosition( i, 2 )->widget();
      cb        = dynamic_cast<QCheckBox*>(item);
      if ( ! cb->isChecked() ) continue;

      item      = tempsLayout->itemAtPosition( i, 0 )->widget();
      interface = dynamic_cast<QLabel*>(item)->text();  

      item      = tempsLayout->itemAtPosition( i, 1 )->widget();
      sensor    = dynamic_cast<QLabel*>(item)->text();

      // Get line edit entry, but remove any commas
      item      = tempsLayout->itemAtPosition( i, 3 )->widget();
      label     = dynamic_cast<QLineEdit*>(item)->text();
      label.remove( "," );

      // Set temperature entry in settings
      key       = "temp" + QString::number( i );

      settings->setValue( key, interface + "," + sensor + "," + label );
   }

   settings->endGroup();
   settings->sync();
   emit updateTemps();
}

//void SM_Font::help()
//{
// US_Help* help = new US_Help();
// help->show_help( "manual/usfont.html" );
//}

