.PHONY: image
image:
	hooks/build

.PHONY: clean
clean:
	-rm -rf *.results

.PHONY: test
test:
	./test.bash

.PHONY: debug
debug:
	docker run --rm -it --entrypoint bash ${IMAGE_NAME}
