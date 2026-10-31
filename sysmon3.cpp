#include <QApplication>
#include <QAction>
#include <QTime>
#include <QTimer>
#include <QMessageBox>

#include "sysmon3.h"
#include "sm3_config.h"
#include "setup.h"
#include "udp.h"
#include "version.h"

int main(int argc, char *argv[])
{  
   QApplication a(argc, argv);
   QCoreApplication::setOrganizationName("LinuxFromScratch");
   QCoreApplication::setApplicationName("sysmon3");
   QCoreApplication::setApplicationVersion(SMVERSION);

   QString server;
   if ( argc > 1 ) server = argv[ 1 ];

   sysmon3 mainWin( server );
   // We don't show yet

   return a.exec();
}  

sysmon3::sysmon3( QString server )
{
   setContextMenuPolicy( Qt::ActionsContextMenu );  // Add actions for right click
   setWindowTitle( tr( "sysmon3" ) );

   // Actions 
   QAction* configAction = new QAction( tr("Configuration"), this );
   configAction->setShortcut( Qt::Key_F1 );
   connect( configAction, SIGNAL(triggered()), this, SLOT( config() ) );
   addAction( configAction );

   QAction* quitAction = new QAction( tr("E&xit"), this);
   quitAction->setShortcut(tr("Ctrl+Q"));
   connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
   addAction(quitAction);

   QAction* frameAction = new QAction( tr("F&rame"), this );
   frameAction->setShortcut( tr("Ctrl+F") );
   connect( frameAction, &QAction::triggered, this, &sysmon3::changeFrame );
   addAction( frameAction );

   timer = new QTimer( this );
   timer->setInterval( 1000 );  // Every second by default
   connect(timer, &QTimer::timeout, this, &sysmon3::refresh );
   //timer->start();  // Do this later

   bool runSetup = true;

   if ( ! server.isEmpty() )
   {
      // If server has been set up, don't run setup()
      // We can't use SM_Settings yet because we don't know the server   

      QSettings settings( "LinuxFromScratch/hosts" );  // Just use this temporarily 

      int size = settings.beginReadArray( "hosts" );
      if ( size > 0 )
      {
         for ( int i = 0; i < size; i++ )
         {
            settings.setArrayIndex( i );
            if ( settings.value( "hosts" ).toString() == server )
            {
               runSetup = false;  
               break;
            }
         }
      }
   }

   if ( runSetup )
   {
     setup* s = new setup();
     s->show();

     // The main window starts when the user presses config's "Start" button
     connect( s,    SIGNAL( showMainWindow( QString ) ), 
              this, SLOT  ( showMain      ( QString ) ) );
   }
   else 
      showMain( server );  // This is the sever name on the command line
}

void sysmon3::config( void )
{  
   // Only one configuration window at a time
   if ( configWindow != nullptr )
   {
      configWindow->raise();
      configWindow->activateWindow();
      return;
   }

   SM_Config* w = new SM_Config( settingsPtr, data, this->geometry() );
   configWindow = w;  // convenience

   connect( w, &SM_Config::updateEntries, this, &sysmon3::updateLayout );
   connect( w, &SM_Config::updateFonts,   this, &sysmon3::updateFont   );
   connect( w, &SM_Config::updateColors,  this, &sysmon3::updateColor  );
   connect( w, &SM_Config::updateTemps,   this, &sysmon3::updateTemp   );

   configWindow->show();
}

sysmon3::~sysmon3()
{
   // Setting values requires valid settingsPtr
   if ( settingsPtr == nullptr ) return;

   position = this->pos();
   settingsPtr->setValue( "positionX", QString::number( position.x() ) );
   settingsPtr->setValue( "positionY", QString::number( position.y() ) );
   settingsPtr->sync();
}

// This is a SLOT, but perhaps the slot should just be setup_all
void sysmon3::showMain( QString server )
{
   //server      = server1;
   settingsPtr = new SM_Settings( "LinuxFromScratch/" + server );

   // Start layout   
   layout = new QVBoxLayout; 

   // hostname  - we don't have the name yet
   lbl_hostname = banner( serverData.server, 0, QFont::Bold );
   lbl_hostname->setPalette( banner_palette );  // And we don't have a palette
   layout->addWidget( lbl_hostname );

   setup_all( server );
}

