#include <QApplication>
#include <QAction>
#include <QTimer>
#include <QMessageBox>

#include "sysmon3.h"
#include "sm3_config.h"
#include "setup.h"
#include "udp.h"

int main(int argc, char *argv[])
{  
   QApplication a(argc, argv);
   QCoreApplication::setOrganizationName("LinuxFromScratch");
   QCoreApplication::setApplicationName("sysmon3");
   QCoreApplication::setApplicationVersion(QT_VERSION_STR);

   QString arg1;
   if ( argc > 1 ) arg1 = argv[ 1 ];

   sysmon3 mainWin( arg1 );
   // We don't show yet

   return a.exec();
}  

sysmon3::sysmon3( QString arg1 )
{
   setContextMenuPolicy( Qt::ActionsContextMenu );  // Add actions for right click
   setWindowTitle( tr( "sysmon3" ) );

   // actions 
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

//   QAction* sizeAction = new QAction( tr("R&esize"), this );
//   sizeAction->setShortcut( tr("Ctrl+R") );
//   connect( sizeAction, &QAction::triggered, this, &sysmon3::changeSize );
//   addAction( sizeAction );

   timer = new QTimer( this );
   timer->setInterval( 1000 );  // Every second
   connect(timer, &QTimer::timeout, this, QOverload<>::of(&sysmon3::update));
   //timer->start();  // Do this later

   bool runSetup = true;

   if ( ! arg1.isEmpty() )
   {
      // If server has been set up, don't run setup()
      int size = settings.beginReadArray( "hosts" );
      if ( size > 0 )
      {
         for ( int i = 0; i < size; i++ )
         {
            settings.setArrayIndex( i );
            if ( settings.value( "hosts" ).toString() == arg1 )
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
     connect( s, SIGNAL( showMainWindow( QString ) ), this, SLOT( showMain( QString ) ) );
   }
   else 
      showMain( arg1 );
}

void sysmon3::config( void )
{  
   SM_Config* w = new SM_Config( server, &settings, this );
   w->show();
   connect( w, SIGNAL( updateEntries() ), this, SLOT( updateLayout() ) );
   connect( w, SIGNAL( updateFonts  () ), this, SLOT( updateFont()   ) );
   connect( w, SIGNAL( updateColors () ), this, SLOT( updateColor()  ) );
   connect( w, SIGNAL( updateTemps  () ), this, SLOT( updateTemp()   ) );
}

sysmon3::~sysmon3()
{
   position = this->pos();
   settings.setValue( server + "-positionX", QString::number( position.x() ) );
   settings.setValue( server + "-positionY", QString::number( position.y() ) );
   settings.sync();
}

// This is a SLOT, but perhaps the slot should just be setup_all
void sysmon3::showMain( QString server1 )
{
   server = server1;

   // Start layout   
   layout = new QVBoxLayout; 

   // hostname  we don't have the name yet
   lbl_hostname = banner( serverData.server, 0, QFont::Bold );
   lbl_hostname->setPalette( banner_palette );
   layout->addWidget( lbl_hostname );

   setup_all( server );
}

void sysmon3::setup_all( QString server )
{
   udp  = new sysmonUDP( &server );
   data = udp->getData();

   parse_data();
//qDebug() << "1111";
   // Use server name to initialize saved data
   //get_settings( serverData.server );
   get_settings( server );

   // Workaround...
   lbl_hostname->setText( serverData.server );
   lbl_hostname->setPalette( banner_palette );

   setup_time();
   setup_date();
   setup_uptime();
   setup_cpuLoad();
   setup_memory();
//qDebug() << "1112";
   setup_temps();
//qDebug() << "1113";

   updateFont();
//qDebug() << "1114";

   // End of layout

   QWidget* window = new QWidget();
   window->setLayout( layout );

   setCentralWidget( window );

   if ( settings.contains( server + "-positionX" ) )
   {
      int x = settings.value( server + "-positionX" ).toInt();
      int y = settings.value( server + "-positionY" ).toInt();
      position = QPoint( x, y );
      this->move( position );
   }

   this->show();
   timer->start();
   tick = 0;
}

void sysmon3::setup_time()
{
   lbl_time = nullptr;

   if ( ! settings.value( server + "-useTime", true ).toBool() ) return;
   lbl_time = label ( serverData.time );
   layout->addWidget( lbl_time );
}

void sysmon3::setup_date()
{
   lbl_date = nullptr;
   if ( ! settings.value( server + "-useDate", true ).toBool() ) return;

   lbl_date = label( serverData.date );
   layout->addWidget( lbl_date );
}

void sysmon3::setup_uptime()
{
   lbl_uptime = nullptr;

   if ( ! settings.value( server + "-useUptime", true ).toBool() ) return;

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
   bool useCPU    = settings.value( server + "-useCPU",    true ).toBool();
   bool useCPUbar = settings.value( server + "-useCPUbar", true ).toBool();

   lbl_cpu = nullptr;

   if ( useCPU )
   {
      lbl_loads = label( serverData.load );
      layout->addWidget( lbl_loads );
   }

   if ( useCPUbar )
   {
      load = new QProgressBar();
      load->setRange( 0, 100 );
      load->setValue( serverData.cpuPercent * 100 );
      load->setFont( font_normal );

      // Set palette
      QPalette p = load->palette();

      QString color = settings.value( server + "-progressColor", "#308cc6" ).toString();
      QString bg    = settings.value( server + "-progressBg",    "#ffffff" ).toString();

      p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
      p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
      p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
      p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );
      load->setPalette( p );

      layout->addWidget( load );
   }

   lbl_cpu = banner( "CPU Load", 0, QFont::Bold );
   layout->addWidget( lbl_cpu   );
}

void sysmon3::setup_memory()
{
   lbl_memory = nullptr;
   if ( ! settings.value( server + "-useMemory", true ).toBool() ) return;

   memory = new QProgressBar();
   memory->setRange( 0, 100 );
   memory->setFont ( font_normal );
   memory->setValue( qRound( serverData.memPercent ) );

   // set palette for memory
   QPalette p = memory->palette();

   QString color = settings.value( server + "-progressColor", "#308cc6" ).toString();
   QString bg    = settings.value( server + "-progressBg",    "#ffffff" ).toString();

   p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
   p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );
   memory->setPalette( p );

   lbl_memory = banner( "Memory Use", 0, QFont::Bold );
   layout->addWidget( memory );
   layout->addWidget( lbl_memory );
}

