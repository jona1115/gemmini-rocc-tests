# Test Branch for Project-SADD-MaMA
- This is the branch used in Project SADD MaMA. See [here](www.todo.com) for more details.
- This is branched off the forked directory's 1a1a1c6 commit.
- This is to be used with other directories forked by me, eg [Chipyard](www.todo.com), [Gemmini](www.todo.com), and [BOOM](www.todo.com).

Note: Tests we wrote are in the bareMetalC folder, indented by `MaMA_`.

---
Below content are from original README

# Quickstart
1. Clone submodules:
    ```bash
    git submodule update --init --recursive
    ```
1. Build the tests in this repo:
    ```bash
    ./build.sh
    ```

The tests' binaries will be installed in `build/`. The tests whose source code can be found in `bareMetalC/` will be installed in `build/bareMetalC/`, the tests in `imagenet/` will be installed in `build/imagenet/`, and so forth.

To run the tests yourself on a Gemmini ISA simulator, follow these steps:
1. Install [esp-isa-sim](https://github.com/ucb-bar/esp-isa-sim). If you are using [Chipyard](https://github.com/ucb-bar/chipyard), then running `./scripts/build-toolchains.sh esp-tools` from Chipyard's root directory will install it for you. The ISA simulator is called `spike`.
1. Run test programs on `spike`:
    ```bash
    cd build/bareMetalC
    spike --extension=gemmini mvin_mvout-baremetal
    ```

# Writing Your Own Gemmini Tests
`bareMetalC/template.c` is a template Gemmini test that you can base your own Gemmini tests off of. To write your own Gemmini test, run:

```bash
cp bareMetalC/template.c bareMetalC/my_test.c
```

Then, add `my_test` to the `tests` list at the top of `bareMetalC/Makefile`. Afterwards, running `./build.sh` will install `my_test-baremetal` in `build/bareMetalC`.