void sysmon3::setup_all( QString server )
{
   udp = new sysmonUDP( &server );
   fetch();

   // Use server name to initialize saved data
   get_settings();

   // Workaround before first fetch of data
   lbl_hostname->setText( serverData.server );
   lbl_hostname->setPalette( banner_palette );

   setup_time();
   setup_date();
   setup_uptime();
   setup_cpuLoad();
   setup_memory();
   setup_temps();
   updateFont();
   // End of layout

   QWidget* window = new QWidget();
   window->setLayout( layout );

   setCentralWidget( window );

   if ( settingsPtr->contains( "positionX" ) )
   {
      int x = settingsPtr->value( "positionX" ).toInt();
      int y = settingsPtr->value( "positionY" ).toInt();
      position = QPoint( x, y );
      this->move( position );
   }

   this->show();

   // Use the refreshInterval for queries
   int refresh = settingsPtr->value( "refreshInterval" ).toInt();
   timer->setInterval( qBound( 1, refresh, 9 ) * 1000 );

   timer->start();
   tick = 0;
}

void sysmon3::setup_time()
{
   lbl_time = nullptr;

   if ( settingsPtr->value( "useTime" ) != "true" ) return;
   lbl_time = label ( serverData.time );
   layout->addWidget( lbl_time );
}

void sysmon3::setup_date()
{
   lbl_date = nullptr;

   if ( settingsPtr->value( "useDate" ) != "true" )  return;

   lbl_date = label( serverData.date );
   layout->addWidget( lbl_date );
}

void sysmon3::setup_uptime()
{
   lbl_uptime = nullptr;

   if ( settingsPtr->value( "useUptime" ) != "true" ) return;

   uint secs    = serverData.uptime;
   uint minutes = (secs / 60  ) % 60;
   uint hours   = (secs / 3600) % 24;
   uint days    = secs / (3600 * 24);

   QString uptimeString = QStringLiteral( "%1d %2:%3" )
      .arg(days)
      .arg(hours,  2, 10, QChar('0') )
      .arg(minutes,2, 10, QChar('0') );

   lbl_uptime = label( uptimeString );
   layout->addWidget( lbl_uptime );
}

void sysmon3::setup_cpuLoad()
{
   bool useCPU    = settingsPtr->value( "useCPU" )    == "true";
   bool useCPUbar = settingsPtr->value( "useCPUbar" ) == "true";

   lbl_cpu   = nullptr;
   lbl_loads = nullptr;
   load      = nullptr;

   if ( useCPU || useCPUbar )
   {
     lbl_cpu = banner( "CPU Load", 0, QFont::Bold );
     layout->addWidget( lbl_cpu   );
   }

   if ( useCPU )
   {
      lbl_loads = label( serverData.load );
      layout->addWidget( lbl_loads );
   }

   if ( useCPUbar )
   {
      load = new QProgressBar();
      load->setRange( 0, 100 );
      load->setValue( qRound( serverData.cpuPercent * 100 ) );
      load->setFont( font_normal );

      // Set palette
      QPalette p = load->palette();

      QString color = settingsPtr->value( "progressColor" );
      QString bg    = settingsPtr->value( "progressBg"    );

      p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
      p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
      p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
      p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );
      load->setPalette( p );

      layout->addWidget( load );
   }
}

void sysmon3::setup_memory()
{
   lbl_memory = nullptr;
   memory     = nullptr;
   if ( settingsPtr->value( "useMemory" ) != "true" ) return;

   memory = new QProgressBar();
   memory->setRange( 0, 100 );
   memory->setFont ( font_normal );
   memory->setValue( qRound( serverData.memPercent ) );

   // set palette for memory
   QPalette p = memory->palette();

   QString color = settingsPtr->value( "progressColor" );
   QString bg    = settingsPtr->value( "progressBg"    );

   p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
   p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );
   memory->setPalette( p );

   lbl_memory = banner( "Memory Use", 0, QFont::Bold );
   layout->addWidget( lbl_memory );
   layout->addWidget( memory );
}

