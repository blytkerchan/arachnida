# the library
Scorpion_SRC =					\
	BIO.cpp					\
	Context.cpp				\
	Details/GetPasswords.cpp		\
	Exceptions/SSL.cpp			\
	Private/OpenSSL.cpp

Scorpion_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Scorpion/%,$(Scorpion_SRC))

INSTALL_HEADERS += $(patsubst %,Scorpion/%,$(Scorpion_INSTALL_HEADERS))
Scorpion_OBJ := $(patsubst %.cpp,lib/Scorpion/%.lo,$(Scorpion_SRC))
OBJ += $(Scorpion_OBJ)

$(eval $(call LINK_LIBRARY_template,libScorpion.la,$(Scorpion_OBJ),-whole-archive -version-info 1:0:0 -rpath $(libdir)))
