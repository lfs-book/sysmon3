#ifndef SETUP_H
#define SETUP_H

#include <QFrame>
#include <QLayout>
#include <QListWidget>
#include <QLineEdit>

class setup : public QFrame
{
   Q_OBJECT

public:
   setup();
   ~setup(){}

private:
   QVBoxLayout* layout;       // Top level layout
   QListWidget* listBox;
   QLineEdit*   editLine;

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