void sysmon3::setup_temps()
{
   tempsLayout = nullptr;
   lbl_temps   = nullptr;
   rowKeys.clear();

   QStringList tempConfig = settingsPtr->readGroup( "temperatures" );

   // If null, return
   if ( tempConfig.size() == 0 ) return;  

   // Create tempsLayout 
   tempsLayout = new QGridLayout();
 
   lbl_temps = banner( "Temperatures", 0, QFont::Bold );
   layout->addWidget( lbl_temps );
   
   // Populate tempsLayout
   for ( const QString& entry : tempConfig )
   {
      QStringList  sl   = entry.split( ',' ); // interface,sensor,label
      if ( sl.size() < 3 ) continue;          // Skip a damaged entry

      QString key  = sl[ 0 ] + "," + sl[ 1 ];
      int     row = rowKeys.size();
 
      // Remember the key by row, so two sensors can share a label
      rowKeys.append( key );

      tempsLayout->addWidget( label( sl[ 2 ]         ), row, 0 ); // Label name
      tempsLayout->addWidget( label( tempText( key ) ), row, 1 ); // Temperature
   }

   // Add to main layout
   layout->addLayout( tempsLayout );
}

// Get new data from sysmond.  On failure, keep the previous values.
void sysmon3::fetch()
{
   QString reply = udp->getData();
   if ( parse_data( reply ) ) 
   {
      data = reply;
      failures = 0;
      return;
   }

   if ( ++failures < 3 ) return;   // Three failures in a row is a problem

   // msgBox.exec() runs a nested event loop.  Qt won't re-enter this
   // timer's slot while it is still running, but stop the timer anyway
   // so it's clear nothing is fetched while the box is open.
   timer->stop();

   QMessageBox msgBox( this );
   msgBox.setText( "No valid reply from sysmond (" + reply.left( 80 ) + "). Retry?" );

   msgBox.addButton( "Retry", QMessageBox::YesRole );
   
   QAbstractButton* exitButton =
      msgBox.addButton( "Exit",  QMessageBox::NoRole );
   msgBox.exec();

   if ( msgBox.clickedButton() == exitButton )
   {
      // Unlike exit(), this lets the destructor save the window
      // position.  The queued call also works during startup,
      // before a.exec() is running.
      QTimer::singleShot( 0, []{ QCoreApplication::exit( EXIT_FAILURE ); } );
      // Note: the []{...} construct is a C++ lambda (unnamed function)
      // The [] contains the local variables for the function (none)
      // The {} contains the function body
      return;
   }

   failures = 0;
   timer->start();
}

// Parse a reply from sysmond into serverData.  Returns false, leaving
// serverData unchanged, if data is not a valid reply.  This includes
// the error strings from sysmonUDP::getData() such as "Timeout".
//
// <host>;time:<time>;uptime:<s>;load:<1 5 15>;cpu%:<0.00-1.00>;mem%:<0-100>
// interface;<interface name>,<sensor1>:value1,[<sensor#>:value#...]
// ...
bool sysmon3::parse_data( const QString& reply )
{
   QStringList lines = reply.split( '\n', Qt::SkipEmptyParts );
   if ( lines.isEmpty() ) return false;

   QStringList basicInfo = lines.at( 0 ).split( ';' );
   if ( basicInfo.size() < 6 ) return false;

   // Get the value of a "name:value" field, checking the name
   auto field = [ &basicInfo ]( int i, const QString& name, QString& value )
   {
      if ( ! basicInfo.at( i ).startsWith( name + ':' ) ) return false;
      value = basicInfo.at( i ).mid( name.size() + 1 );
      return true;
   };

   QString time, uptime, loads, cpu, mem;

   if ( ! field( 1, "time",   time   ) ||
        ! field( 2, "uptime", uptime ) ||
        ! field( 3, "load",   loads  ) ||
        ! field( 4, "cpu%",   cpu    ) ||
        ! field( 5, "mem%",   mem    ) )
      return false;

   // Parse "Sun Oct  6 14:31:31 2024 CST"
   QStringList items = time.split( ' ', Qt::SkipEmptyParts );
   if ( items.size() < 6 ) return false;

   // Everything checked; store it
   serverData.server     = basicInfo.at( 0 );

   serverData.date       = items.at( 0 ) + " " +
                           items.at( 1 ) + " " +
                           items.at( 2 ) + " " +
                           items.at( 4 );

   serverData.time       = items.at( 3 );
   serverData.tz         = items.at( 5 );

   // "10358266.36" -- whole seconds only
   serverData.uptime     = uptime.section( '.', 0, 0 ).toULongLong();
   serverData.load       = loads;
   serverData.cpuPercent = cpu.toDouble();
   serverData.memPercent = mem.toDouble();

   // Parse interfaces/temperatures
   serverData.tempData.clear();

   // line format: interface;<interface name>,<sensor1>:value1,[<sensor#>:value#...] 
   for ( int i = 1; i < lines.size(); i++ )
   {
      if ( ! lines.at( i ).startsWith( "interface;" ) ) continue;

      // split() always returns at least one element
      QStringList tempData      = lines.at( i ).mid( 10 ).split( ',' );
      QString     interfaceName = tempData.at( 0 );

      for ( int j = 1; j < tempData.size(); j++ )
      {
         // Split at the last ':' in case a sensor label contains one
         int colon = tempData.at( j ).lastIndexOf( ':' );
         if ( colon < 0 ) continue;

         QString key = interfaceName + "," + tempData.at( j ).left( colon );
         serverData.tempData[ key ] = tempData.at( j ).mid( colon + 1 );
      }
   }

   return true;
}

