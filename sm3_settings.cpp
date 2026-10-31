#include <algorithm>   // std::sort

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

   for ( const QString& entry : values )
   {
      int semicolon = entry.indexOf( ';' );
      if ( semicolon < 0 ) continue;          // Not "key;value"

      settings.setValue( entry.left( semicolon ), entry.mid( semicolon + 1 ) );
   }

   settings.endGroup();
}

QStringList SM_Settings::readGroup( const QString& group )
{
   QStringList config;

   settings.beginGroup( group );
   QStringList keys = settings.childKeys();

   // childKeys() is alphabetical: temp10 before temp2.  The keys are all
   // "temp" + number, so shorter first, then alphabetical, is numeric order.
   std::sort( keys.begin(), keys.end(), []( const QString& a, const QString& b )
      { return a.size() != b.size() ? a.size() < b.size() : a < b; } );

   for (const QString& key : keys) 
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

