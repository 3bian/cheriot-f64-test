-- Copyright 3bian Limited and CHERIoT Contributors.
-- SPDX-License-Identifier: Apache-2.0

set_project("Empty Project template")

sdkdir = "third_party/cheriot_rtos/sdk"

set_toolchains("cheriot-clang")

includes(sdkdir)
includes(path.join(sdkdir, "lib"))

option("board")
    set_default("sail")

option("print-doubles")
    set_default(true)

compartment("entry_point")
    add_deps("debug",
             "freestanding",
             "softfloat",
             "stdio")
    add_files("src/main.cc")

firmware("f64-tests")
    add_deps("entry_point")
    on_load(function(target)
        target:values_set("board", "$(board)")
        target:values_set("threads", {
            {
                compartment = "entry_point",
                priority = 1,
                entry_point = "init",
                stack_size = 0x1000,
                trusted_stack_frames = 2
            }
        }, {expand = false})
    end)
