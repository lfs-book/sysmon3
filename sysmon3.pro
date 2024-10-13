QT       += core gui widgets network

TARGET   = sysmon3
TEMPLATE = app

HEADERS = sysmon3.h     \
          sm3_widgets.h \
          sm3_config.h  \
          sm3_font.h    \
          sm3_color.h   \
          sm3_temps.h   \
          version.h     \
          setup.h       \
          udp.h

SOURCES = sysmon3.cpp     \
          sm3_widgets.cpp \
          sm3_config.cpp  \
          sm3_font.cpp    \
          sm3_color.cpp   \
          sm3_temps.cpp   \
          setup.cpp       \
          udp.cpp

isEmpty(PREFIX) {
    PREFIX = /usr
}
isEmpty(BINDIR) {
    BINDIR = $${PREFIX}/bin
}
target.path = $$BINDIR
INSTALLS += target
