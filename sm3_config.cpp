//! \file sm3_config.cpp

#include <QGuiApplication>
#include <QFontDatabase>
#include <QMessageBox>
#include <QScreen>

#include "sysmon3.h"
#include "sm3_config.h"
#include "sm3_font.h"
#include "sm3_temps.h"

SM_Config::SM_Config( SM_Settings* setngs, QString dta, QRect geometry )
{
              settingsPtr = setngs;
              data        = dta;
              parentGeom  = geometry;

   QString    family      = settingsPtr->value( "fontFamily" );
   int        fontSize    = settingsPtr->value( "fontSize"   ).toInt();

   QFont      oldfont     = QFont( family, fontSize );
              widgetsPtr  = new SM_Widgets( oldfont );

   setWindowTitle( "sysmon3 Configuration" );
   setAttribute( Qt::WA_DeleteOnClose );

   // Directories
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setContentsMargins ( 2, 2, 2, 2 );
   topbox->setSpacing         ( 2 );

   version = widgetsPtr->sm_label( "Version: " SMVERSION );
   topbox->addWidget( version );

   int row = 0;
   QGridLayout* mainEntries = new QGridLayout();

   // Refresh interval
   int refresh = settingsPtr->value( "refreshInterval" ).toInt();

   lbl_refresh = widgetsPtr->sm_label( tr( "Refresh Interval:" ) );
   
   SBrefresh   = widgetsPtr->sm_spinBox();
   SBrefresh->setValue( refresh );
   SBrefresh->setRange( 1, 9 );

   PBrefresh   = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBrefresh, &QPushButton::clicked, this, &SM_Config::refresh_help );

   mainEntries->addWidget( lbl_refresh, row,   0 );
   mainEntries->addWidget( SBrefresh,   row,   1 );
   mainEntries->addWidget( PBrefresh,   row++, 2 );

   // time
   QString timeFormat  = settingsPtr->value( "timeFormat" );
   bool    timeChecked = settingsPtr->value( "useTime" ) == "true";

   QGridLayout* CBTimeLayout = 
                   widgetsPtr->sm_checkbox( "time", CBtime, timeChecked );
                LEtime       = widgetsPtr->sm_lineedit( timeFormat );
                PBtime       = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBtime, &QPushButton::clicked, this, &SM_Config::time_help );

   mainEntries->addLayout( CBTimeLayout, row,   0 );
   mainEntries->addWidget( LEtime,       row,   1 );
   mainEntries->addWidget( PBtime,       row++, 2 );

   // date
   QString dateFormat  = settingsPtr->value( "dateFormat" );
   bool    dateChecked = settingsPtr->value( "useDate" ) == "true";

   QGridLayout* CBDateLayout = 
                   widgetsPtr->sm_checkbox( "date", CBdate, dateChecked );
                LEdate       = widgetsPtr->sm_lineedit( dateFormat );
                PBdate       = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBdate, &QPushButton::clicked, this, &SM_Config::date_help );

   mainEntries->addLayout( CBDateLayout, row,   0 );
   mainEntries->addWidget( LEdate,       row,   1 );
   mainEntries->addWidget( PBdate,       row++, 2 );

   // uptime
   bool uptimeChecked = settingsPtr->value( "useUptime" ) == "true";

   QGridLayout* CBuptimeLayout = 
                   widgetsPtr->sm_checkbox( "uptime", CBuptime, uptimeChecked );
                PBuptime       = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBuptime, &QPushButton::clicked, this, &SM_Config::uptime_help );

   mainEntries->addLayout( CBuptimeLayout, row,   0 );
   mainEntries->addWidget( PBuptime,       row++, 2 );

   // CPU Load
   bool CPUchecked = settingsPtr->value( "useCPU"    ) == "true";
   bool CPUbar     = settingsPtr->value( "useCPUbar" ) == "true";

   QGridLayout* CBcpuLayout    = 
                   widgetsPtr->sm_checkbox( "CPU Load", CBcpu, CPUchecked );
   QGridLayout* CBcpuBarLayout = 
                   widgetsPtr->sm_checkbox( "CPU Bar",  CBcpuBar, CPUbar );
                PBcpu          = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBcpu, &QPushButton::clicked, this, &SM_Config::cpu_help );

   mainEntries->addLayout( CBcpuLayout,    row,   0 );
   mainEntries->addLayout( CBcpuBarLayout, row,   1 );
   mainEntries->addWidget( PBcpu,          row++, 2 );

   // Memory display
   bool memoryChecked = settingsPtr->value( "useMemory" ) == "true";

   QGridLayout* CBmemoryLayout = 
                   widgetsPtr->sm_checkbox( "Memory Bar", CBmemory, memoryChecked );
                PBmemory       = widgetsPtr->sm_pushbutton( tr( "Help" ) );

   connect( PBmemory, &QPushButton::clicked, this, &SM_Config::memory_help );

   mainEntries->addLayout( CBmemoryLayout, row,   0 );
   mainEntries->addWidget( PBmemory,       row++, 2 );

   topbox->addLayout( mainEntries );

   // Misc Settings
   row = 0;
   QGridLayout* otherSettings = new QGridLayout();

   // Font Preferences
   lbl_font = widgetsPtr->sm_label(      tr( "Font Preferences:" ) );
   pb_font  = widgetsPtr->sm_pushbutton( tr( "Change Font"       ) );

   otherSettings->addWidget( lbl_font, row,   0 );
   otherSettings->addWidget( pb_font,  row++, 1 );
   connect( pb_font, &QPushButton::clicked, this, &SM_Config::update_font );

   // Color Preferences
   lbl_color = widgetsPtr->sm_label(      tr( "Color Preferences:" ) );
   pb_color  = widgetsPtr->sm_pushbutton( tr( "Change Colors"      ) );

   otherSettings->addWidget( lbl_color, row,   0 );
   otherSettings->addWidget( pb_color,  row++, 1 );
   connect( pb_color, &QPushButton::clicked, this, &SM_Config::update_colors );

   // Temperature Preferences
   lbl_temps = widgetsPtr->sm_label(      tr( "Temperature Preferences:" ) );
   pb_temps  = widgetsPtr->sm_pushbutton( tr( "Change Temps"             ) );

   otherSettings->addWidget( lbl_temps, row,   0 );
   otherSettings->addWidget( pb_temps,  row++, 1 );
   connect( pb_temps, &QPushButton::clicked, this, &SM_Config::update_temps );

   // Pushbuttons
   //pb_help = sm_pushbutton( tr( "Help" ) );
   //connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
   
   pb_apply = widgetsPtr->sm_pushbutton( tr( "Apply" ) );
   connect( pb_apply, &QPushButton::clicked, this, &SM_Config::apply );

   pb_exit = widgetsPtr->sm_pushbutton( tr( "Exit" ) );
   connect( pb_exit, &QPushButton::clicked, this, &SM_Config::close );

   QBoxLayout* buttons = new QHBoxLayout();
   buttons->addWidget( pb_apply  );
   //buttons->addWidget( pb_help   );
   buttons->addWidget( pb_exit );

   topbox->addLayout( otherSettings );
   topbox->addLayout( buttons );
   setLayout( topbox);

   // Set the fonts of all widgets in this dialog (the plain base font)
   update_local();

   // Place the window to the left or right of the parent
   // depending on the position on the screen
   QScreen* scr  = QGuiApplication::screenAt( parentGeom.center() );
   int      left = scr ? scr->availableGeometry().left() : 0;
   int      newx = parentGeom.left() - sizeHint().width() - 100;

   if ( newx < left )
      newx = parentGeom.right() + 100;

   this->move( newx, parentGeom.y() );
}

