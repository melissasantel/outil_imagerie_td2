CPPFLAGS=-I/opt/opencv/include
CXXFLAGS=-Wall -Wextra -Werror
LDFLAGS=-Wl,-R/opt/opencv/lib -L/opt/opencv/lib
LDLIBS=\
	-lopencv_core\
	-lopencv_imgproc\
	-lopencv_highgui
BIN=\
	color-convert\
	hsv-modification\
	eq-histogram\
	adaptative-th\
	otsu

.PHONY: all
all: $(BIN)

.PHONY: test
test:
	# ./color-convert ../data/lena_color.png
	# ./hsv-modification 100 0 0 ../data/horse.png horse-hsv.png
	./eq-histogram ../data/eq-ex.png
	# ./adaptative-th ../data/th-ex.png 3 4
	# ./otsu ../data/contraste-c3.png

.PHONY: clean
clean:
	$(RM) *~ *.png

.PHONY: cleanall
cleanall: clean
	$(RM) $(BIN) *.o *.pdf
