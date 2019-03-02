TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += console

SOURCES += \
        main.cpp \
    Cron/Clock.cpp \
    Cron/CronValidator.cpp \
    Cron/Parser.cpp \
    Cron/TimeUnit.cpp \
    Cron/TimeUtil.cpp

HEADERS += \
    Cron/chrono.h \
    Cron/Clock.h \
    Cron/CronValidator.h \
    Cron/Parser.h \
    Cron/TimeUnit.h \
    Cron/TimeUtil.h \
    Cron/Exception.h

