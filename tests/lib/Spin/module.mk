Test_Spin_SRC :=			main.cpp BakeryCounter.cpp  Logger.cpp  atomicPrimitives.cpp   
SRC += $(patsubst %,tests/lib/Spin/%,$(Test_Spin_SRC))

Test_Spin_OBJ := $(patsubst %.cpp,tests/lib/Spin/%.lo,$(Test_Spin_SRC))

OBJ += $(Test_Spin_OBJ)

TEST_LDFLAGS=libSpin.la $(LIBS) -lcppunit -lltdl
$(eval $(call LINK_BINARY_template,Test_Spin.bin,$(Test_Spin_OBJ),$(TEST_LDFLAGS)))
CHECK_DEPS += Test_Spin.bin 


