# NOTE: Make sure you have your project files in the ./project directory
# Will run the autograder and place the results in ./results/results.json

IMAGE=security-is-key
SRC=ghcr.io/mrelnekave

run:
	docker pull ${SRC}/${IMAGE}
	docker run --rm \
		-v ./project:/autograder/submission \
		-v ./results:/autograder/results \
		${SRC}/${IMAGE} \
		/autograder/run_autograder && cat results/results.json

# In case you want to run the autograder manually, use interactive
interactive:
	docker pull ${SRC}/${IMAGE}
	(docker ps | grep ${SRC}/${IMAGE} && \
	docker exec -it ${IMAGE} bash) || \
	docker run --rm --name ${IMAGE} -it \
		-v ./project:/autograder/submission \
		-v ./results:/autograder/results \
		${SRC}/${IMAGE} bash

