Test_Acari_SRC :=			\
	atomicPrimitives.cpp		\
	Attributes.cpp			\
	BakeryCounter.cpp		\
	main.cpp

SRC += $(patsubst %,tests/lib/Acari/%,$(Test_Acari_SRC))

Test_Acari_OBJ := $(patsubst %.cpp,tests/lib/Acari/%.lo,$(Test_Acari_SRC))

OBJ += $(Test_Acari_OBJ)

TEST_LDFLAGS=libAcari.la libAgelena.la $(LIBS) -lcppunit -lltdl -lboost_filesystem-gcc-mt-1_33_1 -lssl -lcrypto -lboost_thread-gcc-mt-1_33_1
$(eval $(call LINK_BINARY_template,Test_Acari.bin,$(Test_Acari_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Acari.bin 


