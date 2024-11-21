.PHONY: GOL

GOL: glad glfw/CMakeLists.txt build
	cmake --build build
	build/GOL

build:
	mkdir build
	cmake -S . -B build .

glad: .git venv
	./venv/bin/activate
	python -m pip install glad
	glad --profile core --out-path ./glad --api gl=4.1 --generator c-debug

venv:
	python -m venv venv
	chmod u+x ./venv/bin/activate*

.git:
	git init

glfw/CMakeLists.txt:
	git submodule update --init --recursive --depth 1
