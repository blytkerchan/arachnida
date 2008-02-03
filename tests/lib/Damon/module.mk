Test_Damon_SRC :=			\
	Request.cpp			\
	main.cpp

SRC += $(patsubst %,tests/lib/Damon/%,$(Test_Damon_SRC))

Test_Damon_OBJ := $(patsubst %.cpp,tests/lib/Damon/%.lo,$(Test_Damon_SRC))

OBJ += $(Test_Damon_OBJ)

TEST_LDFLAGS=libDamon.la libSpin.la libScorpion.la libAcari.la $(LIBS) -lcppunit -lltdl -lboost_filesystem-gcc-mt-1_33_1 -lssl -lcrypto -lboost_thread-gcc-mt-1_33_1 -lboost_regex-gcc-mt-1_33_1
$(eval $(call LINK_BINARY_template,Test_Damon.bin,$(Test_Damon_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Damon.bin 


