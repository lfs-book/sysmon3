//! \file sm3_settings.h
#ifndef SM_SETTINGS_H
#define SM_SETTINGS_H

#include <QSettings>
#include <QListWidget>

//!  A class to allow encompass QSettings and server name.
class SM_Settings 
{
//  Q_OBJECT
  
  public:
    //*! \brief A suporting class for settings access
    SM_Settings( const QString& server );
   
    //! \brief A null destructor.
    ~SM_Settings() {}

    QString     value       ( const QString& key ) const;
    void        setValue    ( const QString& key, const QString& value );
    void        setBoolValue( const QString& key, const bool value );
    bool        contains    ( const QString& key ) const;

    // No longer used in sysmon3
    QStringList readArray ( const QString& key );
    void        writeArray( const QString& key, const QStringList& values );

    // Used in temperatures config
    void        removeGroup( const QString& key );
    void        addGroup   ( const QString& key, const QStringList& values );
    QStringList readGroup  ( const QString& group );

    QString     getDefault ( const QString& key );
    void        sync( void );

  private:
    QString      server;
    QSettings    settings;

    const QMap< QString, QString> defaults
    {   // key              default
       {"fontFamily",    "DejaVu Sans"}, 
       {"fontSize",      "12"         }, 
       {"fontBold",      "false"      },
       
       {"labelColor",    "#ffffff"    }, // white
       {"labelBg",       "#999999"    }, // medium gray
       {"dataColor",     "#000000"    }, // black
       {"dataBg",        "#efefef"    }, // light gray
       {"progressColor", "#308cc6"    }, // blue
       {"progressBg",    "#ffffff"    }, // white

       {"useTime",       "true"       },
       {"useDate",       "true"       },
       {"useUptime",     "true"       },
       {"useCPU",        "true"       },
       {"useCPUbar",     "true"       },
       {"useMemory",     "true"       },

       {"timeFormat",    "HH:mm:ss t" },
       {"dateFormat",    "ddd d MMM"  }
    };
};
#endif
