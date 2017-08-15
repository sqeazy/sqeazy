
build-base: Dockerfile.base
	docker build -t sqy/trusty/base -f $< .

build-encoders-static: Dockerfile.encoders-static build-base
	docker build -t sqy/trusty/encoders-static -f $< .

build-utils-static: Dockerfile.utils-static build-base
	docker build -t sqy/trusty/utils-static -f $< .

build-static: Dockerfile.static build-utils-static build-encoders-static
	docker build -t sqy/trusty/static -f $< .

build-encoders: Dockerfile.encoders build-base
	docker build -t sqy/trusty/encoders -f $< .

build-utils: Dockerfile.utils build-base
	docker build -t sqy/trusty/utils -f $< .

build-shared: build-utils build-encoders
	docker build -t sqy/trusty/shared .

build: build-static build-shared
#	docker build -t sqy/trusty/base -f Dockerfile.base .
#	docker build -t sqy/trusty/encoders-static -f Dockerfile.encoders-static .
#	docker build -t sqy/trusty/static-utils -f Dockerfile.static-utils .
#	docker build -t sqy/trusty/static-deployment -f Dockerfile .

run-base: build-base
	docker run -it --rm sqy/trusty/base:latest bash

run-encoders-static: build-encoders-static
	docker run -it --rm sqy/trusty/encoders-static:latest bash

run-static: build-static
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/static:latest

run: build-shared
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/shared:latest
