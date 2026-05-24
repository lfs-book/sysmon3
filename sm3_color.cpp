//! \file sm_font.cpp
#include "sysmon3.h"
#include "sm3_font.h"

SM_Color::SM_Color( SM_Settings* setngs )
{
              settingsPtr = setngs;

   QString    family      = settingsPtr->value( "fontFamily" );
   int        fontSize    = settingsPtr->value( "fontSize"   ).toInt();

   QFont      oldfont     = QFont( family, fontSize );
              widgetsPtr  = new SM_Widgets( oldfont );

   // Get current font; second parameter is default
   bool    checked = settingsPtr->value( "fontBold" ) == "true";
   int     weight  = checked ? QFont::Bold : QFont::Normal;

   // Frame layout
   setWindowTitle( "Color Selection Dialog" );
   setAttribute( Qt::WA_DeleteOnClose );
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   // Label color
   QGridLayout* labelLayout = new QGridLayout();

   pb_label_color = widgetsPtr->sm_pushbutton( tr( "Change\nLabel Color" ) );
   labelLayout->addWidget( pb_label_color, 0, 0 );
   connect( pb_label_color, SIGNAL( clicked() ), SLOT( label_color() ) );

   pb_label_background = widgetsPtr->sm_pushbutton( tr( "Change\nLabel Background" ) );
   labelLayout->addWidget( pb_label_background, 0 , 1 );
   connect( pb_label_background, SIGNAL( clicked() ), SLOT( label_background() ) );

   sample_label = widgetsPtr->sm_label( "Sample Label" );
   sample_label->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_label, 0, 2 );

   // Data color
   pb_data_color = widgetsPtr->sm_pushbutton( tr( "Change\nData Color" ) );
   labelLayout->addWidget( pb_data_color, 1, 0 );
   connect( pb_data_color, SIGNAL( clicked() ), SLOT( data_color() ) );

   pb_data_background = widgetsPtr->sm_pushbutton( tr( "Change\nData Background" ) );
   labelLayout->addWidget( pb_data_background, 1, 1 );
   connect( pb_data_background, SIGNAL( clicked() ), SLOT( data_background() ) );

   sample_data = widgetsPtr->sm_label( "Sample Data" );
   sample_data->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_data, 1, 2 );

   // ProgressBar color
   pb_progress_color = widgetsPtr->sm_pushbutton( tr( "Change\nProgress Color" ) );
   labelLayout->addWidget( pb_progress_color, 2, 0 );
   connect( pb_progress_color, SIGNAL( clicked() ), SLOT( progress_color() ) );

   pb_progress_background = widgetsPtr->sm_pushbutton( tr( "Change\nProgress Background" ) );
   labelLayout->addWidget( pb_progress_background, 2, 1 );
   connect( pb_progress_background, SIGNAL( clicked() ), SLOT( progress_background() ) );

   sample_progress = new QProgressBar();
   sample_progress->setRange( 0, 100 );
   sample_progress->setValue( 75 );
   sample_progress->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_progress, 2, 2 );

   topbox->addLayout( labelLayout );

   // Default
   pb_default = widgetsPtr->sm_pushbutton( tr( "Set Default Colors" ) );
   connect( pb_default, SIGNAL( clicked() ), SLOT( setDefault() ) );
   topbox->addWidget( pb_default );
   
   // Buttons
   pb_apply = widgetsPtr->sm_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );

   //pb_help = sm_pushbutton( tr( "Help" ) );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_exit = widgetsPtr->sm_pushbutton( tr( "Exit" ) );
   connect( pb_exit, SIGNAL( clicked() ), SLOT( close() ) );

   QBoxLayout* buttons = new QHBoxLayout();

   buttons->addWidget( pb_apply  );
   //buttons->addWidget( pb_help   );
   buttons->addWidget( pb_exit );

   topbox->addLayout( buttons );

   redraw();  // Set colors for examples
}

void SM_Color::setDefault( void )
{
   QPalette p = sample_label->palette();

   //  Get defaults from SM_Settings
   QColor labelColor    = QColor( settingsPtr->getDefault( "labelColor"    ) );
   QColor labelBg       = QColor( settingsPtr->getDefault( "labelBg"       ) );
   QColor dataColor     = QColor( settingsPtr->getDefault( "dataColor"     ) );
   QColor dataBg        = QColor( settingsPtr->getDefault( "dataBg"        ) );
   QColor progressColor = QColor( settingsPtr->getDefault( "progressColor" ) );
   QColor progressBg    = QColor( settingsPtr->getDefault( "progressBg"    ) );

   // The Active group is used for the window that has keyboard focus.
   p.setColor( QPalette::Active,   QPalette::Window,     dataBg        ); 
   p.setColor( QPalette::Active,   QPalette::WindowText, dataColor     ); 
   p.setColor( QPalette::Active,   QPalette::Highlight,  progressColor ); 
   p.setColor( QPalette::Active,   QPalette::Base,       progressBg    );

   //The Inactive group is used for other windows.
   p.setColor( QPalette::Inactive, QPalette::Window,     dataBg        );
   p.setColor( QPalette::Inactive, QPalette::WindowText, dataColor     );
   p.setColor( QPalette::Inactive, QPalette::Highlight,  progressColor );
   p.setColor( QPalette::Inactive, QPalette::Base,       progressBg    );

   sample_data    ->setPalette( p );
   sample_progress->setPalette( p );

   p.setColor( QPalette::Active,   QPalette::Window,     labelBg       ); 
   p.setColor( QPalette::Active,   QPalette::WindowText, labelColor    ); 

   p.setColor( QPalette::Inactive, QPalette::Window,     labelBg       );
   p.setColor( QPalette::Inactive, QPalette::WindowText, labelColor    );

   sample_label ->setPalette( p );
}

