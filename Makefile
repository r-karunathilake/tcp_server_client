TARGETS   := server client 
CXX       := clang 
STD       := -std=gnu17
DEBUG     := -g
OPT       := -O0 # Disable compiler optimization 
WARN      := -Wall
# Flags for different build configurations 
CXXFLAGS  := $(STD)
ifeq ($(BUILD), DEV)
	CXXFLAGS += $(DEBUG) $(OPT) $(WARN)
endif
SOURCE   := server.c client.c
OBJS     := $(SOURCE:.c=.o) # Pattern substitution

# Make recipes 
all: $(TARGETS)
	@echo "Running make 'all' recipe..."

$(TARGETS): %: %.o 
	@echo "Compiling program..."
	# $@ (recipe target) $< (first dependency for the respective target)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Recipe: build object files 
%.o: %.c 
	@echo "Building $< object file..."
	# $< (first dependency for the recipe)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# Clean intermediate object files and executable
clean:
	@echo "Cleaning object files..."
	@rm -f $(OBJS) $(TARGETS)
