# the library
Agelena_SRC =					\
	Logger.cpp				\
	Loggers/DefaultPOSIXLogger.cpp

Agelena_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Agelena/%,$(Agelena_SRC))

INSTALL_HEADERS += $(patsubst %,Agelena/%,$(Agelena_INSTALL_HEADERS))
Agelena_OBJ := $(patsubst %.cpp,lib/Agelena/%.lo,$(Agelena_SRC))
OBJ += $(Agelena_OBJ)

$(eval $(call LINK_LIBRARY_template,libAgelena.la,$(Agelena_OBJ),-whole-archive -version-info 0:0:0 -rpath $(libdir)))