void sysmon3::setup_temps()
{
   tempConfig.clear();
   tempsLayout = nullptr;

   // Get temperature settings in the format 'interface + "," + sensor + "," + label'
   QString group = QString( server + "-temperatures" );
   
   settings.beginGroup( group );
     QStringList keys = settings.childKeys();
     foreach (const QString &key, keys)
       tempConfig << settings.value( key ).toString();
   settings.endGroup();

   // If null, return
   if ( tempConfig.size() == 0 ) return;  // tempConfig is a QStringList

   // Create tempsLayout 
   tempsLayout = new QGridLayout();

   // Populate tempsLayout
   for ( int i = 0; i < tempConfig.size(); i++ )
   {
      QString      t    = tempConfig[ i ];  // interface,sensor,label
      QStringList  sl   = tempConfig[ i ].split( ',' );
      QString      key  = sl[ 0 ] + "," + sl[ 1 ];
      QString      degC = serverData.tempData[ key ];

      // We also need to be able to map label to key for updates
      QString displayLabel = sl[ 2 ];
      currentTemps[ displayLabel ] = key; 

      tempsLayout->addWidget( label( sl[ 2 ] ), i, 0 ); // Label name
      tempsLayout->addWidget( label( degC    ), i, 1 ); // temperature in C
   }

   // Add to main layout
   layout->addLayout( tempsLayout );
}

