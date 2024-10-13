#ifndef SYSMONQT_H
#define SYSMONQT_H

#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPoint>
#include <QProgressBar>
#include <QSettings>
#include <QListWidget>  // needed?

#include <udp.h>

class sysmon3 : public QMainWindow
{
   Q_OBJECT

public:
   sysmon3( QString );
   ~sysmon3();

   QSettings settings;
   QString   server;  // Used before serverData is populated

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
      quint64 uptime;
      QString load;
      qreal   cpuPercent;
      qreal   memPercent;

      QMap<QString, QString> tempData;
   } serverData;

   sysmonUDP* udp;

   QTimer*    timer;

   QLabel* label    ( const QString&, int =  0, int = QFont::Normal );
   QLabel* banner   ( const QString&, int =  0, int = QFont::Bold );
//   QLabel* textlabel( const QString&, int = -1, int = QFont::Normal );   

   int       tick            = 0;     // tick every second
   bool      mFrame          = false; // turn frame on and off

   QPoint    position;                // location of widget
                                      //
   QLabel*   lbl_hostname;
   QLabel*   lbl_time;     // time of day
   


   QLabel*   lbl_date;     
   QLabel*   lbl_uptime;
   QLabel*   lbl_cpu;      // title
   QLabel*   lbl_loads;    // label for cpu
   QLabel*   lbl_memory;   // title

   QProgressBar* load;
   QProgressBar* memory;

   QString   font_family;
   int       font_size;
   QFont     font_normal;

   QString data;

   QPalette  banner_palette; 
   QPalette  data_palette;
   QPalette  progress_palette;

   void setup_all     ( QString );
   void parse_data    ( void );
   void get_settings  ( QString );

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

   void delete_all   ( void );
   void set_palettes ( void );

   QVBoxLayout* layout;       // Top level layout

   // Temperature info
   QGridLayout*  tempsLayout;
   QStringList   tempConfig;

   void setup_temps  ( void );
   void delete_temps ( void );

   // Maps current label to interface,sensor
   QMap<QString, QString> currentTemps;  

private Q_SLOTS:
   void changeFrame ( void );
   void update      ( void );
   void config      ( void );
   void updateLayout( void );
   //void changeSize  ( void );
};

#endif

