Test_Spin_SRC :=			\
	Connection.cpp			\
	Connector.cpp			\
	Listener.cpp			\
	Pipe.cpp			\
	Response.cpp			\
	UDPSocket.cpp			\
	main.cpp

SRC += $(patsubst %,tests/lib/Spin/%,$(Test_Spin_SRC))

Test_Spin_OBJ := $(patsubst %.cpp,tests/lib/Spin/%.lo,$(Test_Spin_SRC))

OBJ += $(Test_Spin_OBJ)

TEST_LDFLAGS=libSpin.la libScorpion.la libAcari.la libAgelena.la $(LIBS) -lcppunit -lltdl -lboost_filesystem-gcc-mt-1_33_1 -lssl -lcrypto -lboost_thread-gcc-mt-1_33_1
$(eval $(call LINK_BINARY_template,Test_Spin.bin,$(Test_Spin_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Spin.bin 


