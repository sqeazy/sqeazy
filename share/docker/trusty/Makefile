
build-base: Dockerfile.base
	docker build -t sqy/trusty/base -f $< .

build-encoders-static: Dockerfile.encoders-static build-base
	docker build -t sqy/trusty/encoders-static -f $< .


build: build-base build-encoders-static
#	docker build -t sqy/trusty/base -f Dockerfile.base .
#	docker build -t sqy/trusty/encoders-static -f Dockerfile.encoders-static .
	docker build -t sqy/trusty/static-utils -f Dockerfile.static-utils .
	docker build -t sqy/trusty/static-deployment -f Dockerfile .

run-base: build-base
	docker run -it --rm sqy/trusty/base:latest bash

run-encoders-static: build-encoders-static
	docker run -it --rm sqy/trusty/encoders-static:latest bash
