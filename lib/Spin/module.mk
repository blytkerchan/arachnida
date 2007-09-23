# the library
Spin_SRC =					\
	Private/BakeryCounter.cpp		\
	Private/ConnectionHandler.cpp		\
	Private/CountedEvent.cpp		\
	Private/Logger.cpp			\
	Private/ObservationAdapter.cpp		\
	Private/atomicPrimitives.cpp		\
	Private/yield.cpp			\
	Handlers/NewConnectionHandler.cpp	\
	Server.cpp


Spin_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Spin/%,$(Spin_SRC))

INSTALL_HEADERS += $(patsubst %,Spin/%,$(Spin_INSTALL_HEADERS))
Spin_OBJ := $(patsubst %.cpp,lib/Spin/%.lo,$(Spin_SRC))
OBJ += $(Spin_OBJ)

$(eval $(call LINK_LIBRARY_template,libSpin.la,$(Spin_OBJ)))
