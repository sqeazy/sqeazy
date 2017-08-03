
build-base: Dockerfile.base
	docker build -t sqy/trusty/base -f $< .

build-encoders-static: Dockerfile.encoders-static build-base
	docker build -t sqy/trusty/encoders-static -f $< .

build-utils-static: Dockerfile.utils-static build-base
	docker build -t sqy/trusty/utils-static -f $< .

build-static: Dockerfile build-utils-static build-encoders-static
	docker build -t sqy/trusty/static -f $< .

build: build-static
#	docker build -t sqy/trusty/base -f Dockerfile.base .
#	docker build -t sqy/trusty/encoders-static -f Dockerfile.encoders-static .
#	docker build -t sqy/trusty/static-utils -f Dockerfile.static-utils .
#	docker build -t sqy/trusty/static-deployment -f Dockerfile .

run-base: build-base
	docker run -it --rm sqy/trusty/base:latest bash

run-encoders-static: build-encoders-static
	docker run -it --rm sqy/trusty/encoders-static:latest bash

run-static: build-static
	docker run -e BINTRAY_USER=psteinb -e BINTRAY_KEY=$(MY_BINTRAY_KEY) -it --rm sqy/trusty/static:latest
