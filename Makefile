.PHONY: GOL

GOL: glad glfw


glad: .git venv
	./venv/bin/activate
	python -m pip install glad
	glad --profile core --out-path ./glad --api gl=4.6 --generator c-debug

venv:
	python -m venv venv
	chmod u+x ./venv/bin/activate*

.git:
	git init

glfw:
	git submodule add --depth 1 https://github.com/glfw/glfw glfw