void sysmon3::get_settings( void )
{
   setFrame();  // Off by default

   // Fonts
   font_family = settingsPtr->value( "fontFamily" );
   font_size   = settingsPtr->value( "fontSize"   ).toInt();
   font_normal = QFont( font_family, font_size );

   // Palettes
   QPalette p;
   
   QString lblColor = settingsPtr->value( "labelColor" );
   QString lblBg    = settingsPtr->value( "labelBg"    );
   
   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( lblBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( lblBg    ) );
   
   banner_palette = p;
   
   QString dataColor = settingsPtr->value( "dataColor" );
   QString dataBg    = settingsPtr->value( "dataBg"    ); 
   
   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( dataBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( dataBg    ) );
   
   data_palette = p;
}

void sysmon3::setFrame()
{  
   if ( mFrame )
     setWindowFlags( Qt::Window );  // Turn on frame
   else
     setWindowFlags( Qt::Window | Qt::FramelessWindowHint ); //Frame off
}     
   
void sysmon3::changeFrame()
{     
   mFrame = ! mFrame;
   setFrame();
   show();
}

// banner ( defaults to Bold and changes text colors )
QLabel* sysmon3::banner( const QString& labelString, int fontAdjust, int weight )
{  
  QLabel* newLabel = label( labelString, fontAdjust, weight );

  newLabel->setAlignment ( Qt::AlignCenter );
  newLabel->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  newLabel->setMidLineWidth( 2 );
   
  // Set banner colors
  newLabel->setPalette( banner_palette );
      
  return newLabel;
}  

QLabel* sysmon3::label( const QString& labelString, int fontAdjust, int weight )
{  
   QLabel* newLabel = new QLabel( labelString, this );
   
   newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
   newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignCenter );
   newLabel->setMargin    ( 2 );
   newLabel->setAutoFillBackground( true );
   
   bool bold   = settingsPtr->value( "fontBold" ) == "true";
        weight = bold ? QFont::Bold : QFont::Normal;
   
   QFont labelFont = QFont( font_family, font_size + fontAdjust,  weight );
   
   newLabel->setFont   ( labelFont    );
   newLabel->setPalette( data_palette );
   
   return newLabel;
}

void sysmon3::refresh()
{
   tick++;

   // Get updates.  On failure, the display keeps the last values.
   fetch();

   // Make sure the hostname is set
   lbl_hostname->setText( serverData.server );

   // Update elements
   update_time   ();
   update_date   ();
   update_uptime ();
   update_cpuLoad();
   update_memory ();
   update_temps  ();
}     
   
void sysmon3::update_time()
{
   if ( lbl_time == nullptr ) return;

   QStringList t = serverData.time.split( ":" );
   if ( t.size() < 3 ) return;   // No valid reply yet

   QTime time = QTime( t.at(0).toInt(), t.at(1).toInt(), t.at(2).toInt() );

   // Get the time format 
   QString format = settingsPtr->value( "timeFormat" );

   QString timeString;

   if ( format.contains( QChar( 't' ) ) )
   {
      format.remove( QChar( 't' ) );  // Don't use the client's tz
      format = format.trimmed();      // and remove the space it leaves

      timeString = time.toString( format ) + " " + serverData.tz;
   }
   else
      timeString = time.toString( format );

   // Do the update
   lbl_time->setText( timeString );
}

