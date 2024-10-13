//! \file us_font.h
#ifndef SM_TEMPS_H
#define SM_TEMPS_H

#include "sm3_widgets.h"
#include "sm3_config.h"

//!  A class to allow the user to select a customized font.
class SM3_Temps : public SM_Widgets
{
  Q_OBJECT
  
  public:
    //*! \brief Construct the window for font selection
    SM3_Temps( QString, QSettings* );
    
    //! \brief A null destructor.
    ~SM3_Temps() {};

    QPushButton* pb_apply;

  signals:  
    void updateTemps( void );

  private:
    QSettings*          settings;
    QString             server;

    QGridLayout*        tempsLayout;

    QStringList         sensors;  // <Interface,Device>
    QStringList         config;  

    QPushButton*        pb_exit;
    //QPushButton*        pb_help;

    void get_temp_data( void );
    void get_saved_temp_config( void );

  private slots:
  
    void apply     ( void );
    //void help      ( void );
};
#endif
