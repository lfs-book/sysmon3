//! \file sm_font.h
#ifndef SM_FONT_H
#define SM_FONT_H

#include "sm3_widgets.h"
#include "sm3_config.h"
#include "sysmon3.h"

//! Define the height of widgets in pixels.
#define BUTTON_H 26

//!  A class to allow the user to select a customized font.
class SM_Font : public QFrame //, public SM_Widgets
{
  Q_OBJECT
  
  public:
    //*! \brief Construct the window for font selection
    SM_Font( sysmon3* parent = nullptr ); 
    
    //! \brief A null destructor.
    ~SM_Font() {};


  signals:  
    void updateFonts( void );

  private:
    SM_Widgets*  widgets;  // Probably should be private

    QSettings*   settings;
    QString      server;

    QComboBox*   cb_family;
    QSpinBox*    sb_size;

    QCheckBox*   ckbox_bold;
    QGridLayout* ckbox_grid;
    QHBoxLayout* stats_row;

    QLabel*      small;
    QLabel*      regular;
    QLabel*      regularBold;
    QLabel*      large;
    QLabel*      largeBold;
    QLabel*      title;
    QLabel*      lbl_family;
    QLabel*      lbl_size;
    QLabel*      samples;

    QPushButton* pb_font;
    QPushButton* pb_default;
    //QPushButton* pb_help;
    QPushButton* pb_exit;
    QPushButton* pb_apply;


    void redraw( void );

  private slots:
  
    void apply     ( void );
    //void help      ( void );
    void selectFont( void );
    void setDefault( void );
    void update    ( int  );
};
#endif
