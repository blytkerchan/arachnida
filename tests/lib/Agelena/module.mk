Test_Agelena_SRC :=			\
	Logger.cpp			\
	main.cpp

SRC += $(patsubst %,tests/lib/Agelena/%,$(Test_Agelena_SRC))

Test_Agelena_OBJ := $(patsubst %.cpp,tests/lib/Agelena/%.lo,$(Test_Agelena_SRC))

OBJ += $(Test_Agelena_OBJ)

TEST_LDFLAGS=libAgelena.la $(LIBS) -lcppunit -lltdl -lboost_filesystem-gcc-mt-1_33_1 -lssl -lcrypto -lboost_thread-gcc-mt-1_33_1
$(eval $(call LINK_BINARY_template,Test_Agelena.bin,$(Test_Agelena_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Agelena.bin 


