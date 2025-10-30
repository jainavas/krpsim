CXX = c++
CXXFLAGS := -Wall -Wextra -Werror -O3 -std=c++17

SRCS = src/main.cpp \
		src/parser.cpp
OBJS = $(SRCS:.cpp=.o)

EXEC = krpsim

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $(EXEC) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(EXEC)

re: fclean all

.PHONY: all clean fclean re
