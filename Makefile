.PHONY: image
image:
	hooks/build

.PHONY: clean
clean:
	-docker rmi golang-test-nocgo-onbuild
	-docker rmi golang-test-cgo-onbuild
	-rm -rf *.results

.PHONY: test
test:
	./test.bash

.PHONY: debug
debug:
	docker run --rm -it --entrypoint bash ${IMAGE_NAME}