//void SM_Config::help( void )
//{
//  showhelp.show_help( "config.html" );
//}

// Display refresh help info
void SM_Config::refresh_help( void )
{
   msg_box( "Set this value to the desired refresh interval.\n", 60 );
}

// Display time help info
void SM_Config::time_help( void )
{
   msg_box(
         "Select how time is displayed.\n"
         "Format:\n"
         "h        The hour without a leading zero\n"
         "hh       The hour with a leading zero\n"
         "H        The hour without a leading zero\n"
         "HH       The hour with a leading zero\n"
         "m        The minute without a leading zero\n"
         "mm       The minute with a leading zero\n"
         "s        The whole second, without any leading zero\n"
         "ss       The whole second, with a leading zero\n"
         "AP or A  Use AM/PM display.\n"
         "ap or a  Use am/pm display.\n"
         "t        The timezone (for example 'CEST')", 60 );
}

// Display date help info
void SM_Config::date_help( void )
{
   msg_box(
         "Select how date is displayed.\n"
         "Format:\n"
         "d     The day as a number without a leading zero (1 to 31)\n"
         "dd    The day as a number with a leading zero (01 to 31)\n"
         "ddd   The abbreviated localized day name (e.g. 'Mon' to 'Sun')\n"
         "dddd  The long localized day name (e.g. 'Monday' to 'Sunday').\n" 
         "M     The month as a number without a leading zero (1 to 12)\n"
         "MM    The month as a number with a leading zero (01 to 12)\n"
         "MMM   The abbreviated month name (e.g. 'Jan' to 'Dec').\n"
         "MMMM  The long month name (e.g. 'January' to 'December').\n"
         "yy    The year as a two digit number (00 to 99)\n"
         "yyyy  The year as a four digit number.\n", 60 );
}

