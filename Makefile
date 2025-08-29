dependencies:
	pip install -r scripts/requirements.txt

build:
	g++ -std=c++17 -Wall -Wextra -O2 -Iinc \
	src/*.cpp -o bin/photobooth \
	`pkg-config --cflags --libs opencv4`

run:
	./bin/photobooth

clean_output:
	rm ./out/*.jpg
	rm ./out/framed/*.png

clean:
	rm ./bin/*
