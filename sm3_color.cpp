//! \file sm3_color.cpp
#include "sm3_font.h"
#include "sm3_color.h"

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
   connect( pb_label_color, &QPushButton::clicked, this, &SM_Color::label_color );

   pb_label_background = widgetsPtr->sm_pushbutton( tr( "Change\nLabel Background" ) );
   labelLayout->addWidget( pb_label_background, 0 , 1 );
   connect( pb_label_background, &QPushButton::clicked, this, &SM_Color::label_background );

   sample_label = widgetsPtr->sm_label( "Sample Label" );
   sample_label->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_label, 0, 2 );

   // Data color
   pb_data_color = widgetsPtr->sm_pushbutton( tr( "Change\nData Color" ) );
   labelLayout->addWidget( pb_data_color, 1, 0 );
   connect( pb_data_color, &QPushButton::clicked, this, &SM_Color::data_color );

   pb_data_background = widgetsPtr->sm_pushbutton( tr( "Change\nData Background" ) );
   labelLayout->addWidget( pb_data_background, 1, 1 );
   connect( pb_data_background, &QPushButton::clicked, this, &SM_Color::data_background );

   sample_data = widgetsPtr->sm_label( "Sample Data" );
   sample_data->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_data, 1, 2 );

   // ProgressBar color
   pb_progress_color = widgetsPtr->sm_pushbutton( tr( "Change\nProgress Color" ) );
   labelLayout->addWidget( pb_progress_color, 2, 0 );
   connect( pb_progress_color, &QPushButton::clicked, this, &SM_Color::progress_color );

   pb_progress_background = widgetsPtr->sm_pushbutton( tr( "Change\nProgress Background" ) );
   labelLayout->addWidget( pb_progress_background, 2, 1 );
   connect( pb_progress_background, &QPushButton::clicked, this, &SM_Color::progress_background );

   sample_progress = new QProgressBar();
   sample_progress->setRange( 0, 100 );
   sample_progress->setValue( 75 );
   sample_progress->setFont( QFont( family, fontSize, weight ) );
   labelLayout->addWidget( sample_progress, 2, 2 );

   topbox->addLayout( labelLayout );

   // Default
   pb_default = widgetsPtr->sm_pushbutton( tr( "Set Default Colors" ) );
   connect( pb_default, &QPushButton::clicked, this, &SM_Color::setDefault );
   topbox->addWidget( pb_default );
   
   // Buttons
   pb_apply = widgetsPtr->sm_pushbutton( tr( "Apply" ) );
   connect( pb_apply, &QPushButton::clicked, this, &SM_Color::apply );

   //pb_help = sm_pushbutton( tr( "Help" ) );
   //connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_exit = widgetsPtr->sm_pushbutton( tr( "Exit" ) );
   connect( pb_exit, &QPushButton::clicked, this, &SM_Color::close );

   QBoxLayout* buttons = new QHBoxLayout();

   buttons->addWidget( pb_apply  );
   //buttons->addWidget( pb_help   );
   buttons->addWidget( pb_exit );

   topbox->addLayout( buttons );

   redraw();  // Set colors for examples
}

void SM_Color::label_color        () { pick( sample_label,    QPalette::WindowText ); }
void SM_Color::label_background   () { pick( sample_label,    QPalette::Window     ); }
void SM_Color::data_color         () { pick( sample_data,     QPalette::WindowText ); }
void SM_Color::data_background    () { pick( sample_data,     QPalette::Window     ); }
void SM_Color::progress_color     () { pick( sample_progress, QPalette::Highlight  ); }
void SM_Color::progress_background() { pick( sample_progress, QPalette::Base       ); }

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

void SM_Color::pick( QWidget* sample, QPalette::ColorRole role )
{
   QPalette p        = sample->palette();
   QColor   newColor = QColorDialog::getColor( p.color( role ), this );
   if ( ! newColor.isValid() ) return;   // Canceled

   p.setColor( role, newColor );         // All color groups
   sample->setPalette( p );
}

// Show colors in the three samples, for all color groups
void SM_Color::showColors( const QString& lblFg,  const QString& lblBg,
                           const QString& dataFg, const QString& dataBg,
                           const QString& progFg, const QString& progBg )
{
   QPalette p = sample_label->palette();
   p.setColor( QPalette::WindowText, QColor( lblFg ) );
   p.setColor( QPalette::Window,     QColor( lblBg ) );
   sample_label->setPalette( p );

   p = sample_data->palette();
   p.setColor( QPalette::WindowText, QColor( dataFg ) );
   p.setColor( QPalette::Window,     QColor( dataBg ) );
   sample_data->setPalette( p );

   p = sample_progress->palette();
   p.setColor( QPalette::Highlight, QColor( progFg ) );
   p.setColor( QPalette::Base,      QColor( progBg ) );
   sample_progress->setPalette( p );
}

// Show the saved colors
void SM_Color::redraw( void )
{
   showColors( settingsPtr->value( "labelColor"    ), 
               settingsPtr->value( "labelBg"       ),
               settingsPtr->value( "dataColor"     ), 
               settingsPtr->value( "dataBg"        ),
               settingsPtr->value( "progressColor" ), 
               settingsPtr->value( "progressBg"    ) );
}

// Show the defaults; Apply saves them
void SM_Color::setDefault( void )
{
   showColors( settingsPtr->getDefault( "labelColor"    ), 
               settingsPtr->getDefault( "labelBg"       ),
               settingsPtr->getDefault( "dataColor"     ), 
               settingsPtr->getDefault( "dataBg"        ),
               settingsPtr->getDefault( "progressColor" ), 
               settingsPtr->getDefault( "progressBg"    ) );
}
