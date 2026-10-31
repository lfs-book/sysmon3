//! \file sm_widgets.h
#ifndef SM_WIDGETS_H
#define SM_WIDGETS_H

#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>

/*! \brief Set up widgets our way.
 
    This class allows easy creation of widgets and applies the appropriate
    font according to the user's settings (or the default).
    Depending on the widget, other parameters may be set.
*/
class SM_Widgets 
{
  public: 
    //! The constructor initializes the font for the class
    //! The destructor is defined as a null operation
    SM_Widgets( const QFont& );
    ~SM_Widgets( void ){}

    //! \param labelString - contents of label
    //! \param fontAdjust  - adjustment to default point size 
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}\n
    //! * Alignment is set to Qt::AlignVCenter | Qt::AlignLeft\n
    //! * Margin is set to 2\n
    QLabel* sm_label( const QString&, int = 0,  QFont::Weight = QFont::Bold );

    //! Same as sm_label except:\n
    //! * different default font size (-1 point)\n
    QLabel* sm_textlabel( const QString&, int = -1, QFont::Weight = QFont::Bold );

    //! Same as us_label except:\n
    //! * alignment set to Qt::AlignCenter\n
    //! * style set to QFrame::WinPanel | QFrame::Raised\n
    //! * MidLineWidth set to 2 for frame border\n
    QLabel* sm_banner( const QString&, int = 0,  QFont::Weight = QFont::Bold );

    //! \param labelString - text in pushbutton
    //! \param enabled - can be disabled
    //! \param fontAdjust -  adjustment to default font size\n
    //! * Standard Font family and size\n
    //! * AutoDefault is set false\n
    //! * button gets the rounded-border style sheet with a gray hover color\n
    QPushButton* sm_pushbutton( const QString&, bool = true, int = 0 ); 

    //! \param text - starting text in line edit box
    //! \param fontAdjust -  adjustment to default point size\n
    //! \param readonly -  if true, make the widget read only\n
    QLineEdit* sm_lineedit( const QString& = QString(), int = -1, bool = false );

    //! \param text - Text to set with checkbox
    //! \param cb - reference to a pointer of the checkbox\n
    //! \param state - Initial check state\n
    //! * Font weight is bold\n
    //! * The checkbox is preceded by a spacer one “w” wide\n
    QGridLayout* sm_checkbox  ( const QString&, QCheckBox*&, bool = false );

    //! * Sets up a default combo box\n
    QComboBox* sm_comboBox( void );

    //! \param fontAdjust -  adjustment to default font size\n
    QSpinBox*  sm_spinBox ( int = 0 );

  private:
      QFont      font;
};

#endif
