#ifndef SETUP_H
#define SETUP_H

#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPoint>
#include <QProgressBar>
#include <QSettings>
#include <QListWidget>

class setup : public QFrame
{
   Q_OBJECT

public:
   setup();
   ~setup();

private:
   QVBoxLayout* layout;       // Top level layout
   QListWidget* listBox;
   QLineEdit*   editLine;
   QSettings    settings;


private slots:
   void test       ( void );
   void add        ( void );
   void deleteEntry( void );
   void save       ( void );
   void start      ( void );

signals:
   void showMainWindow( QString );
};
#endif

