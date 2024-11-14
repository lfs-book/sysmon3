#include "sysmon3.h"
#include "sm3_settings.h"

SM_Settings::SM_Settings( const QString& server ) : settings( server ) 
{
}

QString SM_Settings::value( const QString& key ) const
{
   QString s = settings.value( key ).toString();

   if ( s.isEmpty() ) 
      s = defaults.value( key , "" );
   
   return s;
}

void SM_Settings::setValue( const QString& key, const QString& value )
{
   if ( value == defaults.value( key ) )
      settings.remove( key );
   else
      settings.setValue( key, value );
}

void SM_Settings::setBoolValue( const QString& key, const bool value )
{
   QString boolValue;
   if ( value ) boolValue = "true";
   else         boolValue = "false";

   setValue( key, boolValue );
}

bool SM_Settings::contains( const QString& key ) const
{
   return settings.contains( key );
}

QStringList SM_Settings::readArray( const QString& key )
{
   QStringList sl;

   int size = settings.beginReadArray( key );
   
   if ( size > 0 )
   {
      for ( int i = 0; i < size; i++ ) 
      {
         settings.setArrayIndex( i );
         QString s = settings.value( key ).toString();
         sl.append( s );
      }
   }
   
   settings.endArray();
   return sl;
}

void SM_Settings::writeArray( const QString& key, const QStringList& values )
{
   settings.remove( key );
   settings.beginWriteArray( key );

   for ( int i = 0; i < values.size(); i++  )
   {
      settings.setArrayIndex(i);
      settings.setValue( key, values.at( i ) );
   }
   settings.endArray();
}

// Groups are used to define the temperature interface and sensors for a server
// In the following methods 'group', for now, is always 'temperatures'
void SM_Settings::removeGroup( const QString& group )
{
   settings.beginGroup( group );
   settings.remove( "" );
   settings.endGroup();
}

// This is only used in one place - temperature config
void SM_Settings::addGroup( const QString& group, const QStringList& values )
{
   settings.beginGroup( group );

   for ( int i = 0; i < values.size(); i++ )
   {
      QString     entry = values.at( i );
      QStringList items = entry.split( ';' );
      QString     key   = items.at( 0 );
      QString     value = items.at( 1 );

      settings.setValue( key, value );
   }

   settings.endGroup();
}

QStringList SM_Settings::readGroup( const QString& group )
{
   QStringList config;

   settings.beginGroup( group );
   QStringList keys = settings.childKeys();

   foreach (const QString &key, keys) 
      config << settings.value( key ).toString();

   settings.endGroup();
   return config;
}

QString SM_Settings::getDefault( const QString& key )
{
   return defaults.value( key , "" );
}

void SM_Settings::sync( void )
{
   settings.sync();
}

