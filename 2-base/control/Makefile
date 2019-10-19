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
	# ./hsv-modification 100 0 0 ../data/horse.png horse-hsv.png; pvisu horse-hsv.png
	# ./eq-histogram ../data/eq-ex.png; pvisu ../data/eq-ex.png; pvisu eq.png; pvisu eq-ocv.png; pvisu diff.png
	# ./adaptative-th ../data/th-ex.png 3 4; pvisu th-ex.png; pvisu th.png; pvisu th-ocv-mean.png; pvisu th-ocv-gauss.png; pvisu diff.png
	./otsu ../data/contraste-3.png
	./otsu ../data/text.png


.PHONY: clean
clean:
	$(RM) *~ *.png

.PHONY: cleanall
cleanall: clean
	$(RM) $(BIN) *.o *.pdf
