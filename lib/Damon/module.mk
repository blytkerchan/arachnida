# the library
Damon_SRC =					\
	Private/parseURL.cpp			\
	Request.cpp				\
	Response.cpp

Damon_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Damon/%,$(Damon_SRC))

INSTALL_HEADERS += $(patsubst %,Damon/%,$(Damon_INSTALL_HEADERS))
Damon_OBJ := $(patsubst %.cpp,lib/Damon/%.lo,$(Damon_SRC))
OBJ += $(Damon_OBJ)

$(eval $(call LINK_LIBRARY_template,libDamon.la,$(Damon_OBJ),-whole-archive -version-info 1:0:0 -rpath $(libdir)))
