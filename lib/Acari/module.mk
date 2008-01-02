# the library
Acari_SRC =					\
	atomicPrimitives.cpp			\
	BakeryCounter.cpp			\
	ThreadLocalStorage.cpp

Acari_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Acari/%,$(Acari_SRC))

INSTALL_HEADERS += $(patsubst %,Acari/%,$(Acari_INSTALL_HEADERS))
Acari_OBJ := $(patsubst %.cpp,lib/Acari/%.lo,$(Acari_SRC))
OBJ += $(Acari_OBJ)

$(eval $(call LINK_LIBRARY_template,libAcari.la,$(Acari_OBJ),-whole-archive -version-info 0:0:0 -rpath $(libdir)))
