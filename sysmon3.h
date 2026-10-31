#ifndef SYSMONQT_H
#define SYSMONQT_H

#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPoint>
#include <QPointer>
#include <QProgressBar>
#include <QSettings>
#include <QListWidget>

#include "udp.h"
#include "sm3_settings.h"

class SM_Config;       // A forward declaration is all that is needed here


class sysmon3 : public QMainWindow
{
   Q_OBJECT

public:
   sysmon3( QString );
   ~sysmon3();

   QString      data;
   SM_Settings* settingsPtr = nullptr;

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
      QString load;
      quint64 uptime     = 0;
      qreal   cpuPercent = 0.0;
      qreal   memPercent = 0.0;

      QMap<QString, QString> tempData;
   } serverData;

   sysmonUDP* udp      = nullptr;
   QTimer*    timer    = nullptr;
   int        failures = 0;           // Consecutive bad or missing replies

   QLabel* label    ( const QString&, int =  0, int = QFont::Normal );
   QLabel* banner   ( const QString&, int =  0, int = QFont::Bold );

   int       tick            = 0;     // tick every loop
   bool      mFrame          = false; // turn frame on and off

   QPoint    position;                // location of widget

   QPointer<SM_Config> configWindow;  // Becomes nullptr when window is deleted

   // Every widget pointer starts as nullptr and is reset to nullptr
   // whenever its widget is deleted, so nullptr always means "not shown".
   QLabel*   lbl_hostname = nullptr;
   QLabel*   lbl_time     = nullptr;  // time of day
   QLabel*   lbl_date     = nullptr;
   QLabel*   lbl_uptime   = nullptr;
   QLabel*   lbl_cpu      = nullptr;  // title
   QLabel*   lbl_loads    = nullptr;  // label for cpu
   QLabel*   lbl_memory   = nullptr;  // title
   QLabel*   lbl_temps    = nullptr;  // title for temperatures

   QProgressBar* load     = nullptr;
   QProgressBar* memory   = nullptr;

   QString   font_family;
   int       font_size = 0;
   QFont     font_normal;

   QPalette  banner_palette; 
   QPalette  data_palette;
   QPalette  progress_palette;

   bool parse_data    ( const QString& );
   void setup_all     ( QString );
   void fetch         ( void );
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

   QString tempText   ( const QString& );

   void delete_all    ( void );
   void set_palettes  ( void );

   QVBoxLayout* layout = nullptr;     // Top level layout

   // Temperature info
   QGridLayout*  tempsLayout = nullptr;

   void setup_temps  ( void );
   void delete_temps ( void );

   // The "interface,sensor" key for each row of tempsLayout
   QStringList  rowKeys;

private Q_SLOTS:
   void changeFrame ( void );
   void refresh     ( void );
   void config      ( void );
   void updateLayout( void );
};

#endif

