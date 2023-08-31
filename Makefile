BUILD_DIR=build
MAKE_FLAGS=-j 6
TEST_FLAGS=-j 50 --output-on-failure

.PHONY: all debug debug-werror release release-werror coverage doc clean test test-coverage test-performance

all:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && $(MAKE) $(MAKE_FLAGS) || echo "Type either \"make debug\" or \"make release\"!"

debug:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && $(MAKE) $(MAKE_FLAGS)

debug-werror:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DWERROR:BOOL=ON -DCMAKE_BUILD_TYPE=Debug .. && $(MAKE) $(MAKE_FLAGS)

debug-lib:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug -DMATA_BUILD_EXAMPLES:BOOL=OFF -DBUILD_TESTING:BOOL=OFF .. && $(MAKE) $(MAKE_FLAGS)

release:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && $(MAKE) $(MAKE_FLAGS)

release-werror:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DWERROR:BOOL=ON -DCMAKE_BUILD_TYPE=Release .. && $(MAKE) $(MAKE_FLAGS)

release-lib:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release -DMATA_BUILD_EXAMPLES:BOOL=OFF -DBUILD_TESTING:BOOL=OFF .. && $(MAKE) $(MAKE_FLAGS)

coverage:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug -DMATA_ENABLE_COVERAGE:BOOL=ON .. && $(MAKE) $(MAKE_FLAGS)

doc:
	cd $(BUILD_DIR) && $(MAKE) $(MAKE_FLAGS) doc

test:
	cd $(BUILD_DIR) && ctest $(TEST_FLAGS)

test-coverage:
	find . -name "**.gcda" -delete
	cd $(BUILD_DIR) && ctest $(TEST_FLAGS)
	gcovr -p -e "3rdparty/*" -j 6 --exclude-unreachable-branches --exclude-throw-branches $(BUILD_DIR)/src $(BUILD_DIR)/tests

test-performance:
	./tests-integration/pycobench -c ./tests-integration/jobs/corr-single-param-jobs.yaml < ./tests-integration/inputs/single-automata.input -o ./tests-integration/results/corr-single-param-jobs.out
	./tests-integration/pyco_proc --csv ./tests-integration/results/corr-single-param-jobs.out > ./tests-integration/results/corr-single-param-jobs.csv
	./tests-integration/pycobench -c ./tests-integration/jobs/corr-double-param-jobs.yaml < ./tests-integration/inputs/double-automata.input -o ./tests-integration/results/corr-double-param-jobs.out
	./tests-integration/pyco_proc --csv --param-no 2 ./tests-integration/results/corr-double-param-jobs.out > ./tests-integration/results/corr-double-param-jobs.csv

check:
	cd $(BUILD_DIR) && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && cppcheck --project=compile_commands.json --quiet --error-exitcode=1

install:
	cd $(BUILD_DIR) && $(MAKE) install

uninstall:
	cd $(BUILD_DIR) && $(MAKE) uninstall

clean:
	cd $(BUILD_DIR) && rm -rf *
	find doc/_build -type f -not \( -name '.nojekyll' \) -exec rm -d {} +
