TEMPLATE = app
TARGET = frequency-analyzer

QT += qml quick widgets multimedia 3dcore 3drender 3dextras 3dinput 3dquick 3dquickextras datavisualization
CONFIG += c++17

RESOURCES += materials.qrc

OTHER_FILES += \
    *.qml \

HEADERS += \
    audiosampler.h \
    waterfallitem.h \
    dft/dft.h \
    dft/radix2fft.h \
    ffft/Array.h \
    ffft/Array.hpp \
    ffft/def.h \
    ffft/DynArray.h \
    ffft/DynArray.hpp \
    ffft/FFTReal.h \
    ffft/FFTReal.hpp \
    ffft/FFTRealFixLen.h \
    ffft/FFTRealFixLen.hpp \
    ffft/FFTRealFixLenParam.h \
    ffft/FFTRealPassDirect.h \
    ffft/FFTRealPassDirect.hpp \
    ffft/FFTRealPassInverse.h \
    ffft/FFTRealPassInverse.hpp \
    ffft/FFTRealSelect.h \
    ffft/FFTRealSelect.hpp \
    ffft/FFTRealUseTrigo.h \
    ffft/FFTRealUseTrigo.hpp \
    ffft/OscSinCos.h \
    ffft/OscSinCos.hpp

SOURCES += main.cpp \
    audiosampler.cpp \
    waterfallitem.cpp \
    dft/dft.cpp \
    dft/radix2fft.cpp

DISTFILES += \
    main.qml \


CONFIG += resources_big