void sysmon3::update_date()
{
   if ( lbl_date == nullptr ) return;

   // Get the date format
   QString format = settingsPtr->value( "dateFormat" );

   // Create a QDate() structure so it can be formatted
   // Input is  dow, month day, year such ad  "Mon Oct 7 2024"
   QStringList sl = serverData.date.split( " ", Qt::SkipEmptyParts ); 
   if ( sl.size() < 4 ) return;   // No valid reply yet

   int         year = sl.at(3).toInt();
   int         day  = sl.at(2).toInt();

   // sl.at(0) is day of the week (dow)
   // sl.at(1) is 3 character month
   const QStringList months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
   int         month = months.indexOf( sl.at(1) ) + 1;
   QDate       d     = QDate( year, month, day );
   
   QString serverDate  = d.toString( format );
   lbl_date->setText( serverDate );
}

void sysmon3::update_uptime()
{
   if ( lbl_uptime == nullptr ) return;
   if ( tick % 60  != 0       ) return;  // Update once a minute

   uint secs    = serverData.uptime;
   uint minutes = (secs / 60  ) % 60;
   uint hours   = (secs / 3600) % 24;
   uint days    = secs / (3600 * 24);
   
   QString uptimeString = QStringLiteral( "%1d %2:%3" )
      .arg(days)
      .arg(hours,  2, 10, QChar('0') )
      .arg(minutes,2, 10, QChar('0') );

   lbl_uptime->setText( uptimeString );
}

void sysmon3::update_cpuLoad()
{
   if ( lbl_loads != nullptr )
      lbl_loads->setText( serverData.load );

   if ( load != nullptr ) 
      load->setValue( qRound( serverData.cpuPercent * 100 ) );
}

void sysmon3::update_memory()
{
   if ( memory == nullptr ) return;

   memory->setValue( qRound( serverData.memPercent ) );
}

void sysmon3::update_temps()
{
   if ( tempsLayout == nullptr ) return;

   // Update the temperature in column 1 of each row
   for ( int row = 0; row < rowKeys.size(); row++ )
   {
     QLayoutItem* item = tempsLayout->itemAtPosition( row, 1 );
     if ( item == nullptr ) continue;

     QLabel* label = qobject_cast<QLabel*>( item->widget() );
     if ( label != nullptr ) label->setText( tempText( rowKeys.at( row ) ) );
      
   }
}

// Text for one temperature, e.g. "45 °C".  sysmond leaves out a sensor
// it can't read, so show "--" rather than a bare unit.
QString sysmon3::tempText( const QString& key )
{
   QString degC = serverData.tempData.value( key );
   if ( degC.isEmpty() ) degC = "--";

   return degC + QString::fromUtf8( " \u00B0C" ); // degree C
}

void sysmon3::updateLayout()
{  
   timer->stop();  
   delete_all();

   setup_time   ();
   setup_date   ();
   setup_uptime ();
   setup_cpuLoad();
   setup_memory ();
   setup_temps  ();

   refresh();

   int interval = settingsPtr->value( "refreshInterval" ).toInt();

   // Set timer interval in milliseconds
   timer->setInterval( qBound( 1, interval, 9 ) * 1000 );
   timer->start();  
   tick = 0;
}     

void sysmon3::delete_all( void )
{
   timer->stop();  // We do not want to update while changing layout

   // First delete temps
   delete_temps();  
   
   // Now delete all the other entries except the first (hostname)
   QLayoutItem* item;

   while ( ( item = layout->takeAt( 1 ) ) != nullptr )
   {
      delete item->widget();
      delete item;
   }

   //lbl_hostname = nullptr;  We do not delete this
   lbl_time     = nullptr;
   lbl_date     = nullptr;
   lbl_uptime   = nullptr;
   lbl_cpu      = nullptr;
   load         = nullptr; // cpu QProgressBar
   lbl_loads    = nullptr;
   lbl_memory   = nullptr;
   memory       = nullptr; // memory QProgressBar
   lbl_temps    = nullptr;
   tempsLayout  = nullptr; // Deleted with the other layout items
   rowKeys.clear();

}