void sysmon3::parse_data()
{
   if ( data == "Timeout" )
   {
      QMessageBox msgBox;
      msgBox.setText( "The systemd server does not seem to be running." );
      msgBox.exec();

      exit( EXIT_FAILURE );
   }
   
   QStringList lines     = data.split( '\n', Qt::SkipEmptyParts );
   QStringList basicInfo = lines.at( 0 ).split( ';' );

   serverData.server     = basicInfo.at( 0 );
   
   // Parse "time:Sun Oct  6 14:31:31 2024"
   QString s             = basicInfo.at( 1 );
   QStringList items     = s.split( " ", Qt::SkipEmptyParts);

   QString dom           = items.at( 0 );
   dom.replace(0, 5, "");                  // remove "time:"

   serverData.date       = dom           + " " + 
                           items.at( 1 ) + " " + 
                           items.at( 2 ) + " " +
                           items.at( 4 );

   serverData.time       = items.at( 3 );

   // Parse "uptime:10358266.36";
   QString     seconds   = basicInfo.at( 2 ).split( ':' ).at( 1 );;
   serverData.uptime     = seconds.split( '.' ).at( 0 ).toLongLong();

   // Parse "load:0.36 0.33 0.34"
   serverData.load       = basicInfo.at( 3 ).split( ':' ).at( 1 );

   // Parse "cpu%:0.02" 
   serverData.cpuPercent = basicInfo.at( 4 ).split( ':' ).at( 1 ).toFloat();

   // Parse "mem%:13.20"
   serverData.memPercent = basicInfo.at( 5 ).split( ':' ).at( 1 ).toFloat();

   // Parse interfaces/temperatures
   serverData.tempData.clear();

   // line format: interface;<interface name>,<sensor1>:value1,[<sensor#>:value#...] 
   for ( int i = 1; i < lines.size(); i++ )
   {
      QStringList interface     = lines[ i ].split( ';' );
      if ( interface[ 0 ] != "interface" )
      {
         //qDebug() << "++++Trying to parse line[" << i << "] ->" << lines[ i ]; 
         continue;
      }
      QStringList tempData      = interface[ 1 ].split( ',' );
      QString     interfaceName = tempData[ 0 ];
      for ( int j = 1; j < tempData.size(); j++ )
      {
         QStringList sensorData     = tempData[ j ].split( ':' );
         QString     key            = interfaceName + "," + sensorData[ 0 ];
         serverData.tempData[ key ] = sensorData[ 1 ];
      }
   }
}

void sysmon3::get_settings( QString server )
{
   setFrame();  // Off by default

   // Fonts
   font_family = settings.value( server + "-fontFamily", "DejaVu Sans" ).toString();
   font_size   = settings.value( server + "-fontSize"  , 12 ).toInt();
   font_normal = QFont( font_family, font_size );

   // Palettes
   QPalette p;
   
   QString lblColor = settings.value( server + "-labelColor", "#ffffff" ).toString();
   QString lblBg    = settings.value( server + "-labelBg",    "#999999" ).toString();
//qDebug() << "server: " << server << "; lblColor: " << lblColor << "; lblBg:" << lblBg;
   
   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( lblBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( lblBg    ) );
   
   banner_palette = p;
   
   QString dataColor = settings.value( server + "-dataColor", "#000000" ).toString();
   QString dataBg    = settings.value( server + "-dataBg",    "#efefef" ).toString();
   
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

//void sysmon3::changeSize()
//{     
//   mFrame = ! mFrame;
//   setFixedSize( 500, 500 );
//   show();
//}

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
   
   bool bold   = settings.value( server + "+fontBold", false ).toBool();
        weight = bold ? QFont::Bold : QFont::Normal;
   
   QFont labelFont = QFont( font_family, font_size + fontAdjust,  weight );
   
   newLabel->setFont   ( labelFont    );
   newLabel->setPalette( data_palette );
   
   return newLabel;
}

void sysmon3::update()
{
   tick++;

   // Get updates
   data = udp->getData();

   // Parse and store
   parse_data();

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

   QStringList t    = serverData.time.split( ":" );
   QTime       time = QTime( t.at(0).toInt(), t.at(1).toInt(), t.at(2).toInt() );

   // Get the time format 
   QString format = settings.value( server + "-timeFormat", "HH:mm:ss" ).toString();
   
   // Do the update
   lbl_time->setText( time.toString( format ) );
}

void sysmon3::update_date()
{
   if ( lbl_date == nullptr ) return;

   // Get the date format
   QString format = settings.value( server + "-dateFormat", "ddd d MMM" ).toString();

   // Create a QDate() structure so it can be formatted
   // Input is  dow, month day, year such ad  "Mon Oct 7 2024"
   QStringList sl = serverData.date.split( " ", Qt::SkipEmptyParts ); 
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
   bool useCPU    = settings.value( server + "-useCPU",    true ).toBool();
   bool useCPUbar = settings.value( server + "-useCPUbar", true ).toBool();

   if ( useCPU )
      lbl_loads->setText( serverData.load );

   if ( useCPUbar ) 
      load->setValue( serverData.cpuPercent * 100 );
}

void sysmon3::update_memory()
{
   if ( memory == nullptr ) return;

   memory->setValue( serverData.memPercent );
}

