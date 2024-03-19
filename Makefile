CFLAGS=-O3 -Wall -Werror -Wimplicit-fallthrough
# -O3 	指定编译器进行优化，级别为3。这意味着编译器将尝试进行更激进的优化，以提高生成的代码的性能。
# -Wall 启用所有警告。这将使编译器输出尽可能多的警告，以帮助开发者发现潜在的问题。
# -Wimplicit-fallthrough	启用对switch语句中的隐式case标签的警告。当在switch语句中省略了break语句而导致控制流“意外落入”下一个case标签时，编译器将输出警告。
SRCS=$(wildcard src/*.c)
# wildcard函数 匹配文件名模式并返回匹配的文件列表。
HDRS=$(wildcard src/*.h)
OBJS=$(patsubst src/%.c, obj/%.o, $(SRCS))
# patsubst函数 用于对指定的字符串进行模式替换。
CC=clang

rvemu: $(OBJS)
	$(CC) $(CFLAGS) -lm -o $@ $^ $(LDFLAGS)

$(OBJS): obj/%.o: src/%.c $(HDRS)
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf rvemu obj/

.PHONY: clean