void sysmon3::updateFont( void )
{
   font_family = settingsPtr->value( "fontFamily" );
   font_size   = settingsPtr->value( "fontSize"   ).toInt();
   font_normal = QFont( font_family, font_size );

   bool bold   = settingsPtr->value( "fontBold" ) == "true";
   QFont::Weight weight = bold ? QFont::Bold : QFont::Normal;
   font_normal.setWeight( weight );

   QFont fontBold = QFont( font_family, font_size, QFont::Bold );

   // Set fonts on all elements
   lbl_hostname->setFont( fontBold );

   if ( lbl_time   != nullptr ) lbl_time  ->setFont( font_normal );
   if ( lbl_date   != nullptr ) lbl_date  ->setFont( font_normal );
   if ( lbl_uptime != nullptr ) lbl_uptime->setFont( font_normal );
   if ( lbl_cpu    != nullptr ) lbl_cpu   ->setFont( fontBold    ); // title
   if ( lbl_loads  != nullptr ) lbl_loads ->setFont( font_normal ); 
   if ( lbl_memory != nullptr ) lbl_memory->setFont( fontBold    ); // title
   if ( lbl_temps  != nullptr ) lbl_temps ->setFont( fontBold    ); // title                                                                     
   // Progress bars
   if ( load       != nullptr ) load      ->setFont( font_normal );
   if ( memory     != nullptr ) memory    ->setFont( font_normal );

   // Need to update fonts for temps 
   if ( tempsLayout != nullptr )
   {
      for ( int i = 0; i < tempsLayout->count(); i++ )
      {
        QWidget* w = tempsLayout->itemAt( i )->widget();
        if ( w != nullptr ) w->setFont( font_normal );

      }
   }
}

void sysmon3::updateColor( void )
{
   set_palettes();
   
   // Set palettes on all elements
   lbl_hostname->setPalette( banner_palette );
   if ( lbl_time   != nullptr ) lbl_time  ->setPalette( data_palette );
   if ( lbl_date   != nullptr ) lbl_date  ->setPalette( data_palette );
   if ( lbl_uptime != nullptr ) lbl_uptime->setPalette( data_palette );
   if ( lbl_cpu    != nullptr ) lbl_cpu   ->setPalette( banner_palette ); // title
   if ( lbl_loads  != nullptr ) lbl_loads ->setPalette( data_palette ); 
   if ( lbl_memory != nullptr ) lbl_memory->setPalette( banner_palette ); // title
   if ( lbl_temps  != nullptr ) lbl_temps ->setPalette( banner_palette ); // title

   // Progress bars
   if ( load       != nullptr ) load      ->setPalette( progress_palette );
   if ( memory     != nullptr ) memory    ->setPalette( progress_palette );

   // Need to update colors for temps 
   if ( tempsLayout != nullptr )
   {
      for ( int i = 0; i < tempsLayout->count(); i++ )
      {
        QWidget* w = tempsLayout->itemAt( i )->widget();
        if ( w != nullptr ) w->setPalette( data_palette );
      }
   }
}

void sysmon3::updateTemp( void )
{
   updateLayout();
   //QMessageBox::information(this, "Test", "sysmon3 received updateTemp" );
}

void sysmon3::delete_temps( void )
{
   if ( tempsLayout == nullptr ) return;

   QLayoutItem* item;
   while ( ( item = tempsLayout->takeAt( 0 ) ) != nullptr )
   {
      delete item->widget();
      delete item;
   }
}

void sysmon3::set_palettes( void )
{
   QPalette p;

   QString lblColor = settingsPtr->value( "labelColor" );
   QString lblBg    = settingsPtr->value( "labelBg" );

   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( lblBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( lblBg    ) );

   banner_palette = p;

   QString dataColor = settingsPtr->value( "dataColor" );
   QString dataBg    = settingsPtr->value( "dataBg" );

   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( dataBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( dataBg    ) );

   data_palette = p;

   QString color = settingsPtr->value( "progressColor" );
   QString bg    = settingsPtr->value( "progressBg" );

   p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
   p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );

   progress_palette = p;
}

