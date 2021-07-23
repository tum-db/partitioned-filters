BSD=ON
SHELL=bash

init-python:
	python3 -m venv venv; \
	source venv/bin/activate; \
	pip3 install -r requirements.txt; \
	deactivate; \

init: init-python
	mkdir -p build; \
	cd build; \
	cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_BSD=${BSD} ..; \
	cd ..

test: init
	cd build; \
	make -j $(shell nproc) tester; \
	./test/tester

benchmark: init
	source venv/bin/activate; \
	python3 python/benchmark_executor/benchmark_executor.py; \
	deactivate

plot: init-python
	source venv/bin/activate; \
	python3 python/benchmark_plotter/benchmark_plotter.py; \
	deactivate

jupyter: init-python
	source venv/bin/activate; \
	jupyter-lab; \
	deactivate

plot: init-python
	source venv/bin/activate; \
	cd plots; \
	python3 optimization.py; \
	python3 evaluation.py; \
	python3 perfopt.py; \
	deactivate


clean:
	rm -rf venv build
