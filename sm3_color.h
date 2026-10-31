//! \file sm3_color.h
#ifndef SM_COLOR_H
#define SM_COLOR_H

#include <QColorDialog>
#include <QFrame>
#include <QProgressBar>

#include "sm3_widgets.h"
#include "sysmon3.h"

//!  A class to allow the user to select a customized color.
class SM_Color : public QFrame
{
  Q_OBJECT
  
  public:
    //*! \brief Construct the window for color selection
    SM_Color( SM_Settings* );
    
    //! \brief A null destructor.
    ~SM_Color() {}

  signals:  
    void updateColors( void );

  private:
    SM_Settings* settingsPtr;
    SM_Widgets*  widgetsPtr;

    QLabel*      sample_label;
    QLabel*      sample_data;

    QProgressBar* sample_progress;

    QPushButton* pb_label_color;
    QPushButton* pb_label_background;
    QPushButton* pb_data_color;
    QPushButton* pb_data_background;
    QPushButton* pb_progress_color;
    QPushButton* pb_progress_background;

    QPushButton* pb_apply;
    QPushButton* pb_default;
    QPushButton* pb_exit;

    void redraw    ( void );
    void pick      ( QWidget*, QPalette::ColorRole );
    void showColors( const QString&, const QString&,
                     const QString&, const QString&,
                     const QString&, const QString& );
  private slots:
  
    void label_color        ( void );
    void label_background   ( void );
    void data_color         ( void );
    void data_background    ( void );
    void progress_color     ( void );
    void progress_background( void );

    void apply              ( void );
    void setDefault         ( void );
//    void update_colors      ( int  );
};
#endif
