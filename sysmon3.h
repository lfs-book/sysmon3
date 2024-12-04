#ifndef SYSMONQT_H
#define SYSMONQT_H

#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPoint>
#include <QProgressBar>
#include <QSettings>
#include <QListWidget>

#include "udp.h"
#include "sm3_settings.h"

class sysmon3 : public QMainWindow
{
   Q_OBJECT

public:
   sysmon3( QString );
   ~sysmon3();

   QString      data;
   SM_Settings* settingsPtr;

public slots:
   void showMain   ( QString );
   void updateFont ( void );
   void updateColor( void );
   void updateTemp ( void );

private:
   void    setFrame();

   struct 
   {
      QString server;
      QString date;
      QString time;
      QString tz;
      quint64 uptime;
      QString load;
      qreal   cpuPercent;
      qreal   memPercent;

      QMap<QString, QString> tempData;
   } serverData;

   //QString    server;  // Used before serverData is populated
   sysmonUDP* udp;

   QTimer*    timer;

   QLabel* label    ( const QString&, int =  0, int = QFont::Normal );
   QLabel* banner   ( const QString&, int =  0, int = QFont::Bold );

   int       tick            = 0;     // tick every loop
   bool      mFrame          = false; // turn frame on and off

   QPoint    position;                // location of widget

   QLabel*   lbl_hostname;
   QLabel*   lbl_time;                // time of day
   


   QLabel*   lbl_date;     
   QLabel*   lbl_uptime;
   QLabel*   lbl_cpu;                 // title
   QLabel*   lbl_loads;               // label for cpu
   QLabel*   lbl_memory;              // title
   QLabel*   lbl_temps;               // label for temperatures

   QProgressBar* load;
   QProgressBar* memory;

   QString   font_family;
   int       font_size;
   QFont     font_normal;

   QPalette  banner_palette; 
   QPalette  data_palette;
   QPalette  progress_palette;

   void setup_all     ( QString );
   void parse_data    ( void );
   void get_settings  ( void );

   void setup_time    ( void );
   void setup_date    ( void );
   void setup_uptime  ( void );
   void setup_cpuLoad ( void );
   void setup_memory  ( void );

   void update_time   ( void );
   void update_date   ( void );
   void update_uptime ( void );
   void update_cpuLoad( void );
   void update_memory ( void );
   void update_temps  ( void );

   void delete_all    ( void );
   void set_palettes  ( void );

   QVBoxLayout* layout;       // Top level layout

   // Temperature info
   QGridLayout*  tempsLayout;

   void setup_temps  ( void );
   void delete_temps ( void );

   // Maps current label to interface,sensor
   QMap<QString, QString> currentTemps;  

private Q_SLOTS:
   void changeFrame ( void );
   void update      ( void );
   void config      ( void );
   void updateLayout( void );
};

#endif

