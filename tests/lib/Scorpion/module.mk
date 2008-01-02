Test_Scorpion_SRC :=			\
	Context.cpp			\
	main.cpp

SRC += $(patsubst %,tests/lib/Scorpion/%,$(Test_Scorpion_SRC))

Test_Scorpion_OBJ := $(patsubst %.cpp,tests/lib/Scorpion/%.lo,$(Test_Scorpion_SRC))

OBJ += $(Test_Scorpion_OBJ)

TEST_LDFLAGS=libScorpion.la libAcari.la $(LIBS) -lcppunit -lltdl -lboost_filesystem-gcc-mt-1_33_1 -lssl -lcrypto -lboost_thread-gcc-mt-1_33_1
$(eval $(call LINK_BINARY_template,Test_Scorpion.bin,$(Test_Scorpion_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Scorpion.bin 


