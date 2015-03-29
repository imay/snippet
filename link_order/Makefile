# make all object
# link_1_first: link func1.c first
# link_2_first: link func2.c first
ALL: link_1_first link_2_first
.PHONY: all

link_2_first: main.c libfunc2.a libfunc1.a
	gcc $+ -o $@

link_1_first: main.c libfunc1.a libfunc2.a
	gcc $+ -o $@

lib%.a: %.o
	ar rcs $@ $<

%.o: %.c
	gcc -static -c $< -o $@

# This will 
link_obj: main.c func1.o func2.o
	gcc $^ -o $@
.PHONY: link_obj

clean:
	rm -f *.o *.a link_1_first link_2_first
.PHONY: clean