// Display cpu help info
void SM_Config::cpu_help( void )
{

   msg_box( "Select whether CPU Load and/or CPU Progress bar is displayed.", 45 );
}

// Display uptime help info
void SM_Config::uptime_help( void )
{
   msg_box( "Select whether uptime is displayed.", 45 );
}

// Display memory usage help info
void SM_Config::memory_help( void )
{
   msg_box( "Select whether memory usage is displayed.", 45 );
}

// Present text in a QMessageBox with specified width in characters
void SM_Config::msg_box( const QString& text, int width )
{
   // Trying to get the message box wider than about 60 characters 
   // does not seem to work
   QFont        font     = QFontDatabase::systemFont(QFontDatabase::FixedFont);
   int          boxWidth = QFontMetrics( font ).averageCharWidth() * width;
   QMessageBox  MB( this );
   QSpacerItem* spacer   = new QSpacerItem( boxWidth, 0, QSizePolicy::Minimum, 
                                            QSizePolicy::Expanding );
   
   MB.setText( text );
   MB.setFont( font );
   QGridLayout* MBlayout = (QGridLayout*)MB.layout();
   MBlayout->addItem( spacer, MBlayout->rowCount(), 0, 1, MBlayout->columnCount() );
   MB.exec();
}

// Relay response from update_colors()
void SM_Config::sendColors( void )
{
   emit updateColors();
}

// Relay response from update_Temps()
void SM_Config::sendTemps( void )
{
   emit updateTemps();
}

// Relay response from update_font()
void SM_Config::sendFonts( void )
{
   // Update fonts here also 
   update_local();
   emit updateFonts();
}

void SM_Config::update_font( void )
{
   SM_Font* font = new SM_Font( settingsPtr ); // owned by config
   font->setParent( this, Qt::Window );        // blocks config
   font->setWindowModality( Qt::WindowModal );  
   font->show();
   connect( font, &SM_Font::updateFonts, this, &SM_Config::sendFonts );
}

void SM_Config::update_colors( void )
{
   SM_Color* colors = new SM_Color( settingsPtr );  
   colors->setParent( this, Qt::Window );        // blocks config
   colors->setWindowModality( Qt::WindowModal );  
   colors->show();
   connect( colors, &SM_Color::updateColors, this, &SM_Config::sendColors );
}

void SM_Config::update_temps( void )
{
   SM3_Temps* temps = new SM3_Temps( settingsPtr, data );  
   temps->setParent( this, Qt::Window );        // blocks config
   temps->setWindowModality( Qt::WindowModal );  
   temps->show();
   connect( temps, &SM3_Temps::updateTemps, this, &SM_Config::sendTemps );
}

void SM_Config::update_local( void )
{
   // Update local widgets
   QFont font = QFont( settingsPtr->value( "fontFamily" ),
                       settingsPtr->value( "fontSize"   ).toInt() );
   
   version ->setFont( font );
   pb_font ->setFont( font );
   pb_color->setFont( font );
   pb_temps->setFont( font );
   pb_apply->setFont( font );
   pb_exit ->setFont( font );

   CBtime  ->setFont( font );
   CBdate  ->setFont( font );
   CBuptime->setFont( font );
   CBcpu   ->setFont( font );
   CBcpuBar->setFont( font );
   CBmemory->setFont( font );

   PBrefresh->setFont( font );
   PBtime  ->setFont( font );
   PBdate  ->setFont( font );
   PBuptime->setFont( font );
   PBcpu   ->setFont( font );
   PBmemory->setFont( font );

   SBrefresh->setFont( font );

   LEtime  ->setFont( font );
   LEdate  ->setFont( font );

   lbl_refresh->setFont( font );
   lbl_font ->setFont( font );
   lbl_color->setFont( font );
   lbl_temps->setFont( font );
}

void SM_Config::apply( void )
{
   settingsPtr->setValue( "refreshInterval", 
                          QString::number( SBrefresh->value() ) );

   settingsPtr->setBoolValue( "useTime",   CBtime->isChecked()   );
   settingsPtr->setBoolValue( "useDate",   CBdate->isChecked()   );
   settingsPtr->setBoolValue( "useUptime", CBuptime->isChecked() );
   settingsPtr->setBoolValue( "useCPU",    CBcpu->isChecked()    );
   settingsPtr->setBoolValue( "useCPUbar", CBcpuBar->isChecked() );
   settingsPtr->setBoolValue( "useMemory", CBmemory->isChecked() );

   settingsPtr->setValue( "timeFormat", LEtime->text() );
   settingsPtr->setValue( "dateFormat", LEdate->text() );
   settingsPtr->sync();

   emit updateEntries();
}

