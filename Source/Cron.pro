TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += console

HEADERS += \
    Cron/chrono.h \
    Cron/Clock.h \
    Cron/CronValidator.h \
    Cron/Exception.h \
    Cron/Parser.h \
    Cron/TimeUnit.h \
    Cron/TimeUtil.h

SOURCES += \
    Cron/Clock.cpp \
    Cron/CronValidator.cpp \
    Cron/Parser.cpp \
    Cron/TimeUnit.cpp \
    Cron/TimeUtil.cpp \
    main.cpp



