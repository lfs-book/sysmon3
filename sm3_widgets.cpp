//! \file us_widgets.cpp

#include "sm3_widgets.h"

SM_Widgets::SM_Widgets()
{
   //font = QFont( settings->value( server + "-fontFamily", "DejaVu Sans" ).toString(), 
   //              settings->value( server + "-fontSize"  , 12 ).toInt() );
}

// Work around getting server and settings for setting font
// This needs to be called first in the classes that depend on it
void SM_Widgets::setWidgetData( QString system, QSettings* baseSettings )
{
   server   = system;
   settings = baseSettings;

   font = QFont( settings->value( server + "-fontFamily", "DejaVu Sans" ).toString(), 
                 settings->value( server + "-fontSize"  , 12 ).toInt() );
}


// label
QLabel* SM_Widgets::sm_label( const QString& labelString, int fontAdjust, QFont::Weight weight )
{
   QLabel* newLabel = new QLabel( labelString, this );

   newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
   newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
   newLabel->setMargin    ( 2 );
   newLabel->setAutoFillBackground( true );

   QFont thisFont = font;
   thisFont.setPointSize( font.pointSize() + fontAdjust );
   thisFont.setWeight   ( weight );

   newLabel->setFont( thisFont );

   return newLabel;
}

// textlabel ( fontAdjust defaults to smaller font (-1) if not specified )
QLabel* SM_Widgets::sm_textlabel( const QString& labelString, int fontAdjust, QFont::Weight weight )
{
   return sm_label( labelString, fontAdjust, weight );
}

// banner ( defaults to Bold if weight not specified )
QLabel* SM_Widgets::sm_banner( const QString& labelString, int fontAdjust, QFont::Weight weight )
{
   QLabel* newLabel = sm_label( labelString, fontAdjust, weight );

   newLabel->setAlignment ( Qt::AlignCenter );
   newLabel->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
   newLabel->setMidLineWidth( 2 );
   newLabel->setFont( font );

   return newLabel;
}

// pushbutton
QPushButton* SM_Widgets::sm_pushbutton( const QString& labelString, bool enabled,
                                        int fontAdjust )
{
   QPushButton* button = new QPushButton( tr( labelString.toLatin1() ), this );

   QFont thisFont = font;
   thisFont.setPointSize( font.pointSize() + fontAdjust );
   button->setFont( thisFont );

   QString style = "border: 4px outset gray; border-radius: 10px;";
   button->setStyleSheet( style );

   button->setAutoDefault( false );
   button->setEnabled( enabled );

   return button;
}

// SpinBox
QSpinBox* SM_Widgets::sm_spinBox( const int fontAdjust )
{
   QSpinBox* sbox = new QSpinBox( this );

   QFont thisFont = font;
   thisFont.setPointSize( font.weight() + fontAdjust );

   sbox->setAutoFillBackground( true );
   sbox->setFont( thisFont );

   return sbox;
}

// Combo Box
QComboBox* SM_Widgets::sm_comboBox( void )
{
  QComboBox* cb = new QComboBox( this );

  cb->setAutoFillBackground( true );
  cb->setFont( font );
                     
  return cb;
}

// checkbox
QGridLayout* SM_Widgets::sm_checkbox( 
      const QString& text, QCheckBox*& cb, bool state )
{
   QFont thisFont = font;
   thisFont.setWeight( QFont::Bold );

   QFontMetrics fm( font );

   QLabel* lb_spacer = new QLabel;

   // Space as wide as a 'w'
   lb_spacer->setFixedWidth        ( fm.horizontalAdvance( "w" ) ); 
   lb_spacer->setAutoFillBackground( true );

   cb = new QCheckBox( text.toLatin1(), this );
   cb->setFont              ( thisFont  );
   cb->setChecked           ( state );
   cb->setAutoFillBackground( true  );

   QGridLayout* layout = new QGridLayout;
   layout->setContentsMargins( 0, 0, 0, 0 );
   layout->setSpacing        ( 0 );

   layout->addWidget( lb_spacer, 0, 0 );
   layout->addWidget( cb       , 0, 1 );

   return layout;
}

// lineedit
QLineEdit* SM_Widgets::sm_lineedit( const QString& text, int fontAdjust,
      bool readonly )
{
   QLineEdit* le = new QLineEdit( this );

   if ( readonly ) fontAdjust++;  

   QFont thisFont = font;
   thisFont.setPointSize( font.pointSize() + fontAdjust );
  
   le->setFont              ( thisFont );
   le->insert               ( text );
   le->setAutoFillBackground( true );
   le->show();

   return le;
}

