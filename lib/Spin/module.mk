# the library
Spin_SRC =						\
	Connection.cpp					\
	Connector.cpp					\
	Details/Address.cpp				\
	Details/Attibutes.cpp				\
	Details/Response.cpp				\
	Exceptions/Connection/BindFailure.cpp		\
	Exceptions/Connection/ListenFailure.cpp		\
	Exceptions/HTTP/InvalidHeader.cpp		\
	Exceptions/HTTP/UnknownMethod.cpp		\
	Exceptions/HTTP/UnsupportedProtocol.cpp		\
	Exceptions/Socket.cpp				\
	Handlers/HTTPConnectionHandler.cpp		\
	Handlers/HTTPDataHandler.cpp			\
	Handlers/HTTPRequestHandler.cpp			\
	Handlers/NewConnectionHandler.cpp		\
	Handlers/NewDataHandler.cpp			\
	Handlers/UDPDataHandler.cpp			\
	Listener.cpp					\
	Private/ConnectionHandler.cpp			\
	Private/Pipe.cpp				\
	Private/yield.cpp				\
	UDPSocket.cpp

Spin_INSTALL_HEADERS += $(filter-out Private/%, $(wildcard *.h) $(wildcard */*.h))

SRC += $(patsubst %,lib/Spin/%,$(Spin_SRC))

INSTALL_HEADERS += $(patsubst %,Spin/%,$(Spin_INSTALL_HEADERS))
Spin_OBJ := $(patsubst %.cpp,lib/Spin/%.lo,$(Spin_SRC))
OBJ += $(Spin_OBJ)

$(eval $(call LINK_LIBRARY_template,libSpin.la,$(Spin_OBJ),-whole-archive -version-info 1:0:0 -rpath $(libdir)))