void sysmon3::update_temps()
{
   if ( tempsLayout == nullptr ) return;

   // For each QGridBox row
   for ( int i = 0; i < tempsLayout->rowCount(); i++ )
   {
     // Get displayLabel text
     QLayoutItem* item  = tempsLayout->itemAtPosition( i, 0 );
     QLabel*      label = qobject_cast<QLabel*>( item->widget() );
     QString      text  = label->text();

     // Update temp at position ( i, 1 )
     QString      key   = currentTemps[ text ];
     QString      degC  = serverData.tempData[ key ];
                  item  = tempsLayout->itemAtPosition( i, 1 );
                  label = qobject_cast<QLabel*>( item->widget() );
                  label->setText( degC );
   }
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

   update();
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
}

void sysmon3::updateFont( void )
{
   font_family = settings.value( server + "-fontFamily", "DejaVu Sans" ).toString();
   font_size   = settings.value( server + "-fontSize"  , 12 ).toInt();
   font_normal = QFont( font_family, font_size );
//qDebug() << "2222";
   bool bold   = settings.value( server + "-fontBold"  , false ).toBool();
   QFont::Weight weight = bold ? QFont::Bold : QFont::Normal;
   font_normal.setWeight( weight );

   QFont fontBold = QFont( font_family, font_size, QFont::Bold );

//qDebug() << "2223";
   // Set fonts on all elements
   lbl_hostname->setFont( fontBold );
//qDebug() << "222a";
   if ( lbl_time   != nullptr ) lbl_time  ->setFont( font_normal );
   if ( lbl_date   != nullptr ) lbl_date  ->setFont( font_normal );
   if ( lbl_uptime != nullptr ) lbl_uptime->setFont( font_normal );
   if ( lbl_cpu    != nullptr ) lbl_cpu   ->setFont( fontBold    ); // title
   if ( lbl_loads  != nullptr ) lbl_loads ->setFont( font_normal ); 
   if ( lbl_memory != nullptr ) lbl_memory->setFont( fontBold    ); // title
//qDebug() << "2224";
   // Progress bars
   if ( load       != nullptr ) load      ->setFont( font_normal );
   if ( memory     != nullptr ) memory    ->setFont( font_normal );

//qDebug() << "2224";
   // Need to update fonts for temps 
   if ( tempsLayout != nullptr )
   {
      for ( int i = 0; i < tempsLayout->rowCount(); i++ )
      {
        QLayoutItem* item  = tempsLayout->itemAtPosition( i, 0 );
        QLabel*      label = qobject_cast<QLabel*>( item->widget() );
        label->setFont( font_normal );

                     item  = tempsLayout->itemAtPosition( i, 1 );
                     label = qobject_cast<QLabel*>( item->widget() );
        label->setFont( font_normal );
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

   // Progress bars
   if ( load       != nullptr ) load      ->setPalette( progress_palette );
   if ( memory     != nullptr ) memory    ->setPalette( progress_palette );

   // Need to update colors for temps 
   if ( tempsLayout != nullptr )
   {
      for ( int i = 0; i < tempsLayout->rowCount(); i++ )
      {
        QLayoutItem* item  = tempsLayout->itemAtPosition( i, 0 );
        QLabel*      label = qobject_cast<QLabel*>( item->widget() );
        label->setPalette( data_palette );

                     item  = tempsLayout->itemAtPosition( i, 1 );
                     label = qobject_cast<QLabel*>( item->widget() );
        label->setPalette( data_palette );
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

   QString lblColor = settings.value( server + "-labelColor", "#ffffff" ).toString();
   QString lblBg    = settings.value( server + "-labelBg",    "#999999" ).toString();
//qDebug() << "server: " << server << " lblColor: "  << lblColor;
   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( lblBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( lblColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( lblBg    ) );

   banner_palette = p;

   QString dataColor = settings.value( server + "-dataColor", "#000000" ).toString();
   QString dataBg    = settings.value( server + "-dataBg",    "#efefef" ).toString();

   p.setColor( QPalette::Active,   QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Active,   QPalette::Window,     QColor( dataBg    ) );
   p.setColor( QPalette::Inactive, QPalette::WindowText, QColor( dataColor ) );
   p.setColor( QPalette::Inactive, QPalette::Window,     QColor( dataBg    ) );

   data_palette = p;

   QString color = settings.value( server + "-progressColor", "#308cc6" ).toString();
   QString bg    = settings.value( server + "-progressBg",    "#ffffff" ).toString();

   p.setColor( QPalette::Active,   QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Active,   QPalette::Base,      QColor( bg    ) );
   p.setColor( QPalette::Inactive, QPalette::Highlight, QColor( color ) );
   p.setColor( QPalette::Inactive, QPalette::Base,      QColor( bg    ) );

   progress_palette = p;
}

