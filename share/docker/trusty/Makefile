SQY_VERSION ?= $(shell git describe --tags --abbrev=0)


build-base: Dockerfile.base
	docker build -t sqy/trusty/base:$(SQY_VERSION) -f $< .

build-encoders-static: Dockerfile.encoders-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders-static:$(SQY_VERSION) -f $< .

build-utils-static: Dockerfile.utils-static build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils-static:$(SQY_VERSION) -f $< .

build-static: Dockerfile.static build-utils-static build-encoders-static
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/static:$(SQY_VERSION) -f $< .

build-encoders: Dockerfile.encoders build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders:$(SQY_VERSION) -f $< .

build-utils: Dockerfile.utils build-base
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/utils:$(SQY_VERSION) -f $< .

build-shared: build-utils build-encoders
	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/shared:$(SQY_VERSION) .

build: build-static build-shared
#	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/base -f Dockerfile.base .
#	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/encoders-static -f Dockerfile.encoders-static .
#	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/static-utils -f Dockerfile.static-utils .
#	docker build --build-arg SQY_VERSION=$(SQY_VERSION) -t sqy/trusty/static-deployment -f Dockerfile .

run-base: build-base
	docker run -it --rm sqy/trusty/base:$(SQY_VERSION) bash

run-encoders-static: build-encoders-static
	docker run -it --rm sqy/trusty/encoders-static:$(SQY_VERSION) bash

run-static: build-static
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/static:$(SQY_VERSION)

run-static: build-static
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/static:$(SQY_VERSION)


run: build-shared
	docker run -e ORG_GRADLE_PROJECT_bintray_user=psteinb -e ORG_GRADLE_PROJECT_bintray_key=${MY_BINTRAY_KEY} -it --rm sqy/trusty/shared:$(SQY_VERSION)

push2dockerhub : build-shared
	docker tag sqy/trusty/shared:$(SQY_VERSION) psteinb/sqy-on-trusty:$(SQY_VERSION)
	docker tag sqy/trusty/shared:$(SQY_VERSION) psteinb/sqy-on-trusty:latest
	docker push psteinb/sqy-on-trusty
