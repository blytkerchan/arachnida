# determine the object files
define OBJ_template
$(2)_OBJ :=	$$(patsubst %.c,$(1)/%.lo,$$(filter %.c,$$($(1)_SRC)))		\
		$$(patsubst %.y,$(1)/%.lo,$$(filter %.y,$$($(1)_SRC)))		\
		$$(patsubst %.l,$(1)/%.lo,$$(filter %.l,$$($(1)_SRC)))		\
		$$(patsubst %.cc,$(1)/%.lo,$$(filter %.cc,$$($(1)_SRC)))	\
		$$(patsubst %.cpp,$(1)/%.lo,$$(filter %.cpp,$$($(1)_SRC))) 
OBJ += $$($(2)_OBJ)
endef