void SM_Color::label_color( void )
{
   QPalette p = sample_label->palette();
                                      
   QColor oldColor = p.color( QPalette::Active, QPalette::WindowText);
   QColor newColor = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::WindowText, newColor );
   p.setColor( QPalette::Inactive, QPalette::WindowText, newColor );
   sample_label->setPalette( p );
}

void SM_Color::label_background( void )
{
   QPalette p = sample_label->palette();

   // Background is QPalette::Window
   QColor oldColor = p.color( QPalette::Active, QPalette::Window);
   QColor newColor = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::Window, newColor );
   p.setColor( QPalette::Inactive, QPalette::Window, newColor );
   sample_label->setPalette( p );
}

void SM_Color::data_color( void )
{
   QPalette p = sample_data->palette();
   
   QColor oldColor  = p.color( QPalette::Active, QPalette::WindowText);
   QColor newColor  = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::WindowText, newColor );
   p.setColor( QPalette::Inactive, QPalette::WindowText, newColor );
   sample_data->setPalette( p );
}

void SM_Color::data_background( void )
{
   QPalette p = sample_data->palette();
  
   QColor oldColor  = p.color( QPalette::Active, QPalette::Window);
   QColor newColor  = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::Window, newColor );
   p.setColor( QPalette::Inactive, QPalette::Window, newColor );
   sample_data->setPalette( p );
}

void SM_Color::progress_color( void )
{
   QPalette p = sample_progress->palette();
 
   QColor oldColor  = p.color( QPalette::Active, QPalette::WindowText);
   QColor newColor  = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::Highlight, newColor );
   p.setColor( QPalette::Inactive, QPalette::Highlight, newColor );
   sample_progress->setPalette( p );
}

void SM_Color::progress_background( void )
{
   QPalette p = sample_progress->palette();

   QColor oldColor  = p.color( QPalette::Active, QPalette::Window);
   QColor newColor  = QColorDialog::getColor(oldColor, this );

   if ( ! newColor.isValid() ) return;

   p.setColor( QPalette::Active,   QPalette::Base, newColor );
   p.setColor( QPalette::Inactive, QPalette::Base, newColor );
   sample_progress->setPalette( p );
}

void SM_Color::update( int /* unused */ )
{
   redraw();
}

void SM_Color::apply()
{
   QPalette p          = sample_label->palette();
   QColor   text       = p.color( QPalette::Active, QPalette::WindowText );
   QColor   background = p.color( QPalette::Active, QPalette::Window );

   // Set color values in settingsPtr
   settingsPtr->setValue( "labelColor", text      .name() );
   settingsPtr->setValue( "labelBg"   , background.name() );

   p          = sample_data->palette();
   text       = p.color( QPalette::Active, QPalette::WindowText );
   background = p.color( QPalette::Active, QPalette::Window );

   settingsPtr->setValue( "dataColor", text.name()      );
   settingsPtr->setValue( "dataBg"   , background.name() );

   p          = sample_progress->palette();
   text       = p.color( QPalette::Active, QPalette::Highlight );
   background = p.color( QPalette::Active, QPalette::Base );

   settingsPtr->setValue( "progressColor", text.name()      );
   settingsPtr->setValue( "progressBg"   , background.name() );

   settingsPtr->sync();
   emit updateColors();
}

//void SM_Font::help()
//{
// US_Help* help = new US_Help();
// help->show_help( "manual/usfont.html" );
//}

void SM_Color::redraw( void )
{
   QString lblFg      = settingsPtr->value( "labelColor"    );
   QString lblBg      = settingsPtr->value( "labelBg"       );
                                                          
   QString dataFg     = settingsPtr->value( "dataColor"     );
   QString dataBg     = settingsPtr->value( "dataBg"        );

   QString progressFg = settingsPtr->value( "progressColor" );
   QString progressBg = settingsPtr->value( "progressBg"    );

   QPalette p;
   p.setColor( QPalette::Active, QPalette::WindowText, QColor( lblFg ) );
   p.setColor( QPalette::Active, QPalette::Window,     QColor( lblBg ) );

   sample_label->setPalette( p );

   p.setColor( QPalette::Active, QPalette::WindowText, QColor( dataFg ) );
   p.setColor( QPalette::Active, QPalette::Window,     QColor( dataBg ) );

   sample_data->setPalette( p );

   p.setColor( QPalette::Active, QPalette::Highlight, QColor( progressFg ) );
   p.setColor( QPalette::Active, QPalette::Base,      QColor( progressBg ) );

   sample_progress->setPalette( p );

   this->repaint();
